# re3 Enhanced — first integration build

Based on Hezkore/hez-gta-re3, upstream commit
`0145967e46f64c671aa22ed7406131cd3132f5b5`. Native additions are in
`src/extras/Enhanced.*` and `src/extras/EnhancedRagdoll.cpp`.

Open **Options > Enhanced Options**. Settings are stored in the existing
`re3.ini`, under `[Enhanced]`. Gameplay additions default to off; the existing
pause map remains enabled. No original game addresses or binary ASI code are
used by these native additions, and no class layout/save-format changes are made.

| Setting | First implementation |
| --- | --- |
| F5 save menu | Native save-slot selection. Requires a stationary player on foot, no mission, wanted level, cutscene, remote control, replay or transition. |
| Autosave | 5/10/15 minutes of active gameplay; waits for the same safe conditions. Also queues a save after a mission-pass count increase. Writes `GTA3sf-enhanced-auto1.b` next to normal saves, via a temporary file and atomic Windows replacement. Never overwrites the eight manual slots. |
| Load autosave | Uses the native confirmation/load path after native checksum validation. |
| First person | Enable the feature, then toggle with F6. Basic on-foot runabout camera; scripted cameras, vehicles and special weapon camera modes retain native behavior. No custom arm/weapon animation pack. |
| Radar zoom | Hold TAB for a wider range; optional persistent wider range. |
| GPS route | Set a waypoint with right-click on the pause map. Uses native pedestrian/car path nodes, refreshes as the player moves, and draws on radar/map. No voice guidance or guarantee of legal one-way driving directions; disconnected path groups produce no route. |
| Pause map | Existing native pan/zoom/waypoint controls. If menu-map art is absent, draws the radar tiles. |
| Ragdoll (basic) | Experimental articulated visual solver for original non-skinned PC ped models during death/fall. Bounded to 32 nearby bodies, gravity, distance constraints and static-environment collision. Native entity/body collision remains in charge. Skinned models retain native animation. This is not full rigid-body ragdoll, joint-angle limits, swimming, vehicle-body interactions, or feature parity with the reference ASI. |
| High detail vehicles/peds | Keep detailed meshes within native visibility distances. Does not extend spawning or streaming distance. |
| Heal after manual save | Restores health to at least 100 and serializes that health on a successful manual save. Reverts health if saving fails. Autosave and mission-retry saves do not heal. |

## Mod folders

The distribution keeps x64 Ultimate ASI Loader (`winmm.dll`), x64 CLEO Redux and
`CLEO/CLEO_PLUGINS`. Additional compatible x64 ASIs can go in `scripts/`.
JavaScript/TypeScript scripts go in `CLEO/` and text in `CLEO/CLEO_TEXT/`.
Old GTA III x86 ASIs and `.cs` scripts are not made compatible by this build.
Disable the old `quick_save.js` when testing native F5 saving so it cannot bypass
the Enhanced setting or duplicate input handling. Reference ASIs are not bundled.

## Validation

The workflow `.github/workflows/enhanced.yml` builds x64 Release with full PDB.
EXE and PDB must be distributed as one matching pair. A successful compilation
does not establish in-game correctness. Validate the following on a backup game
installation/save directory before using the build for normal play:

1. Options page and toggles persist after exit/restart; all-off keeps native play.
2. F5 opens once, and is rejected during missions/cutscenes/vehicles/wanted state.
3. Autosave writes/loads its isolated file; normal saves are untouched. Failed
   writes keep the old autosave. New/load game resets pending timers/state.
4. Health is restored only for successful manual saves, including after reload.
5. F6 on foot, enter/exit vehicles, aim special weapons, cutscenes and death.
6. Map pan/zoom/waypoint; GPS on foot/in a car, arrival and disconnected islands.
7. Ragdoll death/fall, off/on, pedestrian deletion, restart, original/skinned models.
8. LOD toggles, frame rate and unchanged native draw/streaming boundaries.
9. `winmm.log` and `cleo_redux.log` still show the x64 runtime loading.

Reference mods supplied by the user are behavioral references only; their binary
code and custom animation assets have not been embedded. Source modifications
are built directly against re3's own APIs.
