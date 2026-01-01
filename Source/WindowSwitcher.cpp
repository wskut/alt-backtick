#include "WindowSwitcher.h"
#include "WindowFinder.h"
#include "Logger.h"

bool SwitchToNextWindow(DWORD processId)
{
    // 현재 활성 창 찾기
    HWND currentWindow = GetForegroundWindow();
    
    // 동일 PID의 모든 윈도우 찾기
    std::vector<HWND> windows = FindWindowsByProcessId(processId);
    
    if (windows.empty())
    {
        Logger::Debug("No windows found for this process");
        return false;
    }
    
    // 현재 활성 창의 인덱스 찾기
    size_t currentIndex = 0;
    bool found = false;
    for (size_t i = 0; i < windows.size(); i++)
    {
        if (windows[i] == currentWindow)
        {
            currentIndex = i;
            found = true;
            break;
        }
    }
    
    // 다음 창 인덱스 계산 (순환)
    size_t nextIndex = found ? (currentIndex + 1) % windows.size() : 0;
    HWND nextWindow = windows[nextIndex];
    
    // 다음 창으로 전환
    if (GetForegroundWindow() == nextWindow)
    {
        Logger::Debug("Window is already foreground");
        return true;
    }
    
    // SetForegroundWindow 제한 우회
    // Windows는 사용자가 "직접" 입력했을 때만 다른 프로세스의 창을 포그라운드로 만들 수 있도록 제한
    // 빈 마우스 입력을 통해 "last input time" 갱신, 제한 우회
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    SendInput(1, &mouseInput, sizeof(INPUT));
    
    // 창 최소화 상태 확인 및 복원
    if (IsIconic(nextWindow))
    {
        ShowWindow(nextWindow, SW_RESTORE);
    }
    else
    {
        ShowWindow(nextWindow, SW_SHOWNA);
    }
    
    // 모달 다이얼로그가 있는 경우 올바른 창 활성화
    HWND targetWindow = GetLastActivePopup(nextWindow);
    if (SetForegroundWindow(targetWindow))
    {
        Logger::Debug("Switched to window " + std::to_string(nextIndex + 1) + " of " + std::to_string(windows.size()));
        return true;
    }
    else
    {
        Logger::Debug("Failed to switch window. Error: " + std::to_string(GetLastError()));
        return false;
    }
}

