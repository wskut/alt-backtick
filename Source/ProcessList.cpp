#include "ProcessList.h"
#include <algorithm>
#include <map>
#include <psapi.h>

// ---------------------------------------------------------------------------
// Window filter — same criteria used consistently across the project.
// ---------------------------------------------------------------------------
static bool IsSwitchableWindow(HWND hwnd)
{
    if (GetParent(hwnd) != NULL)         return false; // top-level only
    if (!IsWindowVisible(hwnd))          return false;
    if (GetWindowTextLengthW(hwnd) == 0) return false;

    wchar_t cls[256];
    if (GetClassNameW(hwnd, cls, 256) > 0)
    {
        if (wcscmp(cls, L"ApplicationFrameWindow") == 0) return false;
        if (wcscmp(cls, L"Progman") == 0)                return false;
        if (wcscmp(cls, L"WorkerW") == 0)                return false;
        if (wcscmp(cls, L"Shell_TrayWnd") == 0)          return false;
        if (wcscmp(cls, L"TaskListThumbnailWnd") == 0)   return false;
        if (wcscmp(cls, L"Windows.UI.Core.CoreWindow") == 0) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Z-order index for each HWND (0 = topmost)
// ---------------------------------------------------------------------------
static std::map<HWND, int> BuildZOrderMap()
{
    std::map<HWND, int> m;
    int idx = 0;
    for (HWND h = GetForegroundWindow(); h; h = GetWindow(h, GW_HWNDNEXT))
        if (IsSwitchableWindow(h))
            m[h] = idx++;
    return m;
}

// ---------------------------------------------------------------------------
// Build a PID → name+path map (cached for 3 seconds)
// ---------------------------------------------------------------------------
struct ProcessInfo { std::string name, path; };

static std::map<DWORD, ProcessInfo> BuildProcessInfoMap()
{
    static std::map<DWORD, ProcessInfo> cache;
    static DWORD lastTick = 0;
    DWORD now = GetTickCount();
    if (now - lastTick < 3000 && !cache.empty()) return cache;

    cache.clear();
    lastTick = now;

    DWORD pids[4096], needed = 0;
    if (!EnumProcesses(pids, sizeof(pids), &needed)) return cache;

    for (DWORD i = 0, n = needed / sizeof(DWORD); i < n; i++)
    {
        DWORD pid = pids[i];
        if (pid == 0 || pid == 4) continue;

        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!h) { h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid); if (!h) continue; }

        wchar_t buf[MAX_PATH]; DWORD sz = MAX_PATH;
        if (QueryFullProcessImageNameW(h, 0, buf, &sz))
        {
            std::wstring full(buf);
            cache[pid].path.assign(full.begin(), full.end());
            auto sep = full.find_last_of(L"\\/");
            if (sep != std::wstring::npos) full = full.substr(sep + 1);
            cache[pid].name.assign(full.begin(), full.end());
        }
        else
        {
            wchar_t base[MAX_PATH];
            if (GetModuleBaseNameW(h, NULL, base, MAX_PATH))
                cache[pid].name.assign(base, base + wcslen(base));
        }
        CloseHandle(h);
    }
    return cache;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
std::vector<ProcessEntry> GetProcessList()
{
    auto zOrder = BuildZOrderMap();

    // Keep the highest-Z (lowest index) HWND per PID
    struct Cand { HWND hwnd; int z; };
    std::map<DWORD, Cand> best;
    for (auto& [hw, z] : zOrder)
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(hw, &pid);
        if (!pid) continue;
        auto it = best.find(pid);
        if (it == best.end() || z < it->second.z) best[pid] = {hw, z};
    }

    auto infos = BuildProcessInfoMap();

    // Filter out unwanted processes by name
    auto isFiltered = [](const std::string& n) {
        static const char* filtered[] = {
            "TextInputHost.exe", "ShellExperienceHost.exe",
            "SearchUI.exe", "StartMenuExperienceHost.exe", nullptr
        };
        for (int i = 0; filtered[i]; i++)
            if (_stricmp(n.c_str(), filtered[i]) == 0) return true;
        return false;
    };

    std::vector<std::pair<int, ProcessEntry>> sorted;
    for (auto& [pid, c] : best)
    {
        ProcessEntry e;
        e.processId = pid;
        e.mainWindow = c.hwnd;
        auto it = infos.find(pid);
        e.processName = (it != infos.end()) ? it->second.name : "unknown";
        e.processPath = (it != infos.end()) ? it->second.path : "";
        if (isFiltered(e.processName)) continue;

        wchar_t title[256] = {};
        GetWindowTextW(c.hwnd, title, 256);
        e.windowTitle.assign(title, title + wcslen(title));

        sorted.emplace_back(c.z, e);
    }

    std::sort(sorted.begin(), sorted.end(),
              [](auto& a, auto& b) { return a.first < b.first; });

    std::vector<ProcessEntry> out;
    out.reserve(sorted.size());
    for (auto& [_, e] : sorted) out.push_back(std::move(e));
    return out;
}
