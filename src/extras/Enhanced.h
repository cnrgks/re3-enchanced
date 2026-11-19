#pragma once

// Built-in features. These do not change any game's class layout or save format.
class CEntity;
class CColPoint;
namespace Enhanced {
extern int8 QuickSave, AutoSave, RadarZoom, GPS, MenuMap;
extern int8 VehicleDistance, PedDistance, ScreenFormat;
extern int8 BorderlessCaptureProbe, FullscreenDXGI, FullscreenDXGIFlipY;
extern bool TraceVehicleEntry;
void FullscreenPresenterChanged(int8 before,int8 after);
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
void AfterSuccessfulSave();
void ResetServices();
void RememberService(int sprite, const CVector &position);
void DrawServices();
bool BeginHomeSave();
void EndHomeSave();
CVector SavePosition(const CVector &original);
float VehicleRangeMultiplier();
float PedRangeMultiplier();
void OpenMap(int8 action);
wchar *MapOptionDraw(bool *disabled, bool hovering);
}
