#pragma once
#include <windows.h>

void InitProcessSwitcher();
void CleanupProcessSwitcher();
bool HandleProcessSwitcher(DWORD vkCode, WPARAM wParam);
