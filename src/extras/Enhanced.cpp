#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "Enhanced.h"
#include "Frontend.h"
#include "frontendoption.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "FileMgr.h"
#include "Game.h"
#include "GenericGameStorage.h"
#include "Hud.h"
#include "Pad.h"
#include "PathFind.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Radar.h"
#include "Replay.h"
#include "Script.h"
#include "Sprite2d.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Timer.h"
#include "TxdStore.h"
#include "Wanted.h"
#include "World.h"

namespace Enhanced {
int8 QuickSave = 0, AutoSave = 0, FirstPerson = 0, RadarZoom = 0;
int8 GPS = 0, MenuMap = 1, Ragdoll = 0, VehicleDetail = 0, PedDetail = 0, HealAfterSave = 0;
const char *AutoSaveTimes[] = { "FEM_OFF", "EN_5MIN", "EN10MIN", "EN15MIN" };
const char *RadarZoomModes[] = { "FEM_OFF", "EN_ZOOM", "EN_WIDE" };

static bool firstPersonActive;
static uint32 elapsedSafeTime, previousTime;
static int32 lastMissions = -1;
static bool missionSavePending;
static CVector route[512];
static int16 routeCount;
static uint32 routeLastUpdate;
static CVector routeTarget;
static CVector routeStart;

// Local strings keep the GXT files intact, including all existing translations.
wchar *GetText(const char *key)
{
    struct Entry { const char *key; const char *value; wchar wide[120]; };
    static Entry strings[] = {
        { "ENHANCE", "ENHANCED OPTIONS" },
        { "EN_QSAV", "F5 SAVE MENU" },
        { "EN_AUTO", "AUTOSAVE" },
        { "EN_LOAD", "LOAD AUTOSAVE" },
        { "EN_FPS", "FIRST PERSON (F6)" },
        { "EN_RADR", "RADAR ZOOM" },
        { "EN_GPS", "GPS ROUTE" },
        { "EN_MAP", "PAUSE MAP" },
        { "EN_RAG", "RAGDOLL (BASIC)" },
        { "EN_VLOD", "HIGH DETAIL VEHICLES" },
        { "EN_PLOD", "HIGH DETAIL PEDESTRIANS" },
        { "EN_HEAL", "HEAL AFTER MANUAL SAVE" },
        { "EN_5MIN", "5 MIN" }, { "EN10MIN", "10 MIN" }, { "EN15MIN", "15 MIN" },
        { "EN_ZOOM", "HOLD TAB" }, { "EN_WIDE", "WIDE + TAB" },
        { "EN_DONE", "Autosave completed." },
        { "EN_FAIL", "Autosave failed. Previous autosave kept." },
        { "EN_SAFE", "Save unavailable: stand still, outside missions, without a wanted level." },
        { "EN_HINT", "F6: first person. TAB: radar zoom. Map: right click to set waypoint." },
    };
    for (unsigned i = 0; i < ARRAY_SIZE(strings); ++i) {
        if (strncmp(strings[i].key, key, 8) == 0) {
            if (!strings[i].wide[0]) AsciiToUnicode(strings[i].value, strings[i].wide);
            return strings[i].wide;
        }
    }
    return nil;
}

static bool GameReady()
{
    CPlayerPed *p = FindPlayerPed();
    return p && !FrontEndMenuManager.m_bGameNotLoaded &&
        !FrontEndMenuManager.m_bWantToRestart && !FrontEndMenuManager.m_bWantToLoad &&
        !FrontEndMenuManager.m_bMenuActive && !FrontEndMenuManager.m_bSaveMenuActive &&
        !CTimer::GetIsPaused() && !CCutsceneMgr::IsRunning() &&
        !CCutsceneMgr::IsCutsceneProcessing() && !TheCamera.m_WideScreenOn &&
        !CReplay::IsPlayingBack() && p->m_fHealth > 0.0f;
}

static bool SafeToSave()
{
    if (!GameReady() || CTheScripts::IsPlayerOnAMission()) return false;
    CPlayerPed *p = FindPlayerPed();
    CPlayerInfo *info = p->GetPlayerInfoForThisPlayerPed();
    return info && info->m_WBState == WBSTATE_PLAYING && !info->m_bInRemoteMode &&
        !p->InVehicle() && p->IsPedInControl() && p->bIsStanding &&
        p->GetPedState() == PED_IDLE && p->m_vecMoveSpeed.MagnitudeSqr() < 0.0001f &&
        (!p->m_pWanted || p->m_pWanted->GetWantedLevel() == 0) &&
        TheCamera.GetScreenFadeStatus() == FADE_0;
}

void Reset()
{
    elapsedSafeTime = previousTime = 0;
    lastMissions = -1;
    missionSavePending = firstPersonActive = false;
    routeCount = 0;
    routeLastUpdate = 0;
}

bool UseFirstPerson()
{
    CPlayerPed *p = FindPlayerPed();
    return FirstPerson && firstPersonActive && p && !p->InVehicle() &&
        p->IsPedInControl() && !CCutsceneMgr::IsRunning() &&
        !CCutsceneMgr::IsCutsceneProcessing() && !TheCamera.m_WideScreenOn &&
        TheCamera.WhoIsInControlOfTheCamera != CAMCONTROL_SCRIPT;
}

void AfterSuccessfulSave()
{
    // Only manual saves call this; autosave does not become periodic healing.
    if (HealAfterSave && FindPlayerPed())
        FindPlayerPed()->m_fHealth = Max(FindPlayerPed()->m_fHealth, 100.0f);
}

static void AutosavePrefix(char *out, size_t size)
{
    snprintf(out, size, "%s-enhanced-auto", DefaultPCSaveFileName);
}

bool ValidateAutosave(char *destination)
{
    // Reuse the native checksum validator with an isolated save prefix.
    // Eight manual slots and mission-retry saves are never overwritten.
    char original[sizeof(DefaultPCSaveFileName)];
    strcpy(original, DefaultPCSaveFileName);
    char prefix[sizeof(DefaultPCSaveFileName)];
    AutosavePrefix(prefix, sizeof(prefix));
    strcpy(DefaultPCSaveFileName, prefix);
    bool result = CheckDataNotCorrupt(0, destination);
    strcpy(DefaultPCSaveFileName, original);
    return result;
}

static bool SaveAutosave()
{
    if (!CStats::LastMissionPassedName[0]) return false;
    char prefix[260], target[300], temporary[304];
    AutosavePrefix(prefix, sizeof(prefix));
    snprintf(target, sizeof(target), "%s1.b", prefix);
    snprintf(temporary, sizeof(temporary), "%s.tmp", target);
    char oldValidName[sizeof(ValidSaveName)];
    strcpy(oldValidName, ValidSaveName);
    // Same native serializer, but no safehouse time skip or stamina mutation.
    // The old autosave is replaced only after writing and closing successfully.
    strncpy(ValidSaveName, temporary, sizeof(ValidSaveName)-1);
    ValidSaveName[sizeof(ValidSaveName)-1] = '\0';
    int file = CFileMgr::OpenFile(temporary, "wb");
    bool ok = false;
    if (file) {
        CTimer::Stop();
        ok = GenericSave(file);
        if (CFileMgr::GetErrorReadWrite(file)) ok = false;
        if (CFileMgr::CloseFile(file) != 0) ok = false;
        CTimer::Update();
    }
    strcpy(ValidSaveName, oldValidName);
    if (ok) {
#ifdef _WIN32
        ok = MoveFileExA(temporary, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        ok = rename(temporary, target) == 0;
#endif
    }
    if (!ok) remove(temporary);
    return ok;
}

void LoadAutosave(int8 action)
{
    if (action != FEOPTION_ACTION_SELECT) return;
    char name[300];
    if (!ValidateAutosave(name)) {
        FrontEndMenuManager.SetHelperText(1);
        return;
    }
    FrontEndMenuManager.m_nCurrSaveSlot = -2; // private sentinel, never an array index
    ChangeScreen(MENUPAGE_LOAD_SLOT_CONFIRM);
}

wchar *MapOptionDraw(bool *disabled, bool)
{
    *disabled = !MenuMap;
    return nil;
}

void OpenMap(int8 action)
{
    if (action == FEOPTION_ACTION_SELECT && MenuMap)
        ChangeScreen(MENUPAGE_MAP);
}

static void UpdateRoute()
{
#ifdef MENU_MAP
    if (!GPS || CRadar::TargetMarkerId == -1 || !FindPlayerPed()) {
        routeCount = 0;
        routeLastUpdate = 0;
        return;
    }
    const CVector start = FindPlayerCoors();
    const CVector target = CRadar::TargetMarkerPos;
    if ((target-start).Magnitude2D() < 12.0f) {
        CRadar::ToggleTargetMarker(target.x, target.y);
        routeCount = 0;
        return;
    }
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (routeLastUpdate && now-routeLastUpdate < 1000 &&
        (target-routeTarget).MagnitudeSqr() < 1.0f &&
        (start-routeStart).MagnitudeSqr() < 225.0f) return;
    routeLastUpdate = now;
    routeTarget = target;
    routeStart = start;
    routeCount = 0;
    const int type = FindPlayerVehicle() ? PATH_CAR : PATH_PED;
    int startNode = ThePaths.FindNodeClosestToCoors(start, type, 250.0f, true);
    int targetNode = ThePaths.FindNodeClosestToCoors(target, type, 250.0f, true);
    if (startNode < 0 || targetNode < 0) return;
    CPathNode *nodes[ARRAY_SIZE(route)];
    int16 count = 0;
    ThePaths.DoPathSearch(type, start, startNode, target, nodes, &count,
        ARRAY_SIZE(nodes), nil, nil, 999999.0f, targetNode);
    for (int i = 0; i < count; ++i) route[routeCount++] = nodes[i]->GetPosition();
#endif
}

void WaypointChanged()
{
    routeLastUpdate = 0;
    UpdateRoute();
}

void Process()
{
    // Also clamp settings loaded from edited ini files before using as indexes.
    AutoSave = Clamp(AutoSave, (int8)0, (int8)3);
    RadarZoom = Clamp(RadarZoom, (int8)0, (int8)2);
    const uint32 now = CTimer::GetTimeInMilliseconds();
    const uint32 dt = previousTime ? Min(now-previousTime, 100u) : 0;
    previousTime = now;
    if (!FirstPerson) firstPersonActive = false;
    if (!GameReady()) return;
    if (FirstPerson && CPad::GetPad(0)->GetFJustDown(5))
        firstPersonActive = !firstPersonActive;
    if (QuickSave && CPad::GetPad(0)->GetFJustDown(4)) {
        if (SafeToSave()) FrontEndMenuManager.m_bSaveMenuActive = true;
        else CHud::SetHelpMessage(GetText("EN_SAFE"), false);
    }
    if (lastMissions < 0) lastMissions = CStats::MissionsPassed;
    if (CStats::MissionsPassed > lastMissions) missionSavePending = true;
    lastMissions = CStats::MissionsPassed;
    if (!AutoSave) {
        elapsedSafeTime = 0;
        missionSavePending = false;
    } else {
        elapsedSafeTime += dt;
        const uint32 intervals[] = {0, 300000, 600000, 900000};
        if ((elapsedSafeTime >= intervals[AutoSave] || missionSavePending) && SafeToSave()) {
            const bool ok = SaveAutosave();
            CHud::SetHelpMessage(GetText(ok ? "EN_DONE" : "EN_FAIL"), false);
            elapsedSafeTime = 0;
            missionSavePending = false;
        }
    }
    UpdateRoute();
}

// Clip route segments mathematically, rather than clamping offscreen points
// onto the radar's rim (which would draw a false circle around the minimap).
static bool ClipRadarSegment(CVector2D &a, CVector2D &b)
{
    const CVector2D delta = b-a;
    const float A = delta.x*delta.x + delta.y*delta.y;
    if (A < 0.000001f) return false;
    const float B = 2.0f*(a.x*delta.x+a.y*delta.y);
    const float C = a.x*a.x+a.y*a.y-0.94f*0.94f;
    const float discriminant = B*B-4.0f*A*C;
    if (discriminant < 0.0f) return false;
    const float root = Sqrt(discriminant);
    const float low = Max(0.0f, (-B-root)/(2.0f*A));
    const float high = Min(1.0f, (-B+root)/(2.0f*A));
    if (high <= low) return false;
    b = a + delta*high;
    a += delta*low;
    return true;
}

void DrawRoute()
{
#ifdef MENU_MAP
    if (!GPS || routeCount < 2) return;
    for (int i = 1; i < routeCount; ++i) {
        CVector2D a, b, sa, sb;
        CRadar::TransformRealWorldPointToRadarSpace(a, CVector2D(route[i-1]));
        CRadar::TransformRealWorldPointToRadarSpace(b, CVector2D(route[i]));
        if (!CMenuManager::bMenuMapActive && !ClipRadarSegment(a,b)) continue;
        CRadar::TransformRadarPointToScreenSpace(sa,a);
        CRadar::TransformRadarPointToScreenSpace(sb,b);
        CVector2D d = sb-sa;
        float length = d.Magnitude();
        if (length < 0.1f) continue;
        CVector2D perpendicular(-d.y/length, d.x/length);
        perpendicular *= SCREEN_SCALE_X(1.5f);
        CSprite2d::Draw2DPolygon(sa.x+perpendicular.x,sa.y+perpendicular.y,
            sb.x+perpendicular.x,sb.y+perpendicular.y,
            sa.x-perpendicular.x,sa.y-perpendicular.y,
            sb.x-perpendicular.x,sb.y-perpendicular.y, CRGBA(240,180,40,220));
    }
#endif
}

void DrawMapFallback()
{
#ifdef MENU_MAP
    // Built-in menu.txd map art is optional. Draw the actual radar tiles when
    // it is absent, using the same world bounds and waypoint coordinates.
    const float tile = CMenuManager::fMapSize * 2.0f / 8.0f;
    bool missing = false;
    for (int y=0; y<8; ++y) for (int x=0; x<8; ++x) {
        char name[16];
        snprintf(name,sizeof(name),"radar%02d",x+y*8);
        int slot = CTxdStore::FindTxdSlot(name);
        if (slot < 0) continue;
        if (!CTxdStore::isTxdLoaded(slot)) missing = true;
        CStreaming::RequestTxd(slot, STREAMFLAGS_DONT_REMOVE);
    }
    if (missing) CStreaming::LoadAllRequestedModels(false);
    CTxdStore::PushCurrentTxd();
    for (int y=0; y<8; ++y) for (int x=0; x<8; ++x) {
        char name[16];
        snprintf(name,sizeof(name),"radar%02d",x+y*8);
        int slot = CTxdStore::FindTxdSlot(name);
        if (slot < 0 || !CTxdStore::GetSlot(slot)->texDict) continue;
        CTxdStore::SetCurrentTxd(slot);
        CSprite2d sprite;
        sprite.SetTexture(name);
        if (sprite.m_pTexture) sprite.Draw(CRect(
            CMenuManager::fMapCenterX-CMenuManager::fMapSize+x*tile,
            CMenuManager::fMapCenterY-CMenuManager::fMapSize+y*tile,
            CMenuManager::fMapCenterX-CMenuManager::fMapSize+(x+1)*tile,
            CMenuManager::fMapCenterY-CMenuManager::fMapSize+(y+1)*tile),CRGBA(255,255,255,255));
        CStreaming::SetModelIsDeletable(slot + STREAM_OFFSET_TXD);
    }
    CTxdStore::PopCurrentTxd();
#endif
}
}
