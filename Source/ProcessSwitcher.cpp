#include "ProcessSwitcher.h"
#include "AltKeyTracker.h"
#include "ProcessList.h"
#include "ProcessOverlay.h"
#include "Logger.h"
#include <string>
#include <vector>

// -----------------------------------------------------------------------
// State machine
// -----------------------------------------------------------------------
enum SwitcherState
{
    STATE_IDLE,
    STATE_ALT_DOWN,
    STATE_SWITCHING
};

static SwitcherState           g_State        = STATE_IDLE;
static std::vector<ProcessEntry> g_ProcessList;
static size_t                    g_Selection  = 0;

// -----------------------------------------------------------------------
// Deferred-switch target (set in Alt-up, consumed in WM_DEFERRED_SWITCH)
// -----------------------------------------------------------------------
static ProcessEntry g_PendingTarget;

// -----------------------------------------------------------------------
// Message-only window for deferred switching
// -----------------------------------------------------------------------
static HWND          g_hSwitchWnd  = nullptr;
static const wchar_t SWITCH_WND_CLASS[] = L"AltBacktickSwitchWnd";

#define WM_DEFERRED_SWITCH  (WM_APP + 1)

// -----------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------
static void SwitchToProcess(const ProcessEntry& entry);
static void EnterAltTabMode();
static void AdvanceSelection();
static void RetreatSelection();

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------
static bool IsKeyDown(WPARAM wParam)
{
    return wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
}

static bool IsKeyUp(WPARAM wParam)
{
    return wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
}

// -----------------------------------------------------------------------
// SwitchToProcess -- activate a target window, bypassing the foreground
// lock.
// -----------------------------------------------------------------------
static void SwitchToProcess(const ProcessEntry& entry)
{
    HWND hwnd = entry.mainWindow;

    if (!IsWindow(hwnd))
    {
        Logger::Debug("SwitchToProcess: target window no longer exists");
        return;
    }

    // Already in the foreground -- nothing to do.
    if (GetForegroundWindow() == hwnd)
    {
        Logger::Debug("SwitchToProcess: target is already foreground");
        return;
    }

    // Restore if minimized, otherwise bring to front without stealing focus.
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }
    else
    {
        ShowWindow(hwnd, SW_SHOWNA);
    }

    // Bypass the foreground-lock restriction by injecting a harmless mouse
    // event.  Windows grants SetForegroundWindow to any thread that has
    // recently received input.
    {
        INPUT mouseInput = {};
        mouseInput.type = INPUT_MOUSE;
        SendInput(1, &mouseInput, sizeof(INPUT));
    }

    SetForegroundWindow(GetLastActivePopup(hwnd));

    Logger::Info("Switched to process " + entry.processName +
                 " (PID " + std::to_string(entry.processId) + ")");
}

// -----------------------------------------------------------------------
// EnterAltTabMode -- populate the process list and set initial selection
// to the entry *after* the current foreground process.
// -----------------------------------------------------------------------
static void EnterAltTabMode()
{
    g_ProcessList = GetProcessList();

    if (g_ProcessList.empty())
    {
        Logger::Debug("EnterAltTabMode: no switchable processes");
        g_Selection = 0;
        return;
    }

    // Find the current foreground PID in the list.
    DWORD foregroundPid = 0;
    HWND  foregroundHwnd = GetForegroundWindow();
    if (foregroundHwnd != nullptr)
    {
        GetWindowThreadProcessId(foregroundHwnd, &foregroundPid);
    }

    size_t currentIndex = 0;
    bool   found = false;

    for (size_t i = 0; i < g_ProcessList.size(); ++i)
    {
        if (g_ProcessList[i].processId == foregroundPid)
        {
            currentIndex = i;
            found = true;
            break;
        }
    }

    // Start at the next entry (wrap).  If we did not find the current
    // foreground entry we start at position 0.
    if (found)
    {
        g_Selection = (currentIndex + 1) % g_ProcessList.size();
    }
    else
    {
        g_Selection = 0;
    }

    Logger::Debug("EnterAltTabMode: " + std::to_string(g_ProcessList.size()) +
                  " processes, selection " + std::to_string(g_Selection));
}

// -----------------------------------------------------------------------
// Selection helpers
// -----------------------------------------------------------------------
static void AdvanceSelection()
{
    if (g_ProcessList.empty())
        return;
    g_Selection = (g_Selection + 1) % g_ProcessList.size();
}

static void RetreatSelection()
{
    if (g_ProcessList.empty())
        return;
    g_Selection = (g_Selection + g_ProcessList.size() - 1) % g_ProcessList.size();
}

