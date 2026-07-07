#include "KeyboardHandler.h"
#include "Common.h"
#include "WindowFinder.h"
#include "WindowSwitcher.h"
#include "ProcessSwitcher.h"
#include "AltKeyTracker.h"
#include "Logger.h"
#include <iostream>

extern HHOOK g_KeyboardHook;

#ifdef DEBUG
#include "KeyboardMonitor.h"
#endif

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboardStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        DWORD vkCode = pKeyboardStruct->vkCode;

#ifdef DEBUG
        // 키 입력 모니터링 출력
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            std::cout << "[KEY DOWN] VK_CODE=" << vkCode << " (" << GetKeyName(vkCode) << ")" << std::endl;
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        {
            std::cout << "[KEY UP]   VK_CODE=" << vkCode << " (" << GetKeyName(vkCode) << ")" << std::endl;
        }
#endif

        // Shared Alt key tracking — feeds IsAltHeld() for all handlers below.
        HandleAltKey(vkCode, wParam);

        // Alt+Tab → process-level window switching
        if (HandleProcessSwitcher(vkCode, wParam))
            return 1;

        // Alt + ` (backtick) → within-process window switching
        bool isAltHeld      = IsAltHeld();
        bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool isBacktick     = (vkCode == TARGET_VIRTUAL_KEY);
        bool isKeyDown      = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

        if (isAltHeld && isBacktick && isKeyDown)
        {
            // Same as Alt+Tab: dismiss the active window's menu bar before
            // switching, so the target window receives clean focus.
            SendMessageW(GetForegroundWindow(), WM_CANCELMODE, 0, 0);

            // Mark the next Alt-up for consumption so it doesn't reach the
            // newly-activated window (unlike Alt+Tab, this switch is immediate).
            ConsumeNextAltUp();

            DWORD activeProcessId = GetActiveWindowProcessId();

            if (activeProcessId != 0)
            {
                if (isShiftPressed)
                {
                    Logger::Debug("[Alt + Shift + `] Detected!");
                    SwitchToPreviousWindow(activeProcessId);
                }
                else
                {
                    Logger::Debug("[Alt + `] Detected!");
                    SwitchToNextWindow(activeProcessId);
                }
            }
            else
            {
                Logger::Debug("Warning: Could not get active window PID");
            }

            return 1;
        }
    }

    return CallNextHookEx(g_KeyboardHook, nCode, wParam, lParam);
}
