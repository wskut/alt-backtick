#include "Common.h"
#include "KeyboardHandler.h"
#include "WindowHistory.h"
#include "TrayIcon.h"
#include "ProcessSwitcher.h"
#include "Logger.h"

HHOOK g_KeyboardHook = NULL;

int main()
{
#ifdef DEBUG
    // 디버그 모드일 때는 콘솔 창을 띄워서 로깅 메시지를 볼 수 있게 함
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

    // UTF-8 콘솔 출력 설정
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 로그 출력 복구
    Logger::Info("=== AltBacktick ===");
    Logger::Info("Installing hooks...");
    Logger::Debug("Keyboard monitoring enabled");
    Logger::Info("Press Alt + ` to switch windows");
    Logger::Info("Press Alt + Tab to switch processes");

    // 트레이 아이콘 및 숨김 윈도우 초기화
    if (!InitTrayIcon())
    {
        return 0;
    }

    // 창 활성화 이력 추적 초기화
    InitWindowActivationTracking();
    InitProcessSwitcher();
    
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
        CleanupProcessSwitcher();
        CleanupWindowActivationTracking();
        CleanupTrayIcon();
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
    
    CleanupProcessSwitcher();
    CleanupWindowActivationTracking();
    CleanupTrayIcon();

    Logger::Info("Hooks uninstalled. Exiting...");

#ifdef DEBUG
    FreeConsole();
#endif

    return 0;
}
