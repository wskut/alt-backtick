#pragma once
#include <windows.h>

/// Returns true if \a hwnd is a valid, visible, switchable top-level window
/// that should appear in window/process switching lists.
bool IsSwitchableWindow(HWND hwnd);
