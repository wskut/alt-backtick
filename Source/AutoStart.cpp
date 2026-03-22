#include "AutoStart.h"
#include <windows.h>
#include "Logger.h"

const char* REG_KEY_RUN = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const char* APP_NAME = "AltBacktick";

static std::string GetExePath() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        return std::string(path);
    }
    return "";
}

bool IsAutoStartEnabled() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY_RUN, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char value[MAX_PATH];
        DWORD valueSize = sizeof(value);
        if (RegQueryValueExA(hKey, APP_NAME, NULL, NULL, (LPBYTE)value, &valueSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        RegCloseKey(hKey);
    }
    return false;
}

bool EnableAutoStart() {
    std::string exePath = GetExePath();
    if (exePath.empty()) return false;

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY_RUN, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueExA(hKey, APP_NAME, 0, REG_SZ, (const BYTE*)exePath.c_str(), exePath.length() + 1) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            Logger::Info("AutoStart enabled.");
            return true;
        }
        RegCloseKey(hKey);
    }
    Logger::Error("Failed to enable AutoStart.");
    return false;
}

bool DisableAutoStart() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY_RUN, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegDeleteValueA(hKey, APP_NAME) == ERROR_SUCCESS || GetLastError() == ERROR_FILE_NOT_FOUND) {
            RegCloseKey(hKey);
            Logger::Info("AutoStart disabled.");
            return true;
        }
        RegCloseKey(hKey);
    }
    Logger::Error("Failed to disable AutoStart.");
    return false;
}
