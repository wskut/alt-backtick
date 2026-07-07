#pragma once

#include <windows.h>

/// Shared Alt-key state tracker.
/// Must be called from LowLevelKeyboardProc for every VK_MENU event.
/// Always returns false (Alt is never consumed).

bool HandleAltKey(DWORD vkCode, WPARAM wParam);

/// Returns true while the Alt key is physically held.
bool IsAltHeld();
