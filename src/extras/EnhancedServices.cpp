#include "common.h"
#include "Enhanced.h"
#include "EnhancedScript.h"
#include "Frontend.h"
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
            command != COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT) continue;
        size_t at = offset+2;
        float x,y,z,sprite;
        if (!EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,x,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,y,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,z,true) ||
            !EnhancedScript::Number(data,SIZE_MAIN_SCRIPT,globalsEnd,at,sprite,false)) continue;
        if (at+3 > SIZE_MAIN_SCRIPT || (data[at] != 2 && data[at] != 3) ||
            Abs(x)>2000.0f || Abs(y)>2000.0f || z < -101.0f || z>1000.0f ||
            sprite != float(int(sprite)) || !IsService(int(sprite))) continue;
        RememberService(int(sprite), CVector(x,y,z));
    }
}
}
void ResetServices() { serviceCount=0; scanned=false; }
void RememberService(int sprite, const CVector &position)
{
    if (!IsService(sprite)) return;
    for (int i=0;i<serviceCount;++i)
        if (services[i].sprite == sprite && (services[i].position-position).Magnitude2D()<12.0f)
            return;
    if (serviceCount < int(ARRAY_SIZE(services))) services[serviceCount++]={sprite,position};
}
void DrawServices()
{
#ifdef MENU_MAP
    if (!CMenuManager::bMenuMapActive) return;
    ScanServices();
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
    float nearest=1.0e20f;
    for (int i=0;i<serviceCount;++i) {
        const Service &service=services[i];
        if (service.sprite != RADAR_SPRITE_SAVE ||
            CTheZones::FindZoneForPoint(service.position) != island) continue;
        const float distance=(service.position-home).Magnitude2D();
        if (distance < nearest) { nearest=distance; home.x=service.position.x; home.y=service.position.y; }
    }
    // Collision for the current island is loaded. Position the saved ped above
    // the ground, with no live teleport, scene reload or traffic destruction.
    home.z=CWorld::FindGroundZForCoord(home.x,home.y)+1.0f;
    return true;
}
}
