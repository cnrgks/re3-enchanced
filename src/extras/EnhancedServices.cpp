#include "common.h"
#include "Enhanced.h"
#include "EnhancedScript.h"
#include "Frontend.h"
#include "Garages.h"
#include "Phones.h"
#include "Pickups.h"
#include "Radar.h"
#include "Script.h"
#include "ScriptCommands.h"
#include "Stats.h"
#include "Timer.h"
#include "World.h"
#include "Zones.h"
#include <vector>

namespace Enhanced {
namespace {
struct Service { int sprite; CVector position; };
Service services[128];
int serviceCount;
uint32 lastDiscovery;
std::vector<CVector> scriptHomes;
std::vector<Service> scriptPhones;
bool scanned;
bool IsPhoneSprite(int sprite)
{
    return sprite == RADAR_SPRITE_EL || sprite == RADAR_SPRITE_ICE || sprite == RADAR_SPRITE_LIZ;
}

const CPhone *FindScriptPhone(const CVector &position)
{
    const CPhone *nearest = nil;
    float distance = 3.0f;
    for (int i = 0; i < Min(gPhoneInfo.m_nScriptPhonesMax, int(ARRAY_SIZE(gPhoneInfo.m_aPhones))); ++i) {
        const CPhone &phone = gPhoneInfo.m_aPhones[i];
        if (!phone.m_pEntity) continue;
        const float candidate = (phone.m_vecPos-position).Magnitude2D();
        if (candidate < distance) {
            nearest = &phone;
            distance = candidate;
        }
    }
    return nearest;
}

bool OffersPhoneMission(const CPhone &phone)
{
    return phone.m_nState == PHONE_STATE_9 ||
        phone.m_nState == PHONE_STATE_ONETIME_MESSAGE_SET ||
        phone.m_nState == PHONE_STATE_REPEATED_MESSAGE_SET ||
        phone.m_nState == PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE;
}

bool IsService(int sprite)
{
    return sprite == RADAR_SPRITE_SPRAY || sprite == RADAR_SPRITE_WEAPON ||
        sprite == RADAR_SPRITE_BOMB || sprite == RADAR_SPRITE_SAVE;
}

bool SamePosition(const CVector &a, const CVector &b)
{
    return (a-b).Magnitude2D() < 1.0f;
}

bool IsUnlocked(const CVector &position)
{
    switch (CTheZones::FindZoneForPoint(position)) {
    case LEVEL_COMMERCIAL: return CStats::IndustrialPassed;
    case LEVEL_SUBURBAN: return CStats::CommercialPassed;
    default: return true;
    }
}

void ScanScriptLocations()
{
    if (scanned) return;
    const uint8 *data = CTheScripts::ScriptSpace;
    const int globalsEnd = CTheScripts::GetSizeOfVariableSpace();
    if (globalsEnd < 8 || globalsEnd >= SIZE_MAIN_SCRIPT) return;
    scanned = true;
    // Recognize the full typed instruction shape, never a fixed EXE address.
    // Safehouse metadata is used for saves. Phone declarations supply the
    // original sprite when a player discovers an active script phone nearby.
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
        if (sprite == RADAR_SPRITE_SAVE)
            scriptHomes.emplace_back(x,y,z);
        if (command == COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT && IsPhoneSprite(int(sprite)))
            scriptPhones.push_back({int(sprite), CVector(x,y,z)});
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

bool ServiceBlipVisibleOnMap(int sprite, const CVector &position)
{
    return !(IsService(sprite) || IsPhoneSprite(sprite)) || IsUnlocked(position);
}

static void RememberService(int sprite, const CVector &position)
{
    if (!IsService(sprite) && !IsPhoneSprite(sprite)) return;
    for (int i=0;i<serviceCount;++i)
        if (services[i].sprite == sprite && SamePosition(services[i].position, position))
            return;
    if (serviceCount < int(ARRAY_SIZE(services))) services[serviceCount++]={sprite,position};
}

static void RememberPhoneBlips()
{
    for (const sRadarTrace &trace : CRadar::ms_RadarTrace) {
        // Compatible saves retain the last coordinates of cleared contact blips.
        if (trace.m_eBlipType != BLIP_CONTACT_POINT || !IsPhoneSprite(trace.m_eRadarSprite)) continue;
        const CPhone *phone = FindScriptPhone(trace.m_vecPos);
        if (phone && OffersPhoneMission(*phone))
            RememberService(trace.m_eRadarSprite, phone->m_vecPos);
    }
}

void DiscoverServices(const CVector &player)
{
#ifdef MENU_MAP
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (lastDiscovery && now-lastDiscovery < 500) return;
    lastDiscovery = now;
    const auto nearby = [&](const CVector &position) {
        return (player-position).MagnitudeSqr() < 45.0f*45.0f && IsUnlocked(position);
    };
    RememberPhoneBlips();
    ScanScriptLocations();
    for (const Service &marker : scriptPhones) {
        const CPhone *phone = FindScriptPhone(marker.position);
        if (phone && nearby(phone->m_vecPos) && OffersPhoneMission(*phone))
            RememberService(marker.sprite, phone->m_vecPos);
    }
    for (uint32 i=0;i<Min(CGarages::NumGarages,uint32(NUM_GARAGES));++i) {
        CGarage &garage=CGarages::aGarages[i];
        int sprite=garage.m_eGarageType==GARAGE_RESPRAY ? RADAR_SPRITE_SPRAY :
            (garage.m_eGarageType>=GARAGE_BOMBSHOP1 && garage.m_eGarageType<=GARAGE_BOMBSHOP3 ? RADAR_SPRITE_BOMB : 0);
        const CVector position(garage.GetGarageCenterX(),garage.GetGarageCenterY(),garage.m_fZ1);
        if (sprite && nearby(position)) RememberService(sprite,position);
    }
    for (int i=0;i<NUMPICKUPS;++i) {
        const CPickup &pickup=CPickups::aPickUps[i];
        if ((pickup.m_eType==PICKUP_IN_SHOP || pickup.m_eType==PICKUP_IN_SHOP_OUT_OF_STOCK) &&
            CPickups::WeaponForModel(pickup.m_eModelIndex)!=WEAPONTYPE_UNARMED && nearby(pickup.m_vecPos))
            RememberService(RADAR_SPRITE_WEAPON,pickup.m_vecPos);
    }
#endif
}

void DrawServices()
{
#ifdef MENU_MAP
    if (!CMenuManager::bMenuMapActive) return;
    RememberPhoneBlips();
    const bool onMission = CTheScripts::IsPlayerOnAMission();
    for (int i=0;i<serviceCount;++i) {
        const Service &service = services[i];
        if (!IsUnlocked(service.position)) continue;
        const bool isPhone = IsPhoneSprite(service.sprite);
        if (isPhone) {
            const CPhone *phone = FindScriptPhone(service.position);
            if (onMission || !phone || !OffersPhoneMission(*phone)) continue;
        }
        bool hasRadarBlip = false;
        for (const sRadarTrace &trace : CRadar::ms_RadarTrace) {
            if (!trace.m_bInUse || trace.m_eRadarSprite != service.sprite ||
                (trace.m_eBlipType != BLIP_COORD && trace.m_eBlipType != BLIP_CONTACT_POINT))
                continue;
            if (!isPhone &&
                ((trace.m_eBlipDisplay != BLIP_DISPLAY_BOTH && trace.m_eBlipDisplay != BLIP_DISPLAY_BLIP_ONLY) ||
                 (trace.m_eBlipType == BLIP_CONTACT_POINT && onMission))) continue;
            const bool sameLocation = isPhone ?
                FindScriptPhone(trace.m_vecPos) == FindScriptPhone(service.position) :
                SamePosition(trace.m_vecPos, service.position);
            if (sameLocation) {
                hasRadarBlip = true;
                break;
            }
        }
        if (hasRadarBlip) continue;
        CVector2D radar, screen;
        CRadar::TransformRealWorldPointToRadarSpace(radar,CVector2D(service.position));
        CRadar::TransformRadarPointToScreenSpace(screen,radar);
        CRadar::DrawRadarSprite(service.sprite,screen.x,screen.y,255);
    }
#endif
}
bool FindHomePosition(const CVector &current, CVector &home)
{
    ScanScriptLocations();
    eLevelName island = CTheZones::FindZoneForPoint(current);
    if (island < LEVEL_INDUSTRIAL || island > LEVEL_SUBURBAN) island=CGame::currLevel;
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
        if (CTheZones::FindZoneForPoint(position) != island) return;
        const float distance=(position-entrance).Magnitude2D();
        if (distance < nearest) { nearest=distance; home.x=position.x; home.y=position.y; }
    };
    for (const CVector &position : scriptHomes) consider(position);
    for (const sRadarTrace &trace : CRadar::ms_RadarTrace)
        if (trace.m_bInUse && trace.m_eRadarSprite == RADAR_SPRITE_SAVE &&
            (trace.m_eBlipType == BLIP_COORD || trace.m_eBlipType == BLIP_CONTACT_POINT))
            consider(trace.m_vecPos);
    // Collision for the current island is loaded. Position the saved ped above
    // the ground, with no live teleport, scene reload or traffic destruction.
    home.z=CWorld::FindGroundZForCoord(home.x,home.y)+1.0f;
    return true;
}
}
