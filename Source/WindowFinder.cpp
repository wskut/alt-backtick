#include "WindowFinder.h"
#include <cstring>

struct EnumWindowsData
{
    DWORD TargetProcessId;
    std::vector<HWND> Windows;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);
    
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    
    if (processId == data->TargetProcessId)
    {
        // 최상위 창만 필터링 (자식 창 제외)
        if (GetParent(hwnd) != NULL)
        {
            return TRUE;
        }
        
        // 실제 렌더링 된 윈도우만 필터링
        if (!IsWindowVisible(hwnd))
        {
            return TRUE;
        }
        
        // 제목이 있는 윈도우만 필터링 (빈 창 제외)
        if (GetWindowTextLength(hwnd) == 0)
        {
            return TRUE;
        }
        
        // 특정 클래스명의 윈도우 제외
        char className[256];
        if (GetClassNameA(hwnd, className, sizeof(className)) > 0)
        {
            // ApplicationFrameWindow 제외 (Windows 10 UWP 앱 관련)
            if (strcmp(className, "ApplicationFrameWindow") == 0)
            {
                return TRUE;
            }
            
            // 바탕화면 창 제외
            if (strcmp(className, "Progman") == 0 || strcmp(className, "WorkerW") == 0)
            {
                return TRUE;
            }
            
            // 작업 표시줄 창 제외
            if (strcmp(className, "Shell_TrayWnd") == 0)
            {
                return TRUE;
            }
        }
        
        data->Windows.push_back(hwnd);
    }
    
    return TRUE;
}

DWORD GetActiveWindowProcessId()
{
    HWND hActiveWindow = GetForegroundWindow();
    if (hActiveWindow == NULL)
    {
        return 0;
    }
    
    DWORD processId = 0;
    GetWindowThreadProcessId(hActiveWindow, &processId);
    return processId;
}

std::vector<HWND> FindWindowsByProcessId(DWORD processId)
{
    EnumWindowsData data;
    data.TargetProcessId = processId;
    data.Windows.clear();
    
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    
    return data.Windows;
}
