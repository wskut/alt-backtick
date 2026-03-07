#pragma once

#include <windows.h>
#include <vector>

DWORD GetActiveWindowProcessId();
std::vector<HWND> FindWindowsByProcessId(DWORD processId);

