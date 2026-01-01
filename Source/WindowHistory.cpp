#include "WindowHistory.h"
#include "WindowFinder.h"
#include <map>
#include <vector>

// 프로세스별 창 활성화 이력 (MRU - Most Recently Used)
static std::map<DWORD, std::vector<HWND>> g_ActivationHistory;
// 프로세스별 창 생성 순서 (열린 순서)
static std::map<DWORD, std::vector<HWND>> g_CreationHistory;
static HWINEVENTHOOK g_HistoryHook = NULL;
static HWINEVENTHOOK g_CreationHook = NULL;

// WinEvent hook 콜백: 창이 활성화될 때마다 호출됨
static VOID CALLBACK EventHookProcedure(
    HWINEVENTHOOK hook,
    DWORD event,
    HWND hwnd,
    LONG objectId,
    LONG childId,
    DWORD eventThread,
    DWORD eventTime
)
{
    if (hwnd == NULL || objectId != OBJID_WINDOW || childId != CHILDID_SELF)
    {
        return;
    }

    // 창의 프로세스 ID 가져오기
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0)
    {
        return;
    }

    // 최상위 창만 처리
    if (GetParent(hwnd) != NULL)
    {
        return;
    }

    // 보이지 않는 창 제외
    if (!IsWindowVisible(hwnd))
    {
        return;
    }

    // 제목이 없는 창 제외
    if (GetWindowTextLength(hwnd) == 0)
    {
        return;
    }

    // 활성화 이력 업데이트
    std::vector<HWND>& history = g_ActivationHistory[processId];

    // 이미 목록에 있으면 맨 앞으로 이동
    for (auto it = history.begin(); it != history.end(); ++it)
    {
        if (*it == hwnd)
        {
            history.erase(it);
            break;
        }
    }

    // 맨 앞에 추가 (가장 최근 활성화)
    history.insert(history.begin(), hwnd);

    // 최대 128개만 유지
    if (history.size() > 128)
    {
        history.resize(128);
    }
}

// 창 생성/표시 이벤트 콜백: 창이 처음 보일 때 호출됨
static VOID CALLBACK CreationEventHookProcedure(
    HWINEVENTHOOK hook,
    DWORD event,
    HWND hwnd,
    LONG objectId,
    LONG childId,
    DWORD eventThread,
    DWORD eventTime
)
{
    if (hwnd == NULL || objectId != OBJID_WINDOW || childId != CHILDID_SELF)
    {
        return;
    }

    // 창의 프로세스 ID 가져오기
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0)
    {
        return;
    }

    // 최상위 창만 처리
    if (GetParent(hwnd) != NULL)
    {
        return;
    }

    // 보이지 않는 창 제외
    if (!IsWindowVisible(hwnd))
    {
        return;
    }

    // 제목이 없는 창 제외
    if (GetWindowTextLength(hwnd) == 0)
    {
        return;
    }

    // 창 생성 이력 업데이트 (이미 있으면 추가하지 않음)
    std::vector<HWND>& creationHistory = g_CreationHistory[processId];
    
    // 이미 목록에 있는지 확인
    bool found = false;
    for (HWND existing : creationHistory)
    {
        if (existing == hwnd)
        {
            found = true;
            break;
        }
    }

    // 없으면 맨 뒤에 추가 (생성 순서 유지)
    if (!found)
    {
        creationHistory.push_back(hwnd);
    }
}

void InitWindowActivationTracking()
{
    // EVENT_SYSTEM_FOREGROUND: 포그라운드 창이 변경될 때
    g_HistoryHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        NULL,
        EventHookProcedure,
        0,
        0,
        WINEVENT_OUTOFCONTEXT
    );

    // EVENT_OBJECT_SHOW: 창이 처음 보일 때 (생성 순서 추적)
    g_CreationHook = SetWinEventHook(
        EVENT_OBJECT_SHOW,
        EVENT_OBJECT_SHOW,
        NULL,
        CreationEventHookProcedure,
        0,
        0,
        WINEVENT_OUTOFCONTEXT
    );

    // 초기 포그라운드 창 기록
    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow != NULL)
    {
        DWORD processId = 0;
        GetWindowThreadProcessId(foregroundWindow, &processId);
        if (processId != 0)
        {
            EventHookProcedure(NULL, EVENT_SYSTEM_FOREGROUND, foregroundWindow, OBJID_WINDOW, CHILDID_SELF, 0, 0);
        }
    }

    // 프로그램 시작 시 이미 존재하는 모든 창을 생성 이력에 추가
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (GetParent(hwnd) == NULL && IsWindowVisible(hwnd) && GetWindowTextLength(hwnd) > 0)
        {
            DWORD processId = 0;
            GetWindowThreadProcessId(hwnd, &processId);
            if (processId != 0)
            {
                CreationEventHookProcedure(NULL, EVENT_OBJECT_SHOW, hwnd, OBJID_WINDOW, CHILDID_SELF, 0, 0);
            }
        }
        return TRUE;
    }, 0);
}

void CleanupWindowActivationTracking()
{
    if (g_HistoryHook != NULL)
    {
        UnhookWinEvent(g_HistoryHook);
        g_HistoryHook = NULL;
    }
    if (g_CreationHook != NULL)
    {
        UnhookWinEvent(g_CreationHook);
        g_CreationHook = NULL;
    }
    g_ActivationHistory.clear();
    g_CreationHistory.clear();
}

std::vector<HWND> GetWindowsByActivationOrder(DWORD processId)
{
    // 생성 순서(열린 순서)로 창 목록 가져오기
    std::vector<HWND> result;
    
    if (g_CreationHistory.find(processId) != g_CreationHistory.end())
    {
        const std::vector<HWND>& creationHistory = g_CreationHistory[processId];
        
        // 현재 존재하는 창만 필터링 (생성 순서 유지)
        for (HWND hwnd : creationHistory)
        {
            if (IsWindow(hwnd))
            {
                DWORD hwndProcessId = 0;
                GetWindowThreadProcessId(hwnd, &hwndProcessId);
                if (hwndProcessId == processId)
                {
                    // 중복 제거
                    bool found = false;
                    for (HWND existing : result)
                    {
                        if (existing == hwnd)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        result.push_back(hwnd);
                    }
                }
            }
        }
    }

    // 생성 이력에 없는 창들은 FindWindowsByProcessId로 가져오기 (맨 뒤에 추가)
    std::vector<HWND> allWindows = FindWindowsByProcessId(processId);
    for (HWND hwnd : allWindows)
    {
        bool found = false;
        for (HWND existing : result)
        {
            if (existing == hwnd)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            result.push_back(hwnd);
        }
    }

    return result;
}

