#include "common.h"
#include "Enhanced.h"
#include "EnhancedScript.h"
#include "Frontend.h"
#include "Garages.h"
#include "Pickups.h"
#include "Radar.h"
#include "Script.h"
#include "ScriptCommands.h"
#include "World.h"
#include "Zones.h"

namespace Enhanced {
namespace {
struct Service { int sprite; CVector position; };
Service services[128];
int serviceCount;
bool scanned;
bool IsService(int sprite)
{
    return sprite == RADAR_SPRITE_SPRAY || sprite == RADAR_SPRITE_WEAPON ||
        sprite == RADAR_SPRITE_BOMB || sprite == RADAR_SPRITE_SAVE;
}
void ScanServices()
{
    if (scanned) return;
    const uint8 *data = CTheScripts::ScriptSpace;
    const int globalsEnd = CTheScripts::GetSizeOfVariableSpace();
    if (globalsEnd < 8 || globalsEnd >= SIZE_MAIN_SCRIPT) return;
    scanned = true;
    // Recognize the full typed instruction shape, never a fixed EXE address.
    // Runtime SetBlipSprite also remembers services whose coordinates are local.
    for (size_t offset = globalsEnd; offset+17 < SIZE_MAIN_SCRIPT; ++offset) {
        uint16 command;
        memcpy(&command, data+offset, 2);
        if (command != COMMAND_ADD_SPRITE_BLIP_FOR_COORD &&
            command != COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT &&
            command != COMMAND_CREATE_PICKUP && command != COMMAND_CREATE_PICKUP_WITH_AMMO) continue;
        size_t at = offset+2;
        if (command == COMMAND_CREATE_PICKUP || command == COMMAND_CREATE_PICKUP_WITH_AMMO) {
            float model,type,ammo,x,y,z;
            if (!EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,model,false) ||
                !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,type,false)) continue;
            if (type != PICKUP_IN_SHOP && type != PICKUP_IN_SHOP_OUT_OF_STOCK) continue;
            if (command == COMMAND_CREATE_PICKUP_WITH_AMMO &&
                !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,ammo,false)) continue;
            if (!EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,x,true) ||
                !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,y,true) ||
                !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,z,true) ||
                at+3>SIZE_MAIN_SCRIPT || (data[at]!=2 && data[at]!=3) ||
                Abs(x)>2000 || Abs(y)>2000 || z < -101 || z>1000 ||
                model < -float(ARRAY_SIZE(CTheScripts::UsedObjectArray))+1 || model>=MODELINFOSIZE) continue;
            int index=int(model);
            if (index<0) index=CTheScripts::UsedObjectArray[-index].index;
            if (CPickups::WeaponForModel(index)!=WEAPONTYPE_UNARMED)
                RememberService(RADAR_SPRITE_WEAPON,CVector(x,y,z));
            continue;
        }
        float x,y,z,sprite;
        if (!EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,x,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,y,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,z,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,sprite,false)) continue;
        if (at+3 > SIZE_MAIN_SCRIPT || (data[at] != 2 && data[at] != 3) ||
            Abs(x)>2000.0f || Abs(y)>2000.0f || z < -101.0f || z>1000.0f ||
            sprite<0 || sprite>=RADAR_SPRITE_COUNT || sprite != float(int(sprite)) || !IsService(int(sprite))) continue;
        RememberService(int(sprite), CVector(x,y,z));
    }
}
}
void ResetServices() { serviceCount=0; scanned=false; }
void RememberService(int sprite, const CVector &position)
{
    if (!IsService(sprite)) return;
    for (int i=0;i<serviceCount;++i)
        if (services[i].sprite == sprite && (services[i].position-position).Magnitude2D()<((sprite==RADAR_SPRITE_WEAPON || sprite==RADAR_SPRITE_SAVE) ? 40.0f : 12.0f))
            return;
    if (serviceCount < int(ARRAY_SIZE(services))) services[serviceCount++]={sprite,position};
}
void DrawServices()
{
#ifdef MENU_MAP
    if (!CMenuManager::bMenuMapActive) return;
    ScanServices();
    for (uint32 i=0;i<Min(CGarages::NumGarages,uint32(NUM_GARAGES));++i) {
        CGarage &garage=CGarages::aGarages[i];
        int sprite=garage.m_eGarageType==GARAGE_RESPRAY ? RADAR_SPRITE_SPRAY :
            (garage.m_eGarageType>=GARAGE_BOMBSHOP1 && garage.m_eGarageType<=GARAGE_BOMBSHOP3 ? RADAR_SPRITE_BOMB : 0);
        if (sprite) RememberService(sprite,CVector(garage.GetGarageCenterX(),garage.GetGarageCenterY(),garage.m_fZ1));
    }
    for (int i=0;i<NUMPICKUPS;++i) {
        const CPickup &pickup=CPickups::aPickUps[i];
        if ((pickup.m_eType==PICKUP_IN_SHOP || pickup.m_eType==PICKUP_IN_SHOP_OUT_OF_STOCK) &&
            CPickups::WeaponForModel(pickup.m_eModelIndex)!=WEAPONTYPE_UNARMED)
            RememberService(RADAR_SPRITE_WEAPON,pickup.m_vecPos);
    }
    for (int i=0;i<serviceCount;++i) {
        CVector2D radar, screen;
        CRadar::TransformRealWorldPointToRadarSpace(radar,CVector2D(services[i].position));
        CRadar::TransformRadarPointToScreenSpace(screen,radar);
        CRadar::DrawRadarSprite(services[i].sprite,screen.x,screen.y,255);
    }
#endif
}
bool FindHomePosition(const CVector &current, CVector &home)
{
    ScanServices();
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
    for (int i=0;i<serviceCount;++i) {
        const Service &service=services[i];
        if (service.sprite != RADAR_SPRITE_SAVE ||
            CTheZones::FindZoneForPoint(service.position) != island) continue;
        const float distance=(service.position-entrance).Magnitude2D();
        if (distance < nearest) { nearest=distance; home.x=service.position.x; home.y=service.position.y; }
    }
    // Collision for the current island is loaded. Position the saved ped above
    // the ground, with no live teleport, scene reload or traffic destruction.
    home.z=CWorld::FindGroundZForCoord(home.x,home.y)+1.0f;
    return true;
}
}
