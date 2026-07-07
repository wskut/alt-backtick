#pragma once

#include <windows.h>
#include <vector>

bool SwitchToNextWindow(DWORD processId);
bool SwitchToPreviousWindow(DWORD processId);

/// Bring an arbitrary window to the foreground, bypassing the foreground
/// lock via a synthetic mouse event.  Safe to call for any HWND.
bool BringWindowToForeground(HWND hwnd);

