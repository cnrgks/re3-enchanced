#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "Enhanced.h"
#include "EnhancedMath.h"
#include "EnhancedRoute.h"
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
#include "Pools.h"
#include "Object.h"
#include "Vehicle.h"
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
int8 QuickSave = 0, AutoSave = 0, RadarZoom = 0;
int8 GPS = 0, MenuMap = 1, VehicleDetail = 0, PedDetail = 0, HealAfterSave = 0;
int8 BreakableWindscreen = 0, Borderless = 0;
static float radarZoom = 1.0f;
static uint32 radarTime;
static bool missionSavePending;
static std::vector<CVector> route;
static int routeCount;
static CRGBA routeColour(230,35,35,230);
static bool routeStartsAtPlayer;
static int routeType = -1;
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
        { "EN_AUTO", "AUTOSAVE AFTER MISSION" },
        { "EN_LOAD", "LOAD AUTOSAVE" },
        { "EN_RADR", "SMOOTH RADAR ZOOM (T)" },
        { "EN_GPS", "GPS ROUTE" },
        { "EN_MAP", "PAUSE MAP" },
        { "EN_VLOD", "HIGH DETAIL VEHICLES" },
        { "EN_PLOD", "HIGH DETAIL PEDESTRIANS" },
        { "EN_HEAL", "HEAL AFTER MANUAL SAVE" },
        { "EN_GLSS", "BREAKABLE WINDSHIELDS" },
        { "EN_BORD", "BORDERLESS FULLSCREEN" },
        { "EN_DONE", "Autosave completed." },
        { "EN_FAIL", "Autosave failed. Previous autosave kept." },
        { "EN_SAFE", "Save unavailable: stand still, outside missions, without a wanted level." },
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
    missionSavePending = false;
    radarZoom = 1.0f;
    radarTime = 0;
    routeCount = 0;
    routeLastUpdate = 0;
}

void OnMissionPassed()
{
    if (AutoSave) missionSavePending = true;
}

void WindowModeChanged(int8, int8)
{
#if defined(RW_GL3) && !defined(LIBRW_SDL2)
    ApplyWindowMode();
#endif
}

float RadarRange(float nativeRange)
{
    const uint32 now = CTimer::GetTimeInMillisecondsPauseMode();
    const float dt = radarTime ? Min(now-radarTime,100u)*0.001f : 0.0f;
    radarTime = now;
    const float target = RadarZoom && CPad::GetPad(0)->GetChar('T') ? 3.0f : 1.0f;
    radarZoom = EnhancedMath::SmoothZoom(radarZoom,target,dt);
    return nativeRange*radarZoom;
}

