#include "AltTracker.h"

static bool g_AltHeld = false;
static bool g_ConsumeNextUp = false;

bool HandleAltKey(DWORD vkCode, WPARAM wParam)
{
    if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_MENU)
    {
        bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool up   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

        if (down)
        {
            g_AltHeld = true;
            g_ConsumeNextUp = false; // clear stale flag
        }

        if (up)
            g_AltHeld = false;
    }
    // Never consume Alt — other handlers (ProcessSwitcher, backtick) must
    // also see every Alt event.
    return false;
}

bool IsAltHeld()
{
    return g_AltHeld;
}

void AltConsumeNextUp()
{
    g_ConsumeNextUp = true;
}

bool AltShouldConsumeUp()
{
    if (g_ConsumeNextUp)
    {
        g_ConsumeNextUp = false;
        return true;
    }
    return false;
}
