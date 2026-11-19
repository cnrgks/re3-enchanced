#include "common.h"
#include "Enhanced.h"
#include "EnhancedScript.h"
#include "Frontend.h"
#include "Garages.h"
#include "Phones.h"
#include "Pickups.h"
#include "PlayerInfo.h"
#include "Radar.h"
#include "Script.h"
#include "ScriptCommands.h"
#include "Stats.h"
#include "Timer.h"
#include "World.h"
#include "Zones.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Enhanced {
namespace {
struct Service { int sprite; CVector position; bool fallback = false; };
Service services[256];
int serviceCount;
uint32 lastDiscovery;
std::vector<CVector> scriptHomes;
std::vector<Service> scriptPhones;
bool scanned;
bool IsServiceSprite(int sprite)
{
    // EIGHT is 8-Ball's mission contact. BOMB is the bomb-shop service.
    return sprite == RADAR_SPRITE_SPRAY || sprite == RADAR_SPRITE_WEAPON ||
        sprite == RADAR_SPRITE_BOMB;
}

bool SameService(const Service &service, int sprite, const CVector &position)
{
    if (service.sprite != sprite) return false;
    // Shop pickups and their native blip describe the same storefront.
    const float tolerance = sprite == RADAR_SPRITE_WEAPON ? 20.0f : 10.0f;
    return (service.position-position).Magnitude2D() < tolerance;
}

eLevelName IslandAt(const CVector &position)
{
    // SCM uses a floor-height sentinel for several coordinate blips.
    return CTheZones::FindZoneForPoint(CVector(position.x, position.y, 0.0f));
}

eLevelName PlayerIsland()
{
    const eLevelName island = IslandAt(FindPlayerCoors());
    return island == LEVEL_GENERIC ? CGame::currLevel : island;
}

bool IsIslandUnlocked(eLevelName island)
{
    switch (island) {
    case LEVEL_COMMERCIAL: return CStats::IndustrialPassed;
    case LEVEL_SUBURBAN: return CStats::CommercialPassed;
    default: return true;
    }
}

bool IsUnlocked(const CVector &position)
{
    return IsIslandUnlocked(IslandAt(position));
}

bool IsPhoneSprite(int sprite)
{
    return sprite == RADAR_SPRITE_EL || sprite == RADAR_SPRITE_ICE || sprite == RADAR_SPRITE_LIZ;
}

bool IsVisibleCoordBlip(const sRadarTrace &trace, int sprite)
{
    return trace.m_bInUse && trace.m_eRadarSprite == sprite &&
        (trace.m_eBlipType == BLIP_COORD || trace.m_eBlipType == BLIP_CONTACT_POINT) &&
        (trace.m_eBlipDisplay == BLIP_DISPLAY_BOTH || trace.m_eBlipDisplay == BLIP_DISPLAY_BLIP_ONLY);
}

void ScanScriptLocations()
{
    if (scanned) return;
    const uint8 *data = CTheScripts::ScriptSpace;
    const int globalsEnd = CTheScripts::GetSizeOfVariableSpace();
    if (globalsEnd < 8 || globalsEnd >= SIZE_MAIN_SCRIPT) return;
    scanned = true;
    // Recognize the full typed instruction shape, never a fixed EXE address.
    for (size_t offset = globalsEnd; offset+17 < SIZE_MAIN_SCRIPT; ++offset) {
        uint16 command;
        memcpy(&command, data+offset, 2);
        if (command != COMMAND_ADD_SPRITE_BLIP_FOR_COORD &&
            command != COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT) continue;
        size_t at = offset+2;
        float x,y,z,sprite;
        if (!EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,x,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,y,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,z,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,sprite,false)) continue;
        if (at+3 > SIZE_MAIN_SCRIPT || (data[at] != 2 && data[at] != 3) ||
            Abs(x)>2000.0f || Abs(y)>2000.0f || z < -101.0f || z>1000.0f ||
            sprite < 0 || sprite >= RADAR_SPRITE_COUNT || sprite != float(int(sprite))) continue;
        const CVector position(x, y, z);
        if (sprite == RADAR_SPRITE_SAVE)
            scriptHomes.push_back(position);
        if (command == COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT && IsPhoneSprite(int(sprite)) &&
            std::none_of(scriptPhones.begin(), scriptPhones.end(), [&](const Service &phone) {
                return phone.sprite == int(sprite) && (phone.position-position).Magnitude2D() < 1.0f;
            }))
            scriptPhones.push_back({int(sprite), position});
    }
}
}
void ResetServices()
{
    serviceCount = 0;
    lastDiscovery = 0;
    scriptHomes.clear();
    scriptPhones.clear();
    scanned = false;
}

