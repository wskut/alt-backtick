#include "WindowSwitcher.h"
#include "WindowHistory.h"
#include "Logger.h"
#include <map>

// 프로세스별로 현재 인덱스만 기억 (창 목록은 활성화 순서로 가져옴)
struct ProcessWindowState
{
    size_t currentIndex;
};

static std::map<DWORD, ProcessWindowState> g_ProcessStates;

// UWP 앱의 경우 부모 창을 반환, 일반 창은 그대로 반환
static HWND GetAppHost(HWND hwnd)
{
    return hwnd;
}

static void ShowWindowX(HWND hwnd)
{
    if (GetForegroundWindow() != hwnd)
    {
        if (IsIconic(hwnd))
        {
            ShowWindow(hwnd, SW_RESTORE);
        }
        else
        {
            ShowWindow(hwnd, SW_SHOWNA);
        }
        SetForegroundWindow(GetLastActivePopup(GetAppHost(hwnd)));
    }
}

bool SwitchToNextWindow(DWORD processId)
{
    // 활성화 순서로 창 목록 가져오기 (Windows의 실행 순서)
    std::vector<HWND> windows = GetWindowsByActivationOrder(processId);
    
    if (windows.empty())
    {
        Logger::Debug("No windows found for this process");
        return false;
    }
    
    // 프로세스 상태 가져오기 또는 초기화
    ProcessWindowState& state = g_ProcessStates[processId];
    
    // 현재 포그라운드 창 찾기
    HWND currentForeground = GetForegroundWindow();
    bool found = false;
    
    for (size_t i = 0; i < windows.size(); i++)
    {
        HWND window = windows[i];
        HWND activePopup = GetLastActivePopup(GetAppHost(window));
        if (window == currentForeground || activePopup == currentForeground)
        {
            state.currentIndex = i;
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        // 현재 창을 찾지 못했으면 첫 번째 창부터 시작
        state.currentIndex = 0;
    }
    
#ifdef DEBUG
    Logger::Debug("Found " + std::to_string(windows.size()) + " windows (by activation order)");
    for (size_t i = 0; i < windows.size(); i++)
    {
        char title[256];
        GetWindowTextA(windows[i], title, sizeof(title));
        Logger::Debug("  Window " + std::to_string(i) + ": HWND=" + std::to_string((long long)windows[i]) + " Title=" + std::string(title));
    }
    char currentTitle[256];
    GetWindowTextA(windows[state.currentIndex], currentTitle, sizeof(currentTitle));
    Logger::Debug("Current window: index " + std::to_string(state.currentIndex) + " Title=" + std::string(currentTitle));
#endif
    
    // 다음 창 인덱스 계산 (순환)
    size_t nextIndex = (state.currentIndex + 1) % windows.size();
    HWND nextWindow = windows[nextIndex];
    
    // 현재 인덱스 업데이트
    state.currentIndex = nextIndex;
    
#ifdef DEBUG
    char nextTitle[256];
    GetWindowTextA(nextWindow, nextTitle, sizeof(nextTitle));
    Logger::Debug("Switching to window " + std::to_string(nextIndex) + ": " + std::string(nextTitle));
#endif
    
    // SetForegroundWindow 제한 우회
    // Windows는 사용자가 "직접" 입력했을 때만 다른 프로세스의 창을 포그라운드로 만들 수 있도록 제한
    // 빈 마우스 입력을 통해 "last input time" 갱신, 제한 우회
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    SendInput(1, &mouseInput, sizeof(INPUT));
    
    // 창 전환
    ShowWindowX(nextWindow);
    
    Logger::Debug("Switched to window " + std::to_string(nextIndex + 1) + " of " + std::to_string(windows.size()));
    return true;
}

bool SwitchToPreviousWindow(DWORD processId)
{
    // 생성 순서(열린 순서)로 창 목록 가져오기
    std::vector<HWND> windows = GetWindowsByActivationOrder(processId);
    
    if (windows.empty())
    {
        Logger::Debug("No windows found for this process");
        return false;
    }
    
    // 프로세스 상태 가져오기 또는 초기화
    ProcessWindowState& state = g_ProcessStates[processId];
    
    // 현재 포그라운드 창 찾기
    HWND currentForeground = GetForegroundWindow();
    bool found = false;
    
    for (size_t i = 0; i < windows.size(); i++)
    {
        HWND window = windows[i];
        HWND activePopup = GetLastActivePopup(GetAppHost(window));
        if (window == currentForeground || activePopup == currentForeground)
        {
            state.currentIndex = i;
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        // 현재 창을 찾지 못했으면 마지막 창부터 시작
        state.currentIndex = windows.size() - 1;
    }
    
#ifdef DEBUG
    Logger::Debug("Found " + std::to_string(windows.size()) + " windows (by activation order)");
    for (size_t i = 0; i < windows.size(); i++)
    {
        char title[256];
        GetWindowTextA(windows[i], title, sizeof(title));
        Logger::Debug("  Window " + std::to_string(i) + ": HWND=" + std::to_string((long long)windows[i]) + " Title=" + std::string(title));
    }
    char currentTitle[256];
    GetWindowTextA(windows[state.currentIndex], currentTitle, sizeof(currentTitle));
    Logger::Debug("Current window: index " + std::to_string(state.currentIndex) + " Title=" + std::string(currentTitle));
#endif
    
    // 이전 창 인덱스 계산 (역순환)
    size_t prevIndex = (state.currentIndex == 0) ? windows.size() - 1 : state.currentIndex - 1;
    HWND prevWindow = windows[prevIndex];
    
    // 현재 인덱스 업데이트
    state.currentIndex = prevIndex;
    
#ifdef DEBUG
    char prevTitle[256];
    GetWindowTextA(prevWindow, prevTitle, sizeof(prevTitle));
    Logger::Debug("Switching to window " + std::to_string(prevIndex) + ": " + std::string(prevTitle));
#endif
    
    // SetForegroundWindow 제한 우회
    // Windows는 사용자가 "직접" 입력했을 때만 다른 프로세스의 창을 포그라운드로 만들 수 있도록 제한
    // 빈 마우스 입력을 통해 "last input time" 갱신, 제한 우회
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    SendInput(1, &mouseInput, sizeof(INPUT));
    
    // 창 전환
    ShowWindowX(prevWindow);
    
    Logger::Debug("Switched to window " + std::to_string(prevIndex + 1) + " of " + std::to_string(windows.size()));
    return true;
}

