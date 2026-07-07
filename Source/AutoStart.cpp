#include "AutoStart.h"
#include "Logger.h"
#include <windows.h>

static const char* TASK_NAME = "AltBacktick";

static std::string GetExePath()
{
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH))
        return std::string(path);
    return "";
}

/// Run schtasks.exe with the given arguments and return true on success.
static bool RunSchTasks(const std::string& args)
{
    std::string cmd = "schtasks " + args;

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return false;

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return code == 0;
}

bool IsAutoStartEnabled()
{
    return RunSchTasks(std::string("/query /tn \"") + TASK_NAME + "\" /nh");
}

bool EnableAutoStart()
{
    std::string exe = GetExePath();
    if (exe.empty())
    {
        Logger::Error("AutoStart: could not get exe path.");
        return false;
    }

    std::string args = std::string("/create /sc onlogon /tn \"") + TASK_NAME +
                       "\" /tr \"" + exe + "\" /rl highest /f";

    if (RunSchTasks(args))
    {
        Logger::Info("AutoStart enabled via Task Scheduler (admin on logon).");
        return true;
    }

    Logger::Error("AutoStart: schtasks /create failed.");
    return false;
}

bool DisableAutoStart()
{
    if (RunSchTasks(std::string("/delete /tn \"") + TASK_NAME + "\" /f"))
    {
        Logger::Info("AutoStart disabled.");
        return true;
    }

    Logger::Error("AutoStart: schtasks /delete failed.");
    return false;
}
