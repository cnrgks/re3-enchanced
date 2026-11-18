#pragma once

// Built-in features. These do not change any game's class layout or save format.
class CEntity;
namespace Enhanced {
extern int8 QuickSave, AutoSave, RadarZoom, GPS, MenuMap;
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
void ProcessWindscreenShot(const CVector &source, const CVector &target, CEntity *shooter);
void AfterSuccessfulSave();
bool ValidateAutosave(char *destination);
void LoadAutosave(int8 action);
void OpenMap(int8 action);
wchar *MapOptionDraw(bool *disabled, bool hovering);
}
