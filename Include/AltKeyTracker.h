#pragma once

#include <windows.h>

/// Shared Alt-key state tracker.
/// Must be called from LowLevelKeyboardProc for every VK_MENU event.
/// Returns false unless the caller requested consuming the next Alt-up
/// (via AltConsumeNextUp) and this event is that Alt-up.

bool HandleAltKey(DWORD vkCode, WPARAM wParam);

/// Returns true while the Alt key is physically held.
bool IsAltHeld();

/// Mark the next Alt-up event for consumption.  Call after an immediate
/// Alt+key switch (Alt+Backtick) to prevent Alt-up from reaching the
/// newly-activated window.
void AltConsumeNextUp();
