#include "WindowFinder.h"
#include <cstring>
#include "WindowFilter.h"

struct EnumWindowsData
{
    DWORD TargetProcessId;
    std::vector<HWND> Windows;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    if (!IsSwitchableWindow(hwnd))
        return TRUE; // skip but keep enumerating

    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    if (processId == data->TargetProcessId)
        data->Windows.push_back(hwnd);

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
