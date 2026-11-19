#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "platform.h"
#include "Enhanced.h"
#include "EnhancedMath.h"
#include "EnhancedRoute.h"
#include "Frontend.h"
#include "frontendoption.h"
#include "Camera.h"
#include "Collision.h"
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
#include "Zones.h"

namespace Enhanced {
int8 QuickSave = 0, AutoSave = 0, RadarZoom = 0;
int8 GPS = 0, MenuMap = 1, VehicleDetail = 0, PedDetail = 0, HealAfterSave = 0;
int8 BreakableWindscreen = 0;
#ifdef RW_D3D9
int8 Borderless = 1;
#else
int8 Borderless = 0;
#endif
int8 VehicleDistance = 0, PedDistance = 0, ScreenFormat = 0;
#ifdef _WIN32
int8 BorderlessCaptureProbe = 1, FullscreenDXGIFlipY = 1;
#else
int8 BorderlessCaptureProbe = 0, FullscreenDXGIFlipY = 1;
#endif
static float radarRange = 120.0f;
static bool homeSaveRequested;
static bool homeSaveActive;
static CVector homeSavePosition;
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

bool FindHomePosition(const CVector &current, CVector &home);
float VehicleRangeMultiplier()
{
    const float values[] = {1.0f,1.5f,2.0f,3.0f};
    return values[Clamp(int(VehicleDistance),0,3)];
}
float PedRangeMultiplier()
{
    const float values[] = {1.0f,1.5f,2.0f,3.0f};
    return values[Clamp(int(PedDistance),0,3)];
}
bool BeginHomeSave()
{
    homeSaveActive=false;
#ifdef MISSION_REPLAY
    if (IsQuickSave) return false;
#endif
    if (!homeSaveRequested || !FindPlayerPed()) return false;
    homeSaveActive=FindHomePosition(FindPlayerCoors(),homeSavePosition);
    return homeSaveActive;
}
void EndHomeSave() { homeSaveActive=false; }
CVector SavePosition(const CVector &original)
{
    return homeSaveActive ? homeSavePosition : original;
}

// Local strings keep the GXT files intact, including all existing translations.
wchar *GetText(const char *key)
{
    struct Entry { const char *key; const char *value; wchar wide[120]; };
    static Entry strings[] = {
        { "ENHANCE", "ENHANCED OPTIONS" },
        { "EN_QSAV", "F5 SAVE MENU" },
        { "EN_AUTO", "SAVE MENU AFTER MISSION" },
        { "EN_RADR", "SMOOTH RADAR ZOOM (T)" },
        { "EN_GPS", "GPS ROUTE" },
        { "EN_MAP", "PAUSE MAP" },
        { "EN_VLOD", "HIGH DETAIL VEHICLES" },
        { "EN_PLOD", "HIGH DETAIL PEDESTRIANS" },
        { "EN_HEAL", "HEAL AFTER MANUAL SAVE" },
        { "EN_GLSS", "BREAKABLE WINDSHIELDS" },
        { "EN_BORD", "BORDERLESS FULLSCREEN" },
        { "EN_BRDL", "BORDERLESS" },
        { "EN_VDST", "VEHICLE DISTANCE" },
        { "EN_PDST", "PEDESTRIAN DISTANCE" },
        { "EN_100", "1X" }, { "EN_150", "1.5X" },
        { "EN_200", "2X" }, { "EN_300", "3X" },
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
    homeSaveRequested = homeSaveActive = false;
    ResetServices();
    radarZoom = 1.0f;
    radarTime = 0;
    routeCount = 0;
    routeLastUpdate = 0;
}

void OnMissionPassed()
{
    if (AutoSave) missionSavePending = true;
}

void ApplyScreenFormatSettings()
{
#ifdef IMPROVED_VIDEOMODE
    ScreenFormat=Clamp(ScreenFormat,int8(0),int8(2));
    Borderless=ScreenFormat==2;
    FrontEndMenuManager.m_nPrefsWindowed=ScreenFormat!=0;
#endif
}

bool TraceVehicleEntry = false;

void WindowModeChanged(int8, int8)
{
#if defined(RW_GL3) && !defined(LIBRW_SDL2)
    ApplyWindowMode();
#elif defined(RW_D3D9)
    _psSelectScreenVM(FrontEndMenuManager.m_nPrefsVideoMode);
#endif
}

float RadarRange(float nativeRange)
{
    const uint32 now = CTimer::GetTimeInMillisecondsPauseMode();
    const float dt = radarTime ? Min(now-radarTime,100u)*0.001f : 0.0f;
    radarTime = now;
    const float target = RadarZoom && CPad::GetPad(0)->GetChar('T') ? 2.25f : 1.0f;
    radarZoom = EnhancedMath::SmoothZoom(radarZoom,target,dt);
    radarRange = nativeRange*radarZoom;
    return radarRange;
}

float BlipScale()
{
#ifdef MENU_MAP
    if (CMenuManager::bMenuMapActive)
        return Clamp(CMenuManager::fMapSize/(SCREEN_HEIGHT*2.0f),0.55f,1.0f);
#endif
    return EnhancedMath::RadarIconScale(radarRange);
}

void AfterSuccessfulSave()
{
    // Only manual saves call this; autosave does not become periodic healing.
    if (HealAfterSave && FindPlayerPed())
        FindPlayerPed()->m_fHealth = Max(FindPlayerPed()->m_fHealth, 100.0f);
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
    struct RoadPoint {
        int from = -1, to = -1;
        CVector position;
        float fraction = 0;
    };
    int first, last, type;
    RoadPoint start, goal;
    int StartNode() const { return ThePaths.m_numPathNodes; }
    int GoalNode() const { return StartNode() + 1; }
    int NodeCount() const { return GoalNode() + 1; }
    float GoalPenalty(int node) const {
        return node == GoalNode() ? 0.0f : std::numeric_limits<float>::infinity();
    }
    template<class Visitor> void VisitRoadEdges(int current, Visitor visit) const {
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

    template<class Visitor> void VisitEdges(int current, Visitor visit) const {
        if (current == GoalNode()) return;
        if (current == StartNode()) {
            const auto enterRoad = [&](int from, int to, float fraction) {
                VisitRoadEdges(from, [&](int next, float weight) {
                    if (next != to) return;
                    if (from == goal.from && to == goal.to && fraction <= goal.fraction)
                        visit(GoalNode(), Max(0.01f, (goal.fraction-fraction)*weight));
                    if (from == goal.to && to == goal.from && fraction <= 1.0f-goal.fraction)
                        visit(GoalNode(), Max(0.01f, (1.0f-goal.fraction-fraction)*weight));
                    visit(to, Max(0.01f, (1.0f-fraction)*weight));
                });
            };
            // At a junction the route can start along any permitted edge.
            if (start.fraction == 0.0f) visit(start.from, 0.01f);
            if (start.fraction == 1.0f) visit(start.to, 0.01f);
            enterRoad(start.from, start.to, start.fraction);
            enterRoad(start.to, start.from, 1.0f-start.fraction);
            return;
        }
        if ((current == goal.from && goal.fraction == 0.0f) ||
            (current == goal.to && goal.fraction == 1.0f))
            visit(GoalNode(), 0.01f);
        VisitRoadEdges(current, [&](int next, float weight) {
            if (current == goal.from && next == goal.to)
                visit(GoalNode(), Max(0.01f, goal.fraction*weight));
            if (current == goal.to && next == goal.from)
                visit(GoalNode(), Max(0.01f, (1.0f-goal.fraction)*weight));
            visit(next, weight);
        });
    }
};

static RouteGraph::RoadPoint ProjectOntoRoad(int from, int to, const CVector &position, bool ignoreHeight)
{
    const CVector &a = ThePaths.m_pathNodes[from].pos;
    const CVector edge = ThePaths.m_pathNodes[to].pos - a;
    const CVector relative = position - a;
    const float length = ignoreHeight ? edge.x*edge.x + edge.y*edge.y : edge.MagnitudeSqr();
    const float dot = relative.x*edge.x + relative.y*edge.y + (ignoreHeight ? 0.0f : relative.z*edge.z);
    const float fraction = length > 0.0f ? Clamp(dot/length, 0.0f, 1.0f) : 0.0f;
    return {from, to, a + edge*fraction, fraction};
}

static bool ClearRouteConnector(const CVector &a, const CVector &b)
{
    // Do not draw a shortcut through a building between the road and marker.
    const eLevelName level = CTheZones::FindZoneForPoint(b);
    return (a-b).MagnitudeSqr() < 40.0f*40.0f &&
        (level == LEVEL_GENERIC || level == CCollision::ms_collisionInMemory) &&
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
    if (!manual && target.z >= -90.0f && (target-start).MagnitudeSqr() < 12.0f*12.0f) {
        routeCount = 0;
        routeLastUpdate = 0;
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
    const bool ignoreHeight = manual || target.z < -90.0f;
    float nearestStart = 120.0f*120.0f;
    RouteGraph::RoadPoint clearStart;
    float nearestClearStart = 40.0f*40.0f;
    float nearestTarget = 120.0f;
    std::vector<RouteGraph::RoadPoint> targetRoads;
    for (int from = graph.first; from < graph.last; ++from) {
        graph.VisitRoadEdges(from, [&](int to, float) {
            const auto entry = ProjectOntoRoad(from, to, start, false);
            const float entryDistance = (entry.position-start).MagnitudeSqr();
            if (entryDistance < nearestStart) {
                nearestStart = entryDistance;
                graph.start = entry;
            }
            if (entryDistance < nearestClearStart && Abs(entry.position.z-start.z) < 4.0f &&
                ClearRouteConnector(start, entry.position)) {
                nearestClearStart = entryDistance;
                clearStart = entry;
            }
            const auto exit = ProjectOntoRoad(from, to, target, ignoreHeight);
            const CVector delta = exit.position-target;
            const float distance = ignoreHeight ? delta.Magnitude2D() : delta.Magnitude();
            if (distance < 120.0f) targetRoads.push_back(exit);
            nearestTarget = Min(nearestTarget, distance);
        });
    }
    if (clearStart.from >= 0) graph.start = clearStart;
    if (graph.start.from < 0 || targetRoads.empty()) return;
    const auto distanceToTarget = [&](const RouteGraph::RoadPoint &road) {
        const CVector delta = road.position-target;
        return ignoreHeight ? delta.Magnitude2D() : delta.Magnitude();
    };
    const bool preferUpperRoad = manual && nearestTarget < 8.0f;
    const float targetRadius = nearestTarget + (preferUpperRoad ? 2.0f : 8.0f);
    targetRoads.erase(std::remove_if(targetRoads.begin(), targetRoads.end(), [&](const auto &road) {
        return distanceToTarget(road) > targetRadius;
    }), targetRoads.end());
    std::stable_sort(targetRoads.begin(), targetRoads.end(), [&](const auto &a, const auto &b) {
        if (preferUpperRoad && a.position.z != b.position.z) return a.position.z > b.position.z;
        return distanceToTarget(a) < distanceToTarget(b);
    });
    std::vector<int> nodes;
    std::vector<RouteGraph::RoadPoint> triedRoads;
    for (const auto &road : targetRoads) {
        if (preferUpperRoad && targetRoads.front().position.z - road.position.z > 2.0f) break;
        if (triedRoads.size() == 6) break;
        if (std::any_of(triedRoads.begin(), triedRoads.end(), [&](const auto &tried) {
            return (road.from == tried.to && road.to == tried.from) ||
                (road.from == tried.from && road.to == tried.to);
        })) continue;
        triedRoads.push_back(road);
        graph.goal = road;
        nodes = EnhancedRoute::Search(graph, graph.StartNode());
        if (!nodes.empty()) break;
    }
    if (nodes.empty()) return;
    CVector destination = target;
    if (ignoreHeight) destination.z = graph.goal.position.z;
    if ((destination-start).MagnitudeSqr() < 12.0f*12.0f &&
        Abs(destination.z-start.z) < 3.0f) {
        if (manual) CRadar::ToggleTargetMarker(target.x, target.y);
        return;
    }
    routeStartsAtPlayer = clearStart.from >= 0;
    if (routeStartsAtPlayer) route.push_back(start);
    route.push_back(graph.start.position);
    for (size_t i = 1; i+1 < nodes.size(); ++i)
        route.push_back(ThePaths.m_pathNodes[nodes[i]].pos);
    route.push_back(graph.goal.position);
    if (ClearRouteConnector(route.back(), destination)) route.push_back(destination);
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
    if (!FrontEndMenuManager.m_bMenuActive && !FrontEndMenuManager.m_bSaveMenuActive)
        homeSaveRequested = false;
    AutoSave = AutoSave != 0;
    RadarZoom = RadarZoom != 0;
    if (!GameReady()) return;
    DiscoverServices(FindPlayerCoors());
    if (QuickSave && CPad::GetPad(0)->GetFJustDown(4)) {
        if (SafeToSave()) { homeSaveRequested=true; FrontEndMenuManager.m_bSaveMenuActive = true; }
        else CHud::SetHelpMessage(GetText("EN_SAFE"), false);
    }
    if (!AutoSave) {
        missionSavePending = false;
    } else {
        if (missionSavePending && SafeToSave()) {
            homeSaveRequested = true;
            FrontEndMenuManager.m_bSaveMenuActive = true;
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
        const CVector aWorld = i == 1 && routeStartsAtPlayer ? FindPlayerCoors() : route[i-1];
        if (i == 1 && routeStartsAtPlayer &&
            ((aWorld-routeStart).MagnitudeSqr() >= 64.0f || Abs(aWorld.z-route[i].z) >= 4.0f ||
             !ClearRouteConnector(aWorld, route[i]))) continue;
        CRadar::TransformRealWorldPointToRadarSpace(a, CVector2D(aWorld));
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
