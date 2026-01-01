#include "WindowHistory.h"
#include "WindowFinder.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>

// 프로세스별 창 생성 순서 (열린 순서)
static std::map<DWORD, std::vector<HWND>> g_CreationHistory;
static HWINEVENTHOOK g_CreationHook = NULL;

// 창 유효성 검증 헬퍼 함수
static bool IsValidWindow(HWND hwnd)
{
    if (hwnd == NULL)
    {
        return false;
    }

    // 최상위 창만 처리
    if (GetParent(hwnd) != NULL)
    {
        return false;
    }

    // 보이지 않는 창 제외
    if (!IsWindowVisible(hwnd))
    {
        return false;
    }

    // 제목이 없는 창 제외
    if (GetWindowTextLength(hwnd) == 0)
    {
        return false;
    }

    // 특정 클래스명의 윈도우 제외
    char className[256];
    if (GetClassNameA(hwnd, className, sizeof(className)) > 0)
    {
        // ApplicationFrameWindow 제외 (Windows 10 UWP 앱 관련)
        if (strcmp(className, "ApplicationFrameWindow") == 0)
        {
            return false;
        }

        // 바탕화면 창 제외
        if (strcmp(className, "Progman") == 0 || strcmp(className, "WorkerW") == 0)
        {
            return false;
        }

        // 작업 표시줄 창 제외
        if (strcmp(className, "Shell_TrayWnd") == 0)
        {
            return false;
        }
    }

    return true;
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
    if (objectId != OBJID_WINDOW || childId != CHILDID_SELF || !IsValidWindow(hwnd))
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

    // 창 생성 이력 업데이트 (이미 있으면 추가하지 않음)
    std::vector<HWND>& creationHistory = g_CreationHistory[processId];
    
    // 이미 목록에 있는지 확인
    if (std::find(creationHistory.begin(), creationHistory.end(), hwnd) == creationHistory.end())
    {
        // 없으면 맨 뒤에 추가 (생성 순서 유지)
        creationHistory.push_back(hwnd);
    }
}

void InitWindowActivationTracking()
{
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

    // 프로그램 시작 시 이미 존재하는 모든 창을 생성 이력에 추가
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (IsValidWindow(hwnd))
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
    if (g_CreationHook != NULL)
    {
        UnhookWinEvent(g_CreationHook);
        g_CreationHook = NULL;
    }
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
                if (hwndProcessId == processId && std::find(result.begin(), result.end(), hwnd) == result.end())
                {
                    result.push_back(hwnd);
                }
            }
        }
    }

    // 생성 이력에 없는 창들은 FindWindowsByProcessId로 가져오기 (맨 뒤에 추가)
    std::vector<HWND> allWindows = FindWindowsByProcessId(processId);
    for (HWND hwnd : allWindows)
    {
        if (std::find(result.begin(), result.end(), hwnd) == result.end())
        {
            result.push_back(hwnd);
        }
    }

    return result;
}