static Service *FindDiscovered(int sprite, const CVector &position)
{
    for (int i = 0; i < serviceCount; ++i)
        if (SameService(services[i], sprite, position)) return &services[i];
    return nil;
}

static void RememberService(int sprite, const CVector &position, bool fallback)
{
    Service *existing = FindDiscovered(sprite, position);
    if (existing) {
        if (fallback && !existing->fallback) *existing = {sprite, position, true};
        return;
    }
    if (serviceCount < int(ARRAY_SIZE(services))) services[serviceCount++] = {sprite, position, fallback};
}

static bool IsServiceBlip(const sRadarTrace &trace)
{
    return IsServiceSprite(trace.m_eRadarSprite) && !CRadar::IsMissionSpriteBlip(trace);
}

bool ServiceBlipVisible(const sRadarTrace &trace, float radarDistance)
{
#ifdef MENU_MAP
    const int sprite = trace.m_eRadarSprite;
    if (sprite == RADAR_SPRITE_SAVE) {
        const eLevelName island = IslandAt(trace.m_vecPos);
        return IsIslandUnlocked(island) &&
            (CMenuManager::bMenuMapActive || island == PlayerIsland());
    }
    if (!IsServiceBlip(trace)) return true;
    if (!IsUnlocked(trace.m_vecPos)) return false;
    if (!CMenuManager::bMenuMapActive && radarDistance <= 1.0f)
        RememberService(sprite, trace.m_vecPos, false);
    return FindDiscovered(sprite, trace.m_vecPos) != nil;
#else
    return true;
#endif
}

// This optional versioned prefix lives in PC save padding. Native blocks and
// their lengths are untouched, and the normal save checksum covers these bytes.
static const uint8 discoveryMagic[] = {'R', '3', 'D', 'I', 'S', 'C', 1, 0};
static const uint32 discoveryHeaderSize = sizeof(discoveryMagic) + sizeof(uint32);
static const uint32 discoveryEntrySize = sizeof(uint32) + 3*sizeof(float);

void SaveServiceDiscovery(uint8 *buffer, uint32 capacity)
{
    if (capacity < discoveryHeaderSize + uint32(serviceCount)*discoveryEntrySize) return;
    const uint32 count = serviceCount;
    memcpy(buffer, discoveryMagic, sizeof(discoveryMagic));
    memcpy(buffer + sizeof(discoveryMagic), &count, sizeof(count));
    buffer += discoveryHeaderSize;
    for (uint32 i = 0; i < count; ++i) {
        const Service &service = services[i];
        const uint32 flags = uint32(service.sprite) | (service.fallback ? 0x100u : 0u);
        const float position[] = {service.position.x, service.position.y, service.position.z};
        memcpy(buffer, &flags, sizeof(flags));
        memcpy(buffer + sizeof(flags), position, sizeof(position));
        buffer += discoveryEntrySize;
    }
}

void LoadServiceDiscovery(const uint8 *buffer, uint32 size)
{
    serviceCount = 0;
    if (size < discoveryHeaderSize || memcmp(buffer, discoveryMagic, sizeof(discoveryMagic)) != 0) return;
    uint32 count;
    memcpy(&count, buffer + sizeof(discoveryMagic), sizeof(count));
    if (count > ARRAY_SIZE(services) || count > (size-discoveryHeaderSize)/discoveryEntrySize) return;
    buffer += discoveryHeaderSize;
    for (uint32 i = 0; i < count; ++i, buffer += discoveryEntrySize) {
        uint32 flags;
        float position[3];
        memcpy(&flags, buffer, sizeof(flags));
        memcpy(position, buffer + sizeof(flags), sizeof(position));
        const int sprite = int(flags & 0xffu);
        if ((flags & ~0x1ffu) || !IsServiceSprite(sprite) ||
            !std::isfinite(position[0]) || !std::isfinite(position[1]) || !std::isfinite(position[2]) ||
            Abs(position[0]) > 2000.0f || Abs(position[1]) > 2000.0f || Abs(position[2]) > 2000.0f) continue;
        const bool fallback = (flags & 0x100u) != 0;
        RememberService(sprite, CVector(position[0], position[1], position[2]), fallback);
    }
}

