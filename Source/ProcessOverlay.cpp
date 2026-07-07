#include "ProcessOverlay.h"
#include "Logger.h"
#include <map>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------
static constexpr int ICON_SIZE       = 32;   // matches taskbar icon size, no upscaling
static constexpr int SLOT_WIDTH      = 52;   // 32 + 20px padding
static constexpr int CORNER_RADIUS   = 8;    // rounded corner radius
static constexpr int PANEL_PADDING   = 16;
static constexpr int OVERLAY_HEIGHT  = 80;   // icon + text + padding
static constexpr int LABEL_FONT_SIZE = 18;

static constexpr COLORREF BG_COLOR        = RGB(40, 40, 40);
static constexpr COLORREF HIGHLIGHT_COLOR = RGB(0, 120, 215);
static constexpr COLORREF TEXT_COLOR      = RGB(230, 230, 230);

static const wchar_t OVERLAY_CLASS[] = L"AltBacktickOverlay";

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------
static HWND       g_hwnd    = NULL;
static bool       g_visible = false;

static std::vector<ProcessEntry> g_list;
static size_t                    g_selection = 0;
static std::map<DWORD, HICON>   g_iconCache;
static std::wstring             g_selectedLabel;

// ---------------------------------------------------------------------------
// Icon helpers
// ---------------------------------------------------------------------------
static HICON ExtractIconForProcess(const ProcessEntry& entry)
{
    if (entry.processPath.empty()) return NULL;

    int len = MultiByteToWideChar(CP_UTF8, 0, entry.processPath.c_str(), -1, NULL, 0);
    if (len <= 0) return NULL;

    std::wstring wp(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, entry.processPath.c_str(), -1, &wp[0], len);

    SHFILEINFOW sfi = {};
    if (SHGetFileInfoW(wp.c_str(), 0, &sfi, sizeof(sfi),
                       SHGFI_ICON | SHGFI_LARGEICON))
        return sfi.hIcon;
    return NULL;
}

static HICON GetCachedIcon(const ProcessEntry& entry)
{
    auto it = g_iconCache.find(entry.processId);
    if (it != g_iconCache.end()) return it->second;

    HICON h = ExtractIconForProcess(entry);
    if (!h && !entry.processName.empty())
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, entry.processName.c_str(), -1, NULL, 0);
        if (len > 0)
        {
            std::wstring wn(static_cast<size_t>(len), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, entry.processName.c_str(), -1, &wn[0], len);
            SHFILEINFOW sfi = {};
            SHGetFileInfoW(wn.c_str(), 0, &sfi, sizeof(sfi),
                           SHGFI_ICON | SHGFI_LARGEICON | SHGFI_SHELLICONSIZE | SHGFI_USEFILEATTRIBUTES);
            h = sfi.hIcon;
        }
    }
    g_iconCache[entry.processId] = h;
    return h;
}

static void ClearIconCache()
{
    for (auto& [_, h] : g_iconCache)
        if (h) DestroyIcon(h);
    g_iconCache.clear();
}

// ---------------------------------------------------------------------------
// Label helper — strip .exe suffix
// ---------------------------------------------------------------------------
static std::string StripExe(const std::string& name)
{
    if (name.size() > 4)
    {
        std::string ext = name.substr(name.size() - 4);
        for (auto& c : ext) c = (char)tolower((unsigned char)c);
        if (ext == ".exe") return name.substr(0, name.size() - 4);
    }
    return name;
}

