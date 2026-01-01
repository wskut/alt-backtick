#include "WindowFinder.h"

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

