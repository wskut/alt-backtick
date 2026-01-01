#pragma once

#include <windows.h>
#include <vector>

void InitWindowActivationTracking();
void CleanupWindowActivationTracking();
std::vector<HWND> GetWindowsByActivationOrder(DWORD processId);

