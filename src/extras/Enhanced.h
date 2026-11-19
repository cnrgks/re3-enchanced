#pragma once

// Built-in features. Discovery uses optional data inside existing PC save padding.
class CEntity;
class CColPoint;
struct sRadarTrace;
namespace Enhanced {
extern int8 QuickSave, AutoSave, RadarZoom, GPS, MenuMap;
extern int8 VehicleDistance, PedDistance, ScreenFormat;
extern int8 VehicleTrafficMultiplier, PedTrafficMultiplier;
int TrafficMultiplier(bool vehicles);
extern int8 BorderlessCaptureProbe, FullscreenDXGIFlipY;
extern bool TraceVehicleEntry;
extern bool TraceTraffic;
struct TrafficSpawnStats {
    uint32 attempts[2];
    uint32 spawned[2];
    uint32 noCandidate;
    uint32 exposed;
};
extern TrafficSpawnStats VehicleSpawnStats, PedSpawnStats;
int TrafficAttempts(bool vehicles, const CVector &centre, float nearbyRadius, bool warmingUp);
int TrafficTarget(bool vehicles, int limit, bool warmingUp);
bool IsTrafficSpawnOccluded(const CVector &position);
void ApplyScreenFormatSettings();
extern int8 VehicleDetail, PedDetail, HealAfterSave, BreakableWindscreen, Borderless;

wchar *GetText(const char *key);
void Process();
void Reset();
void DrawRoute();
void WaypointChanged();
void DrawMapFallback();
void OnMissionPassed();
float RadarRange(float nativeRange);
float BlipScale();
void WindowModeChanged(int8 before, int8 after);
#if defined(RW_GL3) && !defined(LIBRW_SDL2)
void ApplyWindowMode(bool newWindow = false);
#endif
bool ProcessWindscreenShot(const CVector &source, const CVector &target, CEntity *shooter, bool hit, CColPoint &point, CEntity *&entity, bool hitOccupants = true);
void ResetServices();
void DiscoverServices();
bool IsServiceBlip(const sRadarTrace &trace);
bool ServiceBlipVisible(const sRadarTrace &trace, float radarDistance);
void SaveServiceDiscovery(uint8 *buffer, uint32 capacity);
void LoadServiceDiscovery(const uint8 *buffer, uint32 size);
void DrawServices();
void DrawPhoneBlips();
bool IsSavingAtHome();
bool GetLegacyAutosaveExit(const CVector &position, CVector &exit);
CVector SavePosition(const CVector &original);
float VehicleRangeMultiplier();
float PedRangeMultiplier();
void OpenMap(int8 action);
wchar *MapOptionDraw(bool *disabled, bool hovering);
}