static void UpdateLabel(size_t idx)
{
    if (idx >= g_list.size()) return;
    std::string n = StripExe(g_list[idx].processName);
    int len = MultiByteToWideChar(CP_UTF8, 0, n.c_str(), -1, NULL, 0);
    if (len > 0)
    {
        g_selectedLabel.resize((size_t)len);
        MultiByteToWideChar(CP_UTF8, 0, n.c_str(), -1, &g_selectedLabel[0], len);
    }
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------
struct Metrics { int x, y, w, h; };

static Metrics CalcMetrics(size_t count)
{
    Metrics m = {};
    HWND fg = GetForegroundWindow();
    HMONITOR hm = MonitorFromWindow(fg, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(hm, &mi);

    int sw = mi.rcWork.right  - mi.rcWork.left;
    int sh = mi.rcWork.bottom - mi.rcWork.top;
    int sx = mi.rcWork.left;
    int sy = mi.rcWork.top;

    m.w = (int)count * SLOT_WIDTH + PANEL_PADDING * 2;
    int maxW = (int)(sw * 0.7f);
    if (m.w > maxW) m.w = maxW;

    m.h = OVERLAY_HEIGHT;
    m.x = sx + (sw - m.w) / 2;
    m.y = sy + (sh - m.h) / 2; // vertically centered
    return m;
}

// ---------------------------------------------------------------------------
// Window procedure
// ---------------------------------------------------------------------------
static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int w = rc.right, h = rc.bottom;

        HDC     memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
        HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

        // Background
        HBRUSH bg = CreateSolidBrush(BG_COLOR);
        FillRect(memDC, &rc, bg);
        DeleteObject(bg);

        // Icons
        int count = (int)g_list.size();
        for (int i = 0; i < count; i++)
        {
            int sx = PANEL_PADDING + i * SLOT_WIDTH;
            int ix = sx + (SLOT_WIDTH - ICON_SIZE) / 2;
            int iy = (OVERLAY_HEIGHT - ICON_SIZE) / 2 - 4;

            if ((size_t)i == g_selection)
            {
                HBRUSH hl = CreateSolidBrush(HIGHLIGHT_COLOR);
                RECT hr = { ix - 2, iy - 2, ix + ICON_SIZE + 2, iy + ICON_SIZE + 2 };
                FillRect(memDC, &hr, hl);
                DeleteObject(hl);
            }

            HICON ic = GetCachedIcon(g_list[i]);
            if (ic) DrawIconEx(memDC, ix, iy, ic, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
        }

        // Label
        if (!g_selectedLabel.empty())
        {
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, TEXT_COLOR);
            HFONT fnt = CreateFontW(LABEL_FONT_SIZE, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HGDIOBJ oldF = SelectObject(memDC, fnt);
            RECT tr = { 0, OVERLAY_HEIGHT - 22, rc.right, rc.bottom };
            DrawTextW(memDC, g_selectedLabel.c_str(), -1, &tr,
                      DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(memDC, oldF);
            DeleteObject(fnt);
        }

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
bool CreateOverlayWindow(HINSTANCE hInstance)
{
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = OverlayWndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = OVERLAY_CLASS;
    wc.hbrBackground = NULL;
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        OVERLAY_CLASS, L"AltBacktickSwitcher", WS_POPUP,
        0, 0, 100, OVERLAY_HEIGHT,
        NULL, NULL, hInstance, NULL);
    return g_hwnd != NULL;
}

void DestroyOverlayWindow()
{
    if (g_hwnd) { DestroyWindow(g_hwnd); g_hwnd = NULL; }
    ClearIconCache();
    g_list.clear();
    g_visible = false;
}

void ShowOverlay(const std::vector<ProcessEntry>& list, size_t selection)
{
    if (!g_hwnd) return;
    g_list = list;
    g_selection = selection;
    UpdateLabel(selection);

    Metrics m = CalcMetrics(list.size());
    SetWindowPos(g_hwnd, HWND_TOPMOST, m.x, m.y, m.w, m.h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Rounded corners via window region
    HRGN rgn = CreateRoundRectRgn(0, 0, m.w + 1, m.h + 1, CORNER_RADIUS, CORNER_RADIUS);
    SetWindowRgn(g_hwnd, rgn, TRUE);
    DeleteObject(rgn);

    g_visible = true;
    InvalidateRect(g_hwnd, NULL, TRUE);
}

void UpdateSelection(size_t selection)
{
    if (!g_hwnd) return;
    g_selection = selection;
    UpdateLabel(selection);
    InvalidateRect(g_hwnd, NULL, TRUE);
}

void HideOverlay()
{
    if (g_hwnd && g_visible) { ShowWindow(g_hwnd, SW_HIDE); g_visible = false; }
}

bool IsOverlayVisible()
{
    return g_visible;
}
