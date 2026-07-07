#pragma once
#include <windows.h>

void InitProcessSwitcher();
void CleanupProcessSwitcher();
bool HandleProcessSwitcher(DWORD vkCode, WPARAM wParam);

/// Tell ProcessSwitcher to consume the next Alt-up event.  Used after an
/// immediate switch (Alt+Backtick) so the Alt-up doesn't reach the newly
/// activated window.
void ConsumeNextAltUp();