// -----------------------------------------------------------------------
// Message-only window procedure
// -----------------------------------------------------------------------
static LRESULT CALLBACK SwitchWndProc(HWND hWnd, UINT msg,
                                      WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DEFERRED_SWITCH)
    {
        Logger::Debug("WM_DEFERRED_SWITCH: executing deferred switch");

        // Perform the actual process switch.
        SwitchToProcess(g_PendingTarget);

        // Clear the pending target so we never re-activate a stale entry.
        g_PendingTarget = ProcessEntry{};

        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// -----------------------------------------------------------------------
// InitProcessSwitcher
// -----------------------------------------------------------------------
void InitProcessSwitcher()
{
    g_State = STATE_IDLE;
    g_ProcessList.clear();
    g_Selection = 0;

    // Create the overlay (may already be created by the caller -- tolerate
    // double-init).
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    if (!CreateOverlayWindow(hInstance))
    {
        Logger::Error("InitProcessSwitcher: failed to create overlay window");
        // Continue anyway -- we can still function without the overlay.
    }

    // Register the message-only window class (idempotent).
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = SwitchWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = SWITCH_WND_CLASS;

    RegisterClassExW(&wc);

    // Create the message-only window.
    g_hSwitchWnd = CreateWindowExW(
        0,
        SWITCH_WND_CLASS,
        L"AltBacktick Switch Window",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,          // message-only window
        nullptr,
        hInstance,
        nullptr);

    if (g_hSwitchWnd == nullptr)
    {
        Logger::Error("InitProcessSwitcher: failed to create message-only window");
    }
    else
    {
        Logger::Debug("InitProcessSwitcher: message-only window created");
    }

    Logger::Info("ProcessSwitcher initialized");
}

// -----------------------------------------------------------------------
// CleanupProcessSwitcher
// -----------------------------------------------------------------------
void CleanupProcessSwitcher()
{
    // Hide and destroy the overlay.
    HideOverlay();
    DestroyOverlayWindow();

    // Destroy the message-only window.
    if (g_hSwitchWnd != nullptr)
    {
        DestroyWindow(g_hSwitchWnd);
        g_hSwitchWnd = nullptr;
    }

    // Reset state.
    g_State = STATE_IDLE;
    g_ProcessList.clear();
    g_Selection = 0;
    g_PendingTarget = ProcessEntry{};

    Logger::Info("ProcessSwitcher cleaned up");
}

// -----------------------------------------------------------------------
// HandleProcessSwitcher -- called from the low-level keyboard hook.
//
// Returns  true  if the key event should be consumed (not propagated to
//                other applications).
// Returns  false if the key event should pass through.
// -----------------------------------------------------------------------
bool HandleProcessSwitcher(DWORD vkCode, WPARAM wParam)
{
    // ------ Alt key (VK_MENU / VK_LMENU / VK_RMENU) ------
    if (vkCode == VK_MENU || vkCode == VK_LMENU || vkCode == VK_RMENU)
    {
        if (IsKeyDown(wParam) && g_State == STATE_IDLE)
        {
            g_State = STATE_ALT_DOWN;
            Logger::Debug("Alt down -> ALT_DOWN");
        }
        else if (IsKeyUp(wParam) && g_State == STATE_SWITCHING)
        {
            // Alt released while switching -- defer the switch.
            if (!g_ProcessList.empty() &&
                g_Selection < g_ProcessList.size())
            {
                g_PendingTarget = g_ProcessList[g_Selection];
            }

            HideOverlay();
            g_State = STATE_IDLE;
            g_ProcessList.clear();

            Logger::Info("Alt up in SWITCHING -> deferred switch to index " +
                         std::to_string(g_Selection));

            // Post the deferred-switch message to our message-only window.
            if (g_hSwitchWnd != nullptr)
            {
                PostMessageW(g_hSwitchWnd, WM_DEFERRED_SWITCH, 0, 0);
            }

            // Return false so the Alt-up event propagates through the hook
            // chain harmlessly.
            return false;
        }
        else if (IsKeyUp(wParam) && g_State == STATE_ALT_DOWN)
        {
            if (AltShouldConsumeUp())
            {
                // Immediate switch (Alt+Backtick) happened — consume Alt-up
                // so the newly-activated window doesn't receive it.
                g_State = STATE_IDLE;
                Logger::Debug("Alt up in ALT_DOWN -> consumed (immediate switch)");
                return true;
            }

            // Normal Alt release with no switch.
            g_State = STATE_IDLE;
            Logger::Debug("Alt up in ALT_DOWN -> IDLE");
            return false;
        }

        // Alt itself is never consumed.
        return false;
    }

    // ------ Tab key ------
    if (vkCode == VK_TAB)
    {
        // If we aren't tracking Alt, let Tab pass through.
        if (g_State == STATE_IDLE)
        {
            return false;
        }

        if (IsKeyDown(wParam))
        {
            if (g_State == STATE_ALT_DOWN)
            {
                // First Tab after Alt-down: enter switching mode.
                g_State = STATE_SWITCHING;

                // Cancel the original window's menu tracking.  The Alt-down
                // event already reached it and may have activated its menu
                // bar.  WM_CANCELMODE tells the window to dismiss that state,
                // exactly as Windows does internally for native Alt+Tab.
                SendMessageW(GetForegroundWindow(), WM_CANCELMODE, 0, 0);

                EnterAltTabMode();
                ShowOverlay(g_ProcessList, g_Selection);
                Logger::Debug("Tab down in ALT_DOWN -> SWITCHING");
            }
            else if (g_State == STATE_SWITCHING)
            {
                // Subsequent Tab press: advance / retreat selection.
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                {
                    RetreatSelection();
                    Logger::Debug("Tab + Shift -> retreat selection");
                }
                else
                {
                    AdvanceSelection();
                    Logger::Debug("Tab -> advance selection");
                }

                UpdateSelection(g_Selection);
            }
        }

        // Always consume Tab while Alt is held.
        return true;
    }

    // ------ Escape key ------
    if (vkCode == VK_ESCAPE && g_State == STATE_SWITCHING)
    {
        // Cancel the switch -- user dismissed the overlay.
        HideOverlay();
        g_State = STATE_ALT_DOWN;   // Alt key is likely still held.
        g_ProcessList.clear();

        Logger::Debug("Escape in SWITCHING -> cancel, back to ALT_DOWN");

        return true;
    }

    // All other keys pass through.
    return false;
}