void DiscoverServices()
{
#ifdef MENU_MAP
    if (CMenuManager::bMenuMapActive) return;
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (lastDiscovery && now-lastDiscovery < 100) return;
    lastDiscovery = now;
    const auto nearby = [&](const CVector &position) {
        CVector2D radar;
        CRadar::TransformRealWorldPointToRadarSpace(radar, CVector2D(position));
        return radar.MagnitudeSqr() <= 1.0f && IsUnlocked(position);
    };
    for (uint32 i=0;i<Min(CGarages::NumGarages,uint32(NUM_GARAGES));++i) {
        CGarage &garage=CGarages::aGarages[i];
        int sprite=garage.m_eGarageType==GARAGE_RESPRAY ? RADAR_SPRITE_SPRAY :
            (garage.m_eGarageType>=GARAGE_BOMBSHOP1 && garage.m_eGarageType<=GARAGE_BOMBSHOP3 ? RADAR_SPRITE_BOMB : 0);
        const CVector position(garage.GetGarageCenterX(),garage.GetGarageCenterY(),garage.m_fZ1);
        if (sprite && nearby(position)) RememberService(sprite,position,true);
    }
    for (int i=0;i<NUMPICKUPS;++i) {
        const CPickup &pickup=CPickups::aPickUps[i];
        if ((pickup.m_eType==PICKUP_IN_SHOP || pickup.m_eType==PICKUP_IN_SHOP_OUT_OF_STOCK) &&
            CPickups::WeaponForModel(pickup.m_eModelIndex)!=WEAPONTYPE_UNARMED && nearby(pickup.m_vecPos))
            RememberService(RADAR_SPRITE_WEAPON,pickup.m_vecPos,true);
    }
#endif
}

void DrawServices()
{
#ifdef MENU_MAP
    const bool inMap = CMenuManager::bMenuMapActive;
    const bool hideContactPoints = !inMap && CTheScripts::IsPlayerOnAMission();
    for (int i=0;i<serviceCount;++i) {
        const Service &service = services[i];
        if (!service.fallback || !IsUnlocked(service.position)) continue;
        CVector2D radar, screen;
        CRadar::TransformRealWorldPointToRadarSpace(radar,CVector2D(service.position));
        const float radarDistance = CRadar::LimitRadarPoint(radar);
        bool hasRadarBlip = false;
        for (const sRadarTrace &trace : CRadar::ms_RadarTrace) {
            if (!trace.m_bInUse || trace.m_eRadarSprite != service.sprite ||
                (trace.m_eBlipType != BLIP_COORD && trace.m_eBlipType != BLIP_CONTACT_POINT))
                continue;
            if ((trace.m_eBlipDisplay != BLIP_DISPLAY_BOTH && trace.m_eBlipDisplay != BLIP_DISPLAY_BLIP_ONLY) ||
                (trace.m_eBlipType == BLIP_CONTACT_POINT && hideContactPoints) ||
                !SameService(service, trace.m_eRadarSprite, trace.m_vecPos)) continue;
            hasRadarBlip = true;
            break;
        }
        if (hasRadarBlip) continue;
        CRadar::TransformRadarPointToScreenSpace(screen,radar);
        CRadar::DrawRadarSprite(service.sprite,screen.x,screen.y,CRadar::CalculateBlipAlpha(radarDistance));
    }
#endif
}
static bool FindHomeCoordinates(eLevelName island, CVector &home)
{
    ScanScriptLocations();
    // Original safehouse entrances are world coordinates, not executable
    // addresses. Prefer SCM metadata, so scripts can relocate the houses.
    switch (island) {
    case LEVEL_INDUSTRIAL: home=CVector(893.5f,-306.0625f,0); break;
    case LEVEL_COMMERCIAL: home=CVector(103.0f,-478.5f,0); break;
    case LEVEL_SUBURBAN: home=CVector(-665.0f,-6.5f,0); break;
    default: return false;
    }
    const CVector entrance=home;
    float nearest=1.0e20f;
    const auto consider = [&](const CVector &position) {
        if (IslandAt(position) != island) return;
        const float distance=(position-entrance).Magnitude2D();
        if (distance < nearest) { nearest=distance; home.x=position.x; home.y=position.y; }
    };
    for (const CVector &position : scriptHomes) consider(position);
    for (const sRadarTrace &trace : CRadar::ms_RadarTrace)
        if (trace.m_bInUse && trace.m_eRadarSprite == RADAR_SPRITE_SAVE &&
            (trace.m_eBlipType == BLIP_COORD || trace.m_eBlipType == BLIP_CONTACT_POINT))
            consider(trace.m_vecPos);
    return true;
}

