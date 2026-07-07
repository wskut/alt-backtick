#pragma once

#include <windows.h>
#include <string>
#include <vector>

/// One process with a visible, switchable window.
struct ProcessEntry
{
    DWORD processId;
    HWND  mainWindow;         ///< Highest-Z window for this PID
    std::string processName;  ///< "chrome.exe"
    std::string processPath;  ///< Full path, e.g. "C:\Program Files\..."
    std::string windowTitle;  ///< "My Tab — Google Chrome"
};

/// Enumerate all processes with visible windows, sorted by Z-order
/// (most recently active first), one entry per process.
std::vector<ProcessEntry> GetProcessList();
