#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"
#include "platform.h"
#include "Enhanced.h"
#include "Darkel.h"
#include "DMAudio.h"
#include "Font.h"
#include "MusicManager.h"
#include "EnhancedMath.h"
#include "EnhancedRoute.h"
#include "Frontend.h"
#include "frontendoption.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "Collision.h"
#include "CutsceneMgr.h"
#include "FileMgr.h"
#include "Game.h"
#include "GenericGameStorage.h"
#include "Hud.h"
#include "IniFile.h"
#include "Pad.h"
#include "PCSave.h"
#include "PathFind.h"
#include "Pools.h"
#include "Population.h"
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
#include <map>

namespace Enhanced {
int8 QuickSave = 0, AutoSave = 0, GPS = 0, HealAfterSave = 0;
#ifdef RW_D3D9
int8 Borderless = 1;
#else
int8 Borderless = 0;
#endif
int8 ScreenFormat = 0, DistantCrowdedWorld = 0;
#ifdef _WIN32
int8 BorderlessCaptureProbe = 1, FullscreenDXGIFlipY = 1;
#else
int8 BorderlessCaptureProbe = 0, FullscreenDXGIFlipY = 1;
#endif
static float radarRange = 120.0f;
static bool homeSaveActive;
static CVector homeSavePosition;
static float radarZoom = 1.0f;
static uint32 radarTime;
static bool missionSavePending;
bool TraceTraffic = false;
TrafficSpawnStats VehicleSpawnStats, PedSpawnStats;
VehicleTrafficDiagnostics VehicleTrafficStats;
static uint32 trafficReportTime;

struct TrafficArea {
    uint32 sampleTime;
    CVector centre;
    std::vector<int32> nearby;
    float networkLength;
    float arrivals;
    int target;
};

static TrafficArea carTraffic, pedTraffic;
static std::vector<CVector> route;
static int routeCount;
static CRGBA routeColour(230,35,35,230);
static bool waypointLayerKnown;
static float waypointLayerZ;
static uint32 routeLastUpdate;
static CVector routeTarget;
static CVector routeStart;

bool FindHomePosition(const CVector &current, CVector &home);
float VehicleRangeMultiplier()
{
    return DistantCrowdedWorld ? 3.0f : 1.0f;
}
float PedRangeMultiplier()
{
    return VehicleRangeMultiplier();
}
bool IsSavingAtHome()
{
    return homeSaveActive;
}

CVector SavePosition(const CVector &original)
{
    return homeSaveActive ? homeSavePosition : original;
}

// Local strings keep the GXT files intact, including all existing translations.
wchar *GetText(const char *key)
{
    struct Entry { const char *key; const char *value; wchar wide[120]; };
    static Entry strings[] = {
        { "EN_GAME", "GAMEPLAY" },
        { "EN_GSET", "GAMEPLAY SETUP" },
        { "EN_DEFT", "RESTORE DEFAULTS" },
        { "EN_DEFP", "Are you sure you want to restore the default settings?" },
        { "EN_NO", "NO" },
        { "EN_YES", "YES" },
        { "EN_QSAV", "F5 QUICKSAVE" },
        { "EN_AUTO", "MISSION AUTOSAVE" },
        { "EN_ASOK", "Autosaved." },
        { "EN_ASER", "Autosave failed." },
        { "EN_GPS", "MISSION GPS" },
        { "EN_HEAL", "HEAL AFTER SAVING" },
        { "EN_INT", "SKIP INTROS" },
        { "EN_BORD", "BORDERLESS FULLSCREEN" },
        { "EN_BRDL", "BORDERLESS" },
        { "EN_DCW", "DISTANT AND CROWDED WORLD" },
        { "EN_SAFE", "To save, stand still on foot with no active mission and no wanted level." },
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


enum class SelectionWheel { Closed, Radio, Weapon };

static SelectionWheel selectionWheel;
static uint8 wheelSelection;
static bool wheelRadioIconsLoaded;
static CVector2D wheelPointer;
static CSprite2d wheelRadioIcons[USERTRACK + 1];

static const char *const radioTextureNames[] = {
    "fe_radio1", "fe_radio2", "fe_radio5", "fe_radio7", "fe_radio8",
    "fe_radio3", "fe_radio4", "fe_radio6", "fe_radio9",
};
static const char *const weaponLabels[] = {
    "Unarmed", "Baseball Bat", "Pistol", "Uzi", "Shotgun", "AK-47",
    "M16", "Sniper Rifle", "Rocket Launcher", "Flamethrower", "Molotov",
    "Grenade", "Detonator",
};
static_assert(ARRAY_SIZE(radioTextureNames) == USERTRACK, "Radio icon order");
static_assert(ARRAY_SIZE(weaponLabels) == WEAPONTYPE_TOTAL_INVENTORY_WEAPONS, "Weapon icon order");

static bool CanSelectRadio()
{
    CVehicle *vehicle = FindPlayerVehicle();
    return vehicle && MusicManager.PlayerInCar() && !MusicManager.UsesPoliceRadio(vehicle) &&
        !MusicManager.m_bRadioSetByScript && !MusicManager.m_bAnnouncementInProgress;
}

static bool CanSelectWeapon(CPlayerPed *player)
{
    return !player->bInVehicle && !CWorld::Players[CWorld::PlayerInFocus].m_bInRemoteMode &&
        player->IsPedInControl() && !CDarkel::FrenzyOnGoing() &&
        player->m_nPedState != PED_ATTACK && player->m_nPedState != PED_FIGHT &&
        player->m_nPedState != PED_AIM_GUN;
}

static void CloseSelectionWheel()
{
    selectionWheel = SelectionWheel::Closed;
    CTimer::SetWeaponWheelSlowMotion(false);
}

bool IsSelectionWheelOpen()
{
    return selectionWheel != SelectionWheel::Closed;
}

bool IsWeaponWheelOpen()
{
    return selectionWheel == SelectionWheel::Weapon;
}

void CancelSelectionWheelIfUnavailable()
{
    if (selectionWheel != SelectionWheel::Closed && !GameReady())
        CloseSelectionWheel();
}

static int CollectWheelEntries(uint8 *entries)
{
    int count = 0;
    if (selectionWheel == SelectionWheel::Radio) {
        for (int i = HEAD_RADIO; i <= CHATTERBOX; ++i)
            entries[count++] = i;
        if (DMAudio.IsMP3RadioChannelAvailable())
            entries[count++] = USERTRACK;
        entries[count++] = RADIO_OFF;
    } else {
        CPlayerPed *player = FindPlayerPed();
        for (int i = 0; i < WEAPONTYPE_TOTAL_INVENTORY_WEAPONS; ++i) {
            if (player->HasWeapon(i) &&
                (i <= WEAPONTYPE_BASEBALLBAT || i == WEAPONTYPE_DETONATOR ||
                    player->GetWeapon(i).m_nAmmoTotal > 0))
                entries[count++] = i;
        }
    }
    return count;
}

void ProcessSelectionWheel()
{
    if (!GameReady() || CPad::GetPad(0)->ArePlayerControlsDisabled()) {
        CloseSelectionWheel();
        return;
    }

    CPad *pad = CPad::GetPad(0);
    CPlayerPed *player = FindPlayerPed();
    // Function-key indices are zero-based.
    constexpr int radioKey = 5;
    constexpr int weaponKey = 6;
    if (selectionWheel == SelectionWheel::Closed) {
        wheelPointer = CVector2D(0.0f, 0.0f);
        if (pad->GetFJustDown(radioKey) && CanSelectRadio()) {
            selectionWheel = SelectionWheel::Radio;
            wheelSelection = MusicManager.GetRadioInCar();
        } else if (pad->GetFJustDown(weaponKey) && CanSelectWeapon(player)) {
            selectionWheel = SelectionWheel::Weapon;
            wheelSelection = player->GetWeapon()->m_eWeaponType;
            CTimer::SetWeaponWheelSlowMotion(true);
        } else {
            return;
        }
    }

    const bool radio = selectionWheel == SelectionWheel::Radio;
    if (pad->GetBackspaceJustDown() ||
        (radio ? !CanSelectRadio() : !CanSelectWeapon(player))) {
        CloseSelectionWheel();
        return;
    }

    uint8 entries[WEAPONTYPE_TOTAL_INVENTORY_WEAPONS];
    const int count = CollectWheelEntries(entries);
    if (count == 0) {
        CloseSelectionWheel();
        return;
    }
    int selected = 0;
    while (selected + 1 < count && entries[selected] != wheelSelection)
        ++selected;
    if (entries[selected] != wheelSelection)
        selected = 0;
    const int direction = int(pad->GetRightJustDown()) - int(pad->GetLeftJustDown());
    if (direction) {
        wheelPointer = CVector2D(0.0f, 0.0f);
        selected = (selected + count + direction) % count;
    } else if (pad->GetMouseX() != 0.0f || pad->GetMouseY() != 0.0f) {
        const float dx = MousePointerStateHelper.bInvertHorizontally ? -pad->GetMouseX() : pad->GetMouseX();
        const float dy = MousePointerStateHelper.bInvertVertically ? -pad->GetMouseY() : pad->GetMouseY();
        wheelPointer.x += dx;
        wheelPointer.y += dy;
        const float distance = Sqrt(SQR(wheelPointer.x) + SQR(wheelPointer.y));
        if (distance >= 12.0f) {
            const float angle = Atan2(wheelPointer.y, wheelPointer.x) + HALFPI + TWOPI;
            selected = int(angle / (TWOPI / count) + 0.5f) % count;
            if (distance > 48.0f) {
                wheelPointer.x *= 48.0f / distance;
                wheelPointer.y *= 48.0f / distance;
            }
        }
    }
    wheelSelection = entries[selected];

    if (!pad->GetF(radio ? radioKey : weaponKey)) {
        if (radio) {
            MusicManager.SelectCarStation(wheelSelection);
        } else if (player->GetWeapon()->m_eWeaponType != wheelSelection) {
            player->m_nSelectedWepSlot = wheelSelection;
            player->MakeChangesForNewWeapon(wheelSelection);
        }
        CloseSelectionWheel();
    }
}

static bool LoadWheelTxd(int slot, const char *filename)
{
    RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
    if (!stream)
        return false;
    const bool loaded = CTxdStore::LoadTxd(slot, stream);
    RwStreamClose(stream, nil);
    return loaded;
}

static void LoadSelectionWheelTextures()
{
    if (wheelRadioIconsLoaded)
        return;

    wheelRadioIconsLoaded = true;
    const int radioSlot = CTxdStore::AddTxdSlot("wheel_radio");
    CTxdStore::PushCurrentTxd();
    if (LoadWheelTxd(radioSlot, "MODELS/FRONTEND.TXD")) {
        CTxdStore::SetCurrentTxd(radioSlot);
        for (int i = 0; i < USERTRACK; ++i)
            wheelRadioIcons[i].SetTexture(radioTextureNames[i], "");
    }

    const int menuSlot = CTxdStore::AddTxdSlot("wheel_menu");
    if (LoadWheelTxd(menuSlot, "MODELS/MENU.TXD")) {
        CTxdStore::SetCurrentTxd(menuSlot);
        wheelRadioIcons[USERTRACK].SetTexture("mp3logo", "mp3logoA");
    }
    CTxdStore::PopCurrentTxd();
    CTxdStore::RemoveTxdSlot(menuSlot);
    CTxdStore::RemoveTxdSlot(radioSlot);
}

void FreeSelectionWheelTextures()
{
    CloseSelectionWheel();
    for (auto &icon : wheelRadioIcons)
        icon.Delete();
    wheelRadioIconsLoaded = false;
}

static void PrintWheelText(float x, float y, const char *label, float size)
{
    wchar wide[72];
    AsciiToUnicode(label, wide);
    CFont::SetScale(SCREEN_SCALE_X(size), SCREEN_SCALE_Y(size * 1.4f));
    CFont::PrintString(x, y, wide);
}

static const char *RadioLabel(uint8 station)
{
    static const char *const labels[] = {
        "Head Radio", "Double Clef FM", "K-JAH", "Rise FM", "Lips 106",
        "Game FM", "MSX FM", "Flashback 95.6", "Chatterbox", "MP3 Player",
    };
    return station == RADIO_OFF ? "Radio Off" : labels[station];
}

void DrawSelectionWheel()
{
    if (selectionWheel == SelectionWheel::Closed || !GameReady())
        return;

    const bool radio = selectionWheel == SelectionWheel::Radio;
    if (radio)
        LoadSelectionWheelTextures();

    uint8 entries[WEAPONTYPE_TOTAL_INVENTORY_WEAPONS];
    const int count = CollectWheelEntries(entries);
    if (!count)
        return;

    const float x = SCREEN_WIDTH * 0.5f;
    const float y = SCREEN_HEIGHT * 0.46f;
    const float scale = Min(SCREEN_WIDTH / 640.0f, SCREEN_HEIGHT / 448.0f);
    const float radius = 126.0f * scale;
    const float iconRadius = 97.0f * scale;
    const float sectorAngle = TWOPI / count;
    const float halfSector = TWOPI / Max(count, 5) * 0.44f;
    const float half = 17.0f * scale;
    CSprite2d segment;
    CSprite2d::DrawRect(CRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(5, 10, 16, 96));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

    CFont::SetBackgroundOff();
    CFont::SetPropOn();
    CFont::SetFontStyle(FONT_HEADING);
    CFont::SetJustifyOff();
    CFont::SetRightJustifyOff();
    CFont::SetCentreOn();
    CFont::SetCentreSize(SCREEN_WIDTH);
    CFont::SetDropShadowPosition(0);
    CFont::SetColor(CRGBA(245, 239, 212, 255));
    for (int i = 0; i < count; ++i) {
        const uint8 entry = entries[i];
        const float angle = -HALFPI + i * sectorAngle;
        const float start = angle - halfSector;
        const float end = angle + halfSector;
        const float inner = 71.0f * scale;
        const CRGBA colour = entry == wheelSelection
            ? CRGBA(56, 112, 126, 245) : CRGBA(13, 24, 32, 230);
        constexpr int steps = 6;
        for (int step = 0; step < steps; ++step) {
            const float a = start + (end - start) * step / steps;
            const float b = start + (end - start) * (step + 1) / steps;
            segment.Draw(x + Cos(a) * inner, y + Sin(a) * inner,
                         x + Cos(b) * inner, y + Sin(b) * inner,
                         x + Cos(a) * radius, y + Sin(a) * radius,
                         x + Cos(b) * radius, y + Sin(b) * radius, colour);
        }

        const float iconX = x + Cos(angle) * iconRadius;
        const float iconY = y + Sin(angle) * iconRadius;
        CSprite2d *icon = radio
            ? (entry == RADIO_OFF ? nil : &wheelRadioIcons[entry])
            : &CHud::Sprites[entry];
        if (icon && icon->m_pTexture)
            icon->Draw(CRect(iconX - half, iconY - half, iconX + half, iconY + half),
                       CRGBA(255, 255, 255, 255));
        else
            PrintWheelText(iconX, iconY - 5.0f * scale,
                           radio ? RadioLabel(entry) : weaponLabels[entry], 0.38f);
    }

    PrintWheelText(x, y - 12.0f * scale,
                   radio ? RadioLabel(wheelSelection) : weaponLabels[wheelSelection], 0.75f);
    if (!radio && wheelSelection > WEAPONTYPE_BASEBALLBAT && wheelSelection != WEAPONTYPE_DETONATOR) {
        const CWeapon &weapon = FindPlayerPed()->GetWeapon(wheelSelection);
        char ammo[32];
        snprintf(ammo, sizeof(ammo), "%u / %u", unsigned(weapon.m_nAmmoInClip),
                 unsigned(weapon.m_nAmmoTotal));
        PrintWheelText(x, y + 10.0f * scale, ammo, 0.52f);
    }
    PrintWheelText(x, y + radius + 10.0f * scale,
                   radio ? "Mouse / Arrows: Select   Release F6: Tune   Backspace: Cancel"
                         : "Mouse / Arrows: Select   Release F7: Equip   Backspace: Cancel", 0.48f);
}

static bool CanSaveAfterMission()
{
    if (CTheScripts::IsPlayerOnAMission() || CTheScripts::bAlreadyRunningAMissionScript) return false;
    CPlayerInfo *info = FindPlayerPed()->GetPlayerInfoForThisPlayerPed();
    return info && info->m_WBState == WBSTATE_PLAYING && !info->m_bInRemoteMode &&
        TheCamera.GetScreenFadeStatus() == FADE_0;
}

static bool CanQuickSave()
{
    CPlayerPed *p = FindPlayerPed();
    return CanSaveAfterMission() && !p->InVehicle() && p->IsPedInControl() && p->bIsStanding &&
        p->GetPedState() == PED_IDLE && p->m_vecMoveSpeed.MagnitudeSqr() < 0.0001f &&
        (!p->m_pWanted || p->m_pWanted->GetWantedLevel() == 0);
}

void Reset()
{
    CloseSelectionWheel();
    missionSavePending = false;
    homeSaveActive = false;
    VehicleSpawnStats = {};
    PedSpawnStats = {};
    VehicleTrafficStats = {};
    carTraffic = {};
    pedTraffic = {};
    trafficReportTime = CTimer::GetTimeInMilliseconds();
    ResetServices();
    radarZoom = 1.0f;
    radarTime = 0;
    routeCount = 0;
    routeLastUpdate = 0;
    waypointLayerKnown = false;
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
    const float target = CPad::GetPad(0)->GetChar('T') ? 2.25f : 1.0f;
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

void OpenMap(int8 action)
{
#ifdef MENU_MAP
    if (action == FEOPTION_ACTION_SELECT)
        FrontEndMenuManager.OpenMap(false);
#endif
}

// Lane orientation follows CCarCtrl's native one-way-road checks.
// Nodes disabled by SCM (including closed roads/bridges) are never traversed.
struct RouteGraph {
    struct RoadPoint {
        int from = -1, to = -1;
        CVector position;
        float fraction = 0.0f;
        float cost = 0.0f;
        bool connectorClear = false;
    };
    std::vector<RoadPoint> starts, goals;
    std::multimap<int, int> goalsByEndpoint;

    int StartNode() const { return ThePaths.m_numCarPathNodes; }
    int FirstEntry() const { return StartNode() + 1; }
    int FirstGoal() const { return FirstEntry() + int(starts.size()); }
    int NodeCount() const { return FirstGoal() + int(goals.size()); }
    float GoalPenalty(int node) const {
        return node >= FirstGoal() ? goals[node-FirstGoal()].cost :
            std::numeric_limits<float>::infinity();
    }
    void AddGoal(const RoadPoint &road)
    {
        const int node = FirstGoal() + int(goals.size());
        goalsByEndpoint.emplace(road.from, node);
        goalsByEndpoint.emplace(road.to, node);
        goals.push_back(road);
    }

    template<class Visitor> void VisitRoadEdges(int current, Visitor visit, bool traversable = true) const {
        const CPathNode &node = ThePaths.m_pathNodes[current];
        if (traversable && node.bDisabled) return;
        for (int i = 0; i < node.numLinks; ++i) {
            const int edge = node.firstLink + i;
            if (edge < 0 || edge >= ThePaths.m_numConnections) continue;
            const int next = ThePaths.ConnectedNode(edge);
            if (next < 0 || next >= StartNode() || next == current) continue;
            if (traversable && ThePaths.m_pathNodes[next].bDisabled) continue;
            if (traversable) {
                const int linkId = ThePaths.m_carPathConnections[edge];
                if (linkId < 0 || linkId >= ThePaths.m_numCarPathLinks) continue;
                const CCarPathLink &link = ThePaths.m_carPathLinks[linkId];
                if ((link.pathNodeIndex == current ? link.numRightLanes : link.numLeftLanes) <= 0) continue;
            }
            visit(next, Max(1.0f, float(ThePaths.m_distances[edge])));
        }
    }

    template<class Visitor> void VisitGoals(int from, int to, float fraction, float weight, Visitor visit) const {
        const auto range = goalsByEndpoint.equal_range(from);
        for (auto it = range.first; it != range.second; ++it) {
            const RoadPoint &goal = goals[it->second-FirstGoal()];
            const bool forward = from == goal.from;
            const float end = forward ? goal.fraction : 1.0f-goal.fraction;
            if (end >= fraction && (forward ? goal.to : goal.from) == to)
                visit(it->second, Max(0.01f, (end-fraction)*weight));
        }
    }

    template<class Visitor> void VisitEdges(int current, Visitor visit) const {
        if (current >= FirstGoal()) return;
        if (current == StartNode()) {
            for (int i = 0; i < int(starts.size()); ++i)
                visit(FirstEntry()+i, starts[i].cost);
            return;
        }
        if (current >= FirstEntry()) {
            const RoadPoint &start = starts[current-FirstEntry()];
            const auto enterRoad = [&](int from, int to, float fraction) {
                VisitRoadEdges(from, [&](int next, float weight) {
                    if (next != to) return;
                    VisitGoals(from, to, fraction, weight, visit);
                    visit(to, Max(0.01f, (1.0f-fraction)*weight));
                });
            };
            if (start.fraction == 0.0f) visit(start.from, 0.01f);
            if (start.fraction == 1.0f) visit(start.to, 0.01f);
            enterRoad(start.from, start.to, start.fraction);
            enterRoad(start.to, start.from, 1.0f-start.fraction);
            return;
        }
        const auto range = goalsByEndpoint.equal_range(current);
        for (auto it = range.first; it != range.second; ++it) {
            const RoadPoint &goal = goals[it->second-FirstGoal()];
            if ((current == goal.from && goal.fraction == 0.0f) ||
                (current == goal.to && goal.fraction == 1.0f))
                visit(it->second, 0.01f);
        }
        VisitRoadEdges(current, [&](int next, float weight) {
            // Most road nodes are outside the destination area.
            if (range.first != range.second) VisitGoals(current, next, 0.0f, weight, visit);
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
    const bool manual = CRadar::TargetMarkerId != -1;
    if (!manual) waypointLayerKnown = false;
    const CVehicle *vehicle = FindPlayerVehicle();
    if ((!manual && !GPS) || !vehicle) {
        routeCount = 0;
        routeLastUpdate = 0;
        return;
    }
    const CVector start = FindPlayerCoors();
    CVector target;
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
                    if (!entity || entity == FindPlayerPed() || entity == vehicle) continue;
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
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (routeLastUpdate && now-routeLastUpdate < 750 &&
        (target-routeTarget).MagnitudeSqr() < 1.0f &&
        (start-routeStart).MagnitudeSqr() < 64.0f) return;
    routeLastUpdate = now;
    routeTarget = target;
    routeStart = start;
    routeCount = 0;
    route.clear();
    RouteGraph graph;

    constexpr float layerTolerance = 3.0f;
    constexpr float startRadius = 40.0f;
    constexpr float targetRadius = 120.0f;
    constexpr float arrivalDistance = 12.0f;
    const bool ignoreHeight = manual || target.z < -90.0f;
    const auto targetDistance = [&](const RouteGraph::RoadPoint &road) {
        const CVector delta = road.position-target;
        return ignoreHeight ? delta.Magnitude2D() : delta.Magnitude();
    };

    std::vector<RouteGraph::RoadPoint> entries, destinations;
    float nearestStart = startRadius, nearestTarget = targetRadius;
    float startLayerZ = start.z, targetLayerZ = target.z;
    for (int from = 0; from < graph.StartNode(); ++from) {
        // Physical segments determine the clicked layer even while SCM closes it.
        graph.VisitRoadEdges(from, [&](int to, float) {
            if (from > to) return;
            if (!ThePaths.m_pathNodes[from].bDisabled && !ThePaths.m_pathNodes[to].bDisabled) {
                auto entry = ProjectOntoRoad(from, to, start, false);
                const float distance = (entry.position-start).Magnitude();
                if (distance < startRadius && Abs(entry.position.z-start.z) < 4.0f &&
                    ClearRouteConnector(start, entry.position)) {
                    entry.cost = distance;
                    entries.push_back(entry);
                    if (distance < nearestStart) {
                        nearestStart = distance;
                        startLayerZ = entry.position.z;
                    }
                }
            }

            const auto destination = ProjectOntoRoad(from, to, target, ignoreHeight);
            if (!ignoreHeight && Abs(destination.position.z-target.z) > 6.0f) return;
            if (manual && waypointLayerKnown &&
                Abs(destination.position.z-waypointLayerZ) > layerTolerance) return;
            const float distance = targetDistance(destination);
            if (distance >= targetRadius) return;
            destinations.push_back(destination);
            if (distance < nearestTarget) {
                nearestTarget = distance;
                targetLayerZ = destination.position.z;
            }
        }, false);
    }
    if (destinations.empty()) return;

    if (manual && !waypointLayerKnown) {
        if (nearestTarget < 8.0f) {
            for (const auto &road : destinations)
                if (targetDistance(road) <= nearestTarget+2.0f)
                    targetLayerZ = Max(targetLayerZ, road.position.z);
        }
        waypointLayerZ = targetLayerZ;
        waypointLayerKnown = true;
    }
    if (manual) targetLayerZ = waypointLayerZ;
    if (entries.empty()) return;

    for (auto entry : entries) {
        if (entry.cost > nearestStart+6.0f || Abs(entry.position.z-startLayerZ) > layerTolerance) continue;
        entry.cost = Max(0.01f, 4.0f*entry.cost);
        graph.starts.push_back(entry);
    }

    for (auto road : destinations) {
        const float distance = targetDistance(road);
        if (distance > nearestTarget+18.0f || Abs(road.position.z-targetLayerZ) > layerTolerance ||
            ThePaths.m_pathNodes[road.from].bDisabled || ThePaths.m_pathNodes[road.to].bDisabled) continue;
        CVector marker = target;
        if (ignoreHeight) marker.z = road.position.z;
        road.connectorClear = Abs(road.position.z-marker.z) < layerTolerance &&
            ClearRouteConnector(road.position, marker);
        road.cost = 4.0f*distance;
        graph.AddGoal(road);
    }
    if (graph.goals.empty()) return;
    const bool hasClearGoal = std::any_of(graph.goals.begin(), graph.goals.end(),
        [](const auto &goal) { return goal.connectorClear; });
    const auto nodes = EnhancedRoute::Search(graph, graph.StartNode(), [&](int node) {
        return !hasClearGoal || graph.goals[node-graph.FirstGoal()].connectorClear;
    });
    if (nodes.empty()) return;

    const auto &entry = graph.starts[nodes[1]-graph.FirstEntry()];
    const auto &goal = graph.goals[nodes.back()-graph.FirstGoal()];
    CVector destination = target;
    if (ignoreHeight) destination.z = goal.position.z;

    route.push_back(start);
    route.push_back(entry.position);
    for (size_t i = 2; i+1 < nodes.size(); ++i)
        route.push_back(ThePaths.m_pathNodes[nodes[i]].pos);
    route.push_back(goal.position);
    if (goal.connectorClear) route.push_back(destination);
    if (goal.connectorClear && Abs(destination.z-start.z) < layerTolerance &&
        (destination-start).MagnitudeSqr() < arrivalDistance*arrivalDistance) {
        float remaining = 0.0f;
        for (size_t i = 1; i < route.size(); ++i)
            remaining += (route[i]-route[i-1]).Magnitude();
        if (remaining < arrivalDistance) {
            if (manual) {
                CRadar::ToggleTargetMarker(target.x, target.y);
                waypointLayerKnown = false;
            }
            return;
        }
    }
    routeCount = int(route.size());
#endif
}

void WaypointChanged()
{
    waypointLayerKnown = false;
    routeLastUpdate = 0;
    UpdateRoute();
}

static float MeasureTrafficNetwork(bool vehicles, const CVector &centre)
{
    constexpr float scanRadius = 180.0f;
    const int first = vehicles ? 0 : ThePaths.m_numCarPathNodes;
    const int last = vehicles ? ThePaths.m_numCarPathNodes : ThePaths.m_numPathNodes;
    int nearest = -1;
    float nearestDistance = sq(scanRadius);
    for (int i = first; i < last; ++i) {
        const CPathNode &node = ThePaths.m_pathNodes[i];
        if (node.bDisabled || !node.numLinks) continue;
        const float distance = (node.pos-centre).MagnitudeSqr();
        if (distance < nearestDistance) {
            nearest = i;
            nearestDistance = distance;
        }
    }
    if (nearest < 0) return 0.0f;

    const int group = ThePaths.m_pathNodes[nearest].group;
    float length = 0.0f;
    for (int i = first; i < last; ++i) {
        const CPathNode &node = ThePaths.m_pathNodes[i];
        if (node.bDisabled || node.group != group || Abs(node.pos.z-centre.z) > 18.0f) continue;
        for (int j = 0; j < node.numLinks; ++j) {
            const int edge = node.firstLink + j;
            if (edge < 0 || edge >= ThePaths.m_numConnections) continue;
            const int next = ThePaths.ConnectedNode(edge);
            if (next <= i || next >= last) continue;
            const CPathNode &other = ThePaths.m_pathNodes[next];
            if (other.bDisabled || Abs(other.pos.z-centre.z) > 18.0f) continue;
            if (((node.pos+other.pos)*0.5f-centre).MagnitudeSqr2D() > sq(scanRadius)) continue;
            int lanes = 1;
            if (vehicles) {
                const int link = ThePaths.m_carPathConnections[edge];
                if (link < 0 || link >= ThePaths.m_numCarPathLinks) continue;
                lanes = ThePaths.m_carPathLinks[link].numLeftLanes + ThePaths.m_carPathLinks[link].numRightLanes;
            } else if (ThePaths.ConnectionCrossesRoad(edge)) {
                continue;
            }
            length += (other.pos-node.pos).Magnitude2D() * Max(0, lanes);
        }
    }
    return length;
}

int TrafficAttempts(bool vehicles, const CVector &centre, float nearbyRadius, bool warmingUp)
{
    TrafficArea &area = vehicles ? carTraffic : pedTraffic;
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (!area.sampleTime || now < area.sampleTime || now-area.sampleTime >= 750) {
        const bool continuous = area.sampleTime && now >= area.sampleTime &&
            now-area.sampleTime < 2000 && (centre-area.centre).MagnitudeSqr2D() < sq(nearbyRadius);
        std::vector<int32> nearby;
        const auto collect = [&](auto *pool, const auto &isAmbient) {
            for (int i = 0; i < pool->GetSize(); ++i) {
                auto *entity = pool->GetSlot(i);
                if (entity && isAmbient(entity) && (entity->GetPosition()-centre).MagnitudeSqr2D() <= sq(nearbyRadius))
                    nearby.push_back(pool->GetIndex(entity));
            }
        };
        if (vehicles)
            collect(CPools::GetVehiclePool(), [](CVehicle *car) { return car->VehicleCreatedBy == RANDOM_VEHICLE; });
        else
            collect(CPools::GetPedPool(), [](CPed *ped) {
                return !ped->IsPlayer() && ped->CharCreatedBy == RANDOM_CHAR && !ped->bInVehicle && ped->m_fHealth > 0.0f;
            });

        int arrivals = 0;
        if (continuous) {
            for (int32 id : nearby)
                if (std::find(area.nearby.begin(), area.nearby.end(), id) == area.nearby.end()) ++arrivals;
        }
        area.arrivals = continuous ? 0.75f*area.arrivals + 0.25f*arrivals : 0.0f;
        area.nearby.swap(nearby);
        area.networkLength = MeasureTrafficNetwork(vehicles, centre);
        area.centre = centre;
        area.sampleTime = now;
    }

    if (warmingUp) return 4;
    const float range = vehicles ? VehicleRangeMultiplier() : PedRangeMultiplier();
    const float localTarget = area.target / range;
    const bool thin = area.nearby.size() < localTarget * 0.75f && area.arrivals < localTarget * 0.15f;
    const int current = vehicles ? CCarCtrl::NumRandomCars : CPopulation::PedCountForSpawnBudget();
    const int slots = vehicles
        ? Min(CCarCtrl::AmbientVehicleSlotsAvailable(), CPopulation::AmbientVehicleOccupantSlotsAvailable())
        : CPopulation::AmbientPedSlotsAvailable();
    const int deficit = Min(Max(0, area.target - current), slots);
    // Keep one budget check so menu changes and newly freed slots are picked up.
    if (deficit <= 0) return 1;

    const float remaining = float(deficit) / Max(1, area.target);
    int attempts = 1;
    if (thin && (CTimer::GetFrameCounter() & 1) == 0)
        attempts += int(ceilf(3.0f * remaining));
    return Min(attempts, deficit);
}

int TrafficTarget(bool vehicles, int limit, bool warmingUp)
{
    TrafficArea &area = vehicles ? carTraffic : pedTraffic;
    // Reference lengths within the sampling radius are tunable from the traffic log.
    const float referenceLength = vehicles ? 1400.0f : 1100.0f;
    const float capacity = Clamp(area.networkLength / referenceLength, 0.25f, 1.0f);
    area.target = int(ceilf(limit * capacity));
    const float range = vehicles ? VehicleRangeMultiplier() : PedRangeMultiplier();
    return warmingUp ? int(ceilf(area.target / range)) : area.target;
}

bool IsTrafficSpawnOccluded(const CVector &position)
{
    const CVector camera = TheCamera.GetPosition();
    return !CWorld::GetIsLineOfSightClear(camera, position+CVector(0.0f, 0.0f, 0.8f),
        true, false, false, true, true, false) &&
        !CWorld::GetIsLineOfSightClear(camera, position+CVector(0.0f, 0.0f, 1.8f),
        true, false, false, true, true, false);
}

static void ReportTraffic()
{
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (now < trafficReportTime) trafficReportTime = now;
    const uint32 elapsed = now - trafficReportTime;
    if (elapsed < 2000) return;

    if (FILE *log = fopen("re3-traffic.log", "a")) {
        const CVector centre = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
        int32 totalPeds;
        const int32 nearbyPeds = CPopulation::CountNearbyTraffic(centre, &totalPeds);
        fprintf(log, "time=%u interval_ms=%u x=%.1f y=%.1f "
            "cars distance=%.0f density=%.2f near=%d total=%d "
            "near_attempts=%u near_spawned=%u far_attempts=%u far_spawned=%u "
            "peds distance=%.0f density=%.2f near=%d total=%d "
            "near_attempts=%u near_spawned=%u far_attempts=%u far_spawned=%u "
            "car_target=%d car_lane_m=%.0f car_arrivals=%.2f car_no_candidate=%u car_exposed=%u "
            "ped_target=%d ped_walk_m=%.0f ped_arrivals=%.2f ped_no_candidate=%u ped_exposed=%u\n",
            now, elapsed, centre.x, centre.y,
            VehicleRangeMultiplier(), CIniFile::CarNumberMultiplier,
            CCarCtrl::CountNearbyTraffic(centre), CCarCtrl::NumRandomCars,
            VehicleSpawnStats.attempts[0], VehicleSpawnStats.spawned[0],
            VehicleSpawnStats.attempts[1], VehicleSpawnStats.spawned[1],
            PedRangeMultiplier(), CIniFile::PedNumberMultiplier,
            nearbyPeds, totalPeds,
            PedSpawnStats.attempts[0], PedSpawnStats.spawned[0],
            PedSpawnStats.attempts[1], PedSpawnStats.spawned[1],
            carTraffic.target, carTraffic.networkLength, carTraffic.arrivals, VehicleSpawnStats.noCandidate, VehicleSpawnStats.exposed,
            pedTraffic.target, pedTraffic.networkLength, pedTraffic.arrivals, PedSpawnStats.noCandidate, PedSpawnStats.exposed);
        const auto &car = VehicleTrafficStats;
        fprintf(log, "time=%u car_rejections target=%u car_budget=%u ped_pool=%u occupant_budget=%u no_models_ticks=%u "
            "route=%u lane=%u ground=%u visibility=%u collision=%u direction=%u "
            "range_removed=%u offscreen_removed=%u moving_offscreen_kept=%u "
            "vehicle_pool_used=%d vehicle_pool_size=%d ambient_ped_slots=%d occupant_slots=%d traffic_occupants=%d occupant_target=%d distant_crowded_world=%d\n",
            now, car.targetFull, car.carBudgetFull, car.pedPoolLow, car.occupantBudgetFull, car.noModels,
            car.routeRejected, car.laneRejected, car.groundRejected, car.visibilityRejected,
            car.collisionRejected, car.directionRejected, car.rangeRemoved, car.offscreenRemoved,
            car.movingOffscreenKept, CPools::GetVehiclePool()->GetNoOfUsedSpaces(),
            CPools::GetVehiclePool()->GetSize(), CPopulation::AmbientPedSlotsAvailable(),
            CPopulation::AmbientVehicleOccupantSlotsAvailable(), CPopulation::CountAmbientVehicleOccupants(),
            CPopulation::AmbientVehicleOccupantTarget(), int(DistantCrowdedWorld));
        fclose(log);
    }
    trafficReportTime = now;
    VehicleSpawnStats = {};
    PedSpawnStats = {};
    VehicleTrafficStats = {};
}

void Process()
{
    // Also clamp settings loaded from edited ini files before using as indexes.
    AutoSave = AutoSave != 0;
    if (!GameReady()) return;
    if (TraceTraffic) ReportTraffic();
    if (!AutoSave) missionSavePending = false;
    if (QuickSave && CPad::GetPad(0)->GetFJustDown(4)) {
        if (CanQuickSave()) {
            FrontEndMenuManager.m_bSaveMenuActive = true;
            return;
        }
        CHud::SetHelpMessage(GetText("EN_SAFE"), false);
    }
    if (missionSavePending && CanSaveAfterMission() &&
        FindHomePosition(FindPlayerCoors(), homeSavePosition)) {
        missionSavePending = false;
        homeSaveActive = true;
        const bool saved = PcSaveHelper.SaveSlot(SLOT_COUNT - 1, true);
        homeSaveActive = false;
        CHud::SetPriorityHelpMessage(GetText(saved ? "EN_ASOK" : "EN_ASER"));
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
    if (routeCount < 2 || (!GPS && CRadar::TargetMarkerId == -1) || !FindPlayerVehicle()) return;
    for (int i = 1; i < routeCount; ++i) {
        CVector2D a, b, sa, sb;
        const CVector aWorld = i == 1 ? FindPlayerCoors() : route[i-1];
        if (i == 1 &&
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
