#include "KeyboardHandler.h"
#include "Common.h"
#include "WindowFinder.h"
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
        
        // Alt + ` 키 조합 감지
        bool isAltPressed = (GetAsyncKeyState(MODIFIER_KEY) & 0x8000) != 0;
        bool isBacktickPressed = (vkCode == TARGET_VIRTUAL_KEY);
        bool isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        
        if (isAltPressed && isBacktickPressed && isKeyDown)
        {
            Logger::Debug("[Alt + `] Detected!");
            
            DWORD activeProcessId = GetActiveWindowProcessId();
            
            if (activeProcessId != 0)
            {
                Logger::Debug("Active Process ID: " + std::to_string(activeProcessId));
            }
            else
            {
                Logger::Debug("Warning: Could not get active window PID");
            }
            
            // 키 입력 소비 (다른 앱으로 전달 방지)
            return 1;
        }
    }
    
    return CallNextHookEx(g_KeyboardHook, nCode, wParam, lParam);
}
