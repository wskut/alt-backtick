#include "AltKeyTracker.h"

static bool g_AltHeld = false;
static bool g_ConsumeNextUp = false; // one-shot after immediate switch

bool HandleAltKey(DWORD vkCode, WPARAM wParam)
{
    if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_MENU)
    {
        bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool up   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

        if (down)
        {
            g_AltHeld = true;
            // Clear stale consume flag from prior Alt press
            g_ConsumeNextUp = false;
        }

        if (up)
        {
            g_AltHeld = false;
            if (g_ConsumeNextUp)
            {
                g_ConsumeNextUp = false;
                return true; // consumed — new window never sees it
            }
        }
    }
    return false; // normally never consumed
}

bool IsAltHeld()
{
    return g_AltHeld;
}

void AltConsumeNextUp()
{
    g_ConsumeNextUp = true;
}
