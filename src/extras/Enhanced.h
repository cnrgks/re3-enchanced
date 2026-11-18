#pragma once

// Built-in features. These do not change any game's class layout or save format.
class CPed;
namespace Enhanced {
extern int8 QuickSave, AutoSave, FirstPerson, RadarZoom, GPS, MenuMap;
extern int8 Ragdoll, VehicleDetail, PedDetail, HealAfterSave;
extern const char *AutoSaveTimes[];
extern const char *RadarZoomModes[];

wchar *GetText(const char *key);
void Process();
void Reset();
void DrawRoute();
void WaypointChanged();
void DrawMapFallback();
bool UseFirstPerson();
void AfterSuccessfulSave();
bool ValidateAutosave(char *destination);
void LoadAutosave(int8 action);
void OpenMap(int8 action);
wchar *MapOptionDraw(bool *disabled, bool hovering);
void BeginPedRender(CPed *ped);
void EndPedRender(CPed *ped);
void ForgetPed(CPed *ped);
}
