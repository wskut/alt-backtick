#include "WindowFilter.h"

bool IsSwitchableWindow(HWND hwnd)
{
    if (GetParent(hwnd) != NULL)         return false;
    if (!IsWindowVisible(hwnd))          return false;
    if (GetWindowTextLengthW(hwnd) == 0) return false;

    wchar_t cls[256];
    if (GetClassNameW(hwnd, cls, 256) > 0)
    {
        if (wcscmp(cls, L"ApplicationFrameWindow") == 0) return false;
        if (wcscmp(cls, L"Progman") == 0)                return false;
        if (wcscmp(cls, L"WorkerW") == 0)                return false;
        if (wcscmp(cls, L"Shell_TrayWnd") == 0)          return false;
        if (wcscmp(cls, L"TaskListThumbnailWnd") == 0)   return false;
        if (wcscmp(cls, L"Windows.UI.Core.CoreWindow") == 0) return false;
    }
    return true;
}
