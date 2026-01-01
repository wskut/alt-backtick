#include "Common.h"
#include "KeyboardHandler.h"
#include "WindowHistory.h"
#include "Logger.h"

HHOOK g_KeyboardHook = NULL;

int main()
{
    // UTF-8 콘솔 출력 설정
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    Logger::Info("=== AltBacktick ===");
    Logger::Info("Installing hooks...");
    Logger::Debug("Keyboard monitoring enabled");
    Logger::Info("Press Alt + ` to switch windows");
    
    // 창 활성화 이력 추적 초기화
    InitWindowActivationTracking();
    
    // Low-level Keyboard Hook 설치
    g_KeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0
    );
    
    if (g_KeyboardHook == NULL)
    {
        Logger::Error("Failed to install keyboard hook. Error: " + std::to_string(GetLastError()));
        CleanupWindowActivationTracking();
        return 1;
    }
    
    Logger::Info("Hooks installed successfully!");
    Logger::Debug("Press any key to see the key code...");
    Logger::Info("Press Ctrl+C to exit.");
    Logger::Info("--------------------------------");
    
    // 메시지 루프
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Hook 해제
    if (g_KeyboardHook != NULL)
    {
        UnhookWindowsHookEx(g_KeyboardHook);
        g_KeyboardHook = NULL;
    }
    
    CleanupWindowActivationTracking();
    
    Logger::Info("Hooks uninstalled. Exiting...");
    return 0;
}
