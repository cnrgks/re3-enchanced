#pragma once

// Built-in features. Discovery uses optional data inside existing PC save padding.
class CEntity;
class CColPoint;
struct sRadarTrace;
namespace Enhanced {
extern int8 QuickSave, AutoSave, GPS, ScreenFormat;
extern int8 DistantCrowdedWorld;
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
struct VehicleTrafficDiagnostics {
    uint32 targetFull, carBudgetFull, pedPoolLow, occupantBudgetFull, noModels;
    uint32 routeRejected, laneRejected, groundRejected, visibilityRejected, collisionRejected, directionRejected;
    uint32 rangeRemoved, offscreenRemoved, movingOffscreenKept;
};
extern VehicleTrafficDiagnostics VehicleTrafficStats;
int TrafficAttempts(bool vehicles, const CVector &centre, float nearbyRadius, bool warmingUp);
int TrafficTarget(bool vehicles, int limit, bool warmingUp);
bool IsTrafficSpawnOccluded(const CVector &position);
void ApplyScreenFormatSettings();
extern int8 HealAfterSave, Borderless;

wchar *GetText(const char *key);
void Process();
bool IsQuickSaveMenu();
void ResetQuickSaveMenu();
void Reset();
void ProcessSelectionWheel();
void CancelSelectionWheelIfUnavailable();
void DrawSelectionWheel();
void FreeSelectionWheelTextures();
bool IsWeaponWheelOpen();
bool IsRadioWheelOpen();
extern int8 RadioWheel, WeaponWheel;
bool IsSelectionWheelOpen();
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
bool GetAutosaveExitHeading(const CVector &position, float &heading);
CVector SavePosition(const CVector &original);
float VehicleRangeMultiplier();
float PedRangeMultiplier();
void OpenMap(int8 action);
}
