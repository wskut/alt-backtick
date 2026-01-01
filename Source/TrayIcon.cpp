#include "TrayIcon.h"
#include "Common.h"
#include "Logger.h"
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

static NOTIFYICONDATA nid = {};
static HWND g_hwnd = NULL;

static LRESULT CALLBACK TrayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        lstrcpy(nid.szTip, TEXT("AltBacktick"));
        Shell_NotifyIcon(NIM_ADD, &nid);
        return 0;
    }
    case WM_TRAYICON:
    {
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
        {
            POINT cursor;
            GetCursorPos(&cursor);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit AltBacktick"));
            
            SetForegroundWindow(hwnd);
            int selectedId = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, cursor.x, cursor.y, 0, hwnd, NULL);
            if (selectedId == ID_TRAY_EXIT)
            {
                PostQuitMessage(0);
            }
            DestroyMenu(hMenu);
        }
        return 0;
    }
    case WM_DESTROY:
    {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool InitTrayIcon()
{
    // 숨김 윈도우 클래스 등록
    const char CLASS_NAME[] = "AltBacktickTrayClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = TrayWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // 숨김 윈도우 생성
    g_hwnd = CreateWindowEx(
        0, CLASS_NAME, "AltBacktick hidden window", 0,
        0, 0, 0, 0,
        NULL, NULL, wc.hInstance, NULL);

    if (g_hwnd == NULL)
    {
        Logger::Error("Failed to create hidden window for tray icon.");
        return false;
    }

    return true;
}

void CleanupTrayIcon()
{
    if (g_hwnd != NULL)
    {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}