float BlipScale()
{
#ifdef MENU_MAP
    if (CMenuManager::bMenuMapActive)
        return Clamp(CMenuManager::fMapSize/(SCREEN_HEIGHT*2.0f),0.55f,1.0f);
#endif
    return Clamp(1.0f/Sqrt(radarZoom),0.55f,1.0f);
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
    snprintf(temporary, sizeof(temporary), "%s-tmp1.b", prefix);
    if (strlen(temporary) >= sizeof(ValidSaveName)) return false;
    char oldValidName[sizeof(ValidSaveName)];
    strcpy(oldValidName, ValidSaveName);
    // Same native serializer, but no safehouse time skip or stamina mutation.
    // The old autosave is replaced only after writing and closing successfully.
    strncpy(ValidSaveName, temporary, sizeof(ValidSaveName)-1);
    ValidSaveName[sizeof(ValidSaveName)-1] = '\0';
    CFileMgr::SetDir("");
    int file = CFileMgr::OpenFile(temporary, "wb");
    bool ok = false;
    if (file) {
        CTimer::Suspend();
        ok = GenericSave(file);
        if (CFileMgr::GetErrorReadWrite(file)) ok = false;
        if (CFileMgr::CloseFile(file) != 0) ok = false;
        CTimer::Resume();
    }
    strcpy(ValidSaveName, oldValidName);
    if (ok) {
        // The original file layer's error accessor tests EOF, not ferror.
        // Validate the actual bytes on disk before replacing a good autosave.
        char originalPrefix[sizeof(DefaultPCSaveFileName)], validated[300];
        strcpy(originalPrefix, DefaultPCSaveFileName);
        snprintf(DefaultPCSaveFileName, sizeof(DefaultPCSaveFileName), "%s-tmp", prefix);
        ok = CheckDataNotCorrupt(0, validated);
        strcpy(DefaultPCSaveFileName, originalPrefix);
    }
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
    ChangeScreen(MENUPAGE_LOAD_SLOT_CONFIRM, 1);
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

// Lane orientation follows CCarCtrl's native one-way-road checks.
// Nodes disabled by SCM (including closed roads/bridges) are never traversed.
struct RouteGraph {
    struct Goal { int node; float penalty; };
    int first, last, type;
    std::vector<Goal> goals;
    int NodeCount() const { return ThePaths.m_numPathNodes; }
    float GoalPenalty(int node) const {
        for (const Goal &goal : goals) if (goal.node == node) return goal.penalty;
        return std::numeric_limits<float>::infinity();
    }
    template<class Visitor> void VisitEdges(int current, Visitor visit) const {
        const CPathNode &node = ThePaths.m_pathNodes[current];
        if (node.bDisabled) return;
        for (int i = 0; i < node.numLinks; ++i) {
            const int edge = node.firstLink + i;
            if (edge < 0 || edge >= ThePaths.m_numConnections) continue;
            const int next = ThePaths.ConnectedNode(edge);
            if (next < first || next >= last || ThePaths.m_pathNodes[next].bDisabled) continue;
            if (type == PATH_CAR) {
                const int linkId = ThePaths.m_carPathConnections[edge];
                if (linkId < 0 || linkId >= ThePaths.m_numCarPathLinks) continue;
                const CCarPathLink &link = ThePaths.m_carPathLinks[linkId];
                if ((link.pathNodeIndex == current ? link.numRightLanes : link.numLeftLanes) <= 0) continue;
            }
            visit(next, Max(1.0f, float(ThePaths.m_distances[edge])));
        }
    }
};

static bool ClearRouteConnector(const CVector &a, const CVector &b)
{
    // Do not draw a shortcut through a building between the road and marker.
    return (a-b).Magnitude2D() < 40.0f &&
        CWorld::GetIsLineOfSightClear(a+CVector(0,0,1), b+CVector(0,0,1),
            true, false, false, true, true, false);
}

static void UpdateRoute()
{
#ifdef MENU_MAP
    if (!GPS || !FindPlayerPed()) {
        routeCount = 0;
        routeLastUpdate = 0;
        return;
    }
    const CVector start = FindPlayerCoors();
    CVector target;
    bool manual = CRadar::TargetMarkerId != -1;
    bool found = manual;
    routeColour = CRGBA(230,35,35,230);
    if (manual) target = CRadar::TargetMarkerPos;
    else if (CTheScripts::IsPlayerOnAMission()) {
        // SCM has no universal "current GPS objective". Coordinate objectives
        // take priority, then the nearest script-visible entity marker.
        float best = 1.0e20f;
        for (int pass=0; pass<2 && !found; ++pass) {
            for (int i=0; i<NUMRADARBLIPS; ++i) {
                const sRadarTrace &blip = CRadar::ms_RadarTrace[i];
                if (!blip.m_bInUse || blip.m_eRadarSprite != RADAR_SPRITE_NONE ||
                    (blip.m_eBlipDisplay != BLIP_DISPLAY_BOTH &&
                     blip.m_eBlipDisplay != BLIP_DISPLAY_BLIP_ONLY)) continue;
                CVector position;
                if (pass == 0) {
                    if (blip.m_eBlipType != BLIP_COORD) continue;
                    position = blip.m_vecPos;
                } else {
                    CEntity *entity = nil;
                    if (blip.m_eBlipType == BLIP_CAR) entity = CPools::GetVehiclePool()->GetAt(blip.m_nEntityHandle);
                    else if (blip.m_eBlipType == BLIP_CHAR) entity = CPools::GetPedPool()->GetAt(blip.m_nEntityHandle);
                    else if (blip.m_eBlipType == BLIP_OBJECT) entity = CPools::GetObjectPool()->GetAt(blip.m_nEntityHandle);
                    if (!entity || entity == FindPlayerPed() || entity == FindPlayerVehicle()) continue;
                    position = entity->GetPosition();
                }
                float distance = (position-start).MagnitudeSqr();
                if (distance < best) {
                    best=distance; target=position; found=true;
                    // Exactly the native icon colour, including dim variants
                    // and scripts supplying a literal RGBA rather than palette ID.
                    const uint32 colour = CRadar::GetRadarTraceColour(blip.m_nColor, blip.m_bDim);
                    routeColour = CRGBA((colour>>24)&255, (colour>>16)&255,
                        (colour>>8)&255, colour&255);
                }
            }
        }
    }
    if (!found) { routeCount=0; routeLastUpdate=0; return; }
    if ((target-start).Magnitude2D() < 12.0f) {
        if (manual) CRadar::ToggleTargetMarker(target.x, target.y);
        routeCount = 0;
        return;
    }
    const uint32 now = CTimer::GetTimeInMilliseconds();
    const int type = FindPlayerVehicle() ? PATH_CAR : PATH_PED;
    if (type == routeType && routeLastUpdate && now-routeLastUpdate < 750 &&
        (target-routeTarget).MagnitudeSqr() < 1.0f &&
        (start-routeStart).MagnitudeSqr() < 64.0f) return;
    routeLastUpdate = now;
    routeTarget = target;
    routeStart = start;
    routeCount = 0;
    routeType = type;
    route.clear();
    RouteGraph graph;
    graph.type = type;
    graph.first = type == PATH_CAR ? 0 : ThePaths.m_numCarPathNodes;
    graph.last = type == PATH_CAR ? ThePaths.m_numCarPathNodes : ThePaths.m_numPathNodes;
    int startNode = -1;
    float nearestStart = 250.0f*250.0f;
    std::vector<RouteGraph::Goal> candidates;
    for (int node = graph.first; node < graph.last; ++node) {
        const CPathNode &path = ThePaths.m_pathNodes[node];
        if (path.bDisabled || !path.numLinks) continue;
        const float fromStart = (path.pos-start).MagnitudeSqr();
        if (fromStart < nearestStart) { nearestStart=fromStart; startNode=node; }
        const CVector delta = path.pos-target;
        // A map click has no reliable height outside streamed collision.
        const float distance = manual || target.z < -90.0f ?
            delta.Magnitude2D() : delta.Magnitude();
        if (distance < 120.0f) candidates.push_back({node, distance*2.0f});
    }
    std::sort(candidates.begin(), candidates.end(),
        [](const RouteGraph::Goal &a, const RouteGraph::Goal &b) { return a.penalty < b.penalty; });
    if (candidates.size() > 8) candidates.resize(8);
    graph.goals = candidates;
    if (startNode < 0 || graph.goals.empty()) return;
    const std::vector<int> nodes = EnhancedRoute::Search(graph, startNode);
    if (nodes.empty()) return; // Disconnected objectives get no invented straight route.
    routeStartsAtPlayer = ClearRouteConnector(start, ThePaths.m_pathNodes[nodes.front()].pos);
    if (routeStartsAtPlayer) route.push_back(start);
    // Keep the complete path: a fixed-size truncation used to cut across turns.
    for (int node : nodes) route.push_back(ThePaths.m_pathNodes[node].pos);
    if (ClearRouteConnector(route.back(), target)) route.push_back(target);
    routeCount = int(route.size());
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
    AutoSave = AutoSave != 0;
    RadarZoom = RadarZoom != 0;
    if (!GameReady()) return;
    if (QuickSave && CPad::GetPad(0)->GetFJustDown(4)) {
        if (SafeToSave()) FrontEndMenuManager.m_bSaveMenuActive = true;
        else CHud::SetHelpMessage(GetText("EN_SAFE"), false);
    }
    if (!AutoSave) {
        missionSavePending = false;
    } else {
        if (missionSavePending && SafeToSave()) {
            const bool ok = SaveAutosave();
            CHud::SetHelpMessage(GetText(ok ? "EN_DONE" : "EN_FAIL"), false);
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
        CRadar::TransformRealWorldPointToRadarSpace(a, CVector2D(i==1 && routeStartsAtPlayer ? FindPlayerCoors() : route[i-1]));
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
            sb.x-perpendicular.x,sb.y-perpendicular.y, routeColour);
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
        if (!CTxdStore::GetSlot(slot)->texDict) missing = true;
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