bool FindHomePosition(const CVector &current, CVector &home)
{
    eLevelName island = CTheZones::FindZoneForPoint(current);
    if (island == LEVEL_GENERIC) island = CGame::currLevel;
    // These are the on-foot save trigger bounds from the original main.scm.
    // Radar icons are positioned independently of the save triggers.
    CVector lower, upper;
    switch (island) {
    case LEVEL_INDUSTRIAL:
        lower = CVector(891.1875f, -309.6875f, 7.6875f);
        upper = CVector(899.25f, -303.25f, 12.6875f);
        break;
    case LEVEL_COMMERCIAL:
        lower = CVector(100.75f, -486.0f, 15.0f);
        upper = CVector(105.6875f, -483.375f, 18.0f);
        break;
    case LEVEL_SUBURBAN:
        lower = CVector(-670.5f, 3.875f, 18.0f);
        upper = CVector(-660.0f, 12.6875f, 22.0f);
        break;
    default:
        return false;
    }

    home = (lower + upper) * 0.5f;
    bool foundGround;
    const float standingZ = CWorld::FindGroundZFor3DCoord(home.x, home.y, home.z, &foundGround) + 1.0f;
    if (foundGround && standingZ >= lower.z && standingZ <= upper.z)
        home.z = standingZ;
    return true;
}

void DrawHomeAndPhoneBlips()
{
#ifdef MENU_MAP
    const bool inMap = CMenuManager::bMenuMapActive;
    const auto draw = [](int sprite, const CVector &position) {
        CVector2D radar, screen;
        CRadar::TransformRealWorldPointToRadarSpace(radar, CVector2D(position));
        const float distance = CRadar::LimitRadarPoint(radar);
        CRadar::TransformRadarPointToScreenSpace(screen, radar);
        CRadar::DrawRadarSprite(sprite, screen.x, screen.y, CRadar::CalculateBlipAlpha(distance));
    };
    const eLevelName current = PlayerIsland();
    for (int level = LEVEL_INDUSTRIAL; level <= LEVEL_SUBURBAN; ++level) {
        const auto island = eLevelName(level);
        if (!IsIslandUnlocked(island) || (!inMap && island != current)) continue;
        if (std::any_of(CRadar::ms_RadarTrace, CRadar::ms_RadarTrace + NUMRADARBLIPS, [&](const sRadarTrace &trace) {
            return IsVisibleCoordBlip(trace, RADAR_SPRITE_SAVE) && IslandAt(trace.m_vecPos) == island;
        })) continue;
        CVector home;
        if (FindHomeCoordinates(island, home)) draw(RADAR_SPRITE_SAVE, home);
    }

    if (!inMap && CTheScripts::IsPlayerOnAMission()) return;
    ScanScriptLocations();
    for (const Service &marker : scriptPhones) {
        if (!IsUnlocked(marker.position)) continue;
        if (std::any_of(CRadar::ms_RadarTrace, CRadar::ms_RadarTrace + NUMRADARBLIPS, [&](const sRadarTrace &trace) {
            return IsVisibleCoordBlip(trace, marker.sprite) &&
                (trace.m_vecPos-marker.position).Magnitude2D() < 3.0f;
        })) continue;
        // An enabled phone can outlive its native blip. Restore the game's
        // actual script icon while the phone still offers its mission.
        for (int i = 0; i < Min(gPhoneInfo.m_nScriptPhonesMax, int(ARRAY_SIZE(gPhoneInfo.m_aPhones))); ++i) {
            const CPhone &phone = gPhoneInfo.m_aPhones[i];
            if (!phone.m_pEntity || (phone.m_vecPos-marker.position).Magnitude2D() >= 3.0f) continue;
            if (phone.m_nState == PHONE_STATE_9 || phone.m_nState == PHONE_STATE_ONETIME_MESSAGE_SET ||
                phone.m_nState == PHONE_STATE_REPEATED_MESSAGE_SET || phone.m_nState == PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE)
                draw(marker.sprite, marker.position);
            break;
        }
    }
#endif
}
}
