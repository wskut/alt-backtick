#include "AltKeyTracker.h"

static bool g_AltHeld = false;

bool HandleAltKey(DWORD vkCode, WPARAM wParam)
{
    if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_MENU)
    {
        bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool up   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);
        if (down)  g_AltHeld = true;
        if (up)    g_AltHeld = false;
    }
    // Alt is never consumed — always passes through.
    return false;
}

bool IsAltHeld()
{
    return g_AltHeld;
}
