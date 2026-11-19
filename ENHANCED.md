# re3 Enhanced v1.4 — cursor fix and experimental fullscreen presentation

> Known v1.1 issues and the latest requested save/map behavior are tracked in
> [docs/BUGS.md](docs/BUGS.md). Future features: [ROADMAP.md](ROADMAP.md).
> The description below records the released implementation, not confirmation
> that every feature passes gameplay testing. Builds/tests now require the user's explicit request.

Hezkore/hez-gta-re3, upstream 0145967e46f64c671aa22ed7406131cd3132f5b5.
Native C++ features; no fixed EXE addresses or game class/save layout changes.
Settings: Options > Enhanced Options, persisted under [Enhanced] in re3.ini.

- F5 opens the native save-slot menu in safe free roam: on foot, idle, alive,
  no mission, wanted level, replay, cutscene or fade. The mission-pass option
  queues the same menu until this safe state. It does not write automatically.
- Enhanced saves serialize the current island's safehouse entrance and ground
  height for the player and initial camera streaming. The live player is not
  teleported. Native safehouse saves and mission retry keep their own flow.
  Load Autosave is removed. Existing isolated autosave files are not deleted.
- Heal after manual save persists at least 100 health; failed save restores it.
- Radar icons, traces and rotating player arrow share range-dependent scaling,
  including the native speed zoom. T smoothly targets 2.25x instead of 3x.
- Pause-map service icons come from typed SCM metadata, native garages, shop
  weapon pickups and runtime blips, including services cleared by distance.
- Map keeps its two crosshair lines, hides the cursor sprite, removes footer
  control instructions and aligns Back with the blue Rockstar logo's left edge.
- Scrolling an option page moves its viewport by three rows, without cycling
  the focused option or changing its value. Keyboard navigation remains native.
- Vehicle/Pedestrian Distance: 1x, 1.5x, 2x or 3x. Spawn/despawn thresholds and
  render LOD/visibility ranges scale together; density grows modestly. Native
  pool limits remain in force, so this is not an unlimited-population setting.
- High Detail Vehicles/Pedestrians selects native detailed render meshes.
- Windscreen shots test actual mesh triangles, nested component frames and named
  fallback frames. Coarse car collision no longer prevents a pane hit. Broken
  panes still define the aperture. Seated driver/passenger hit models feed the
  ordinary weapon damage flow; bulletproof/script protections stay in effect.
  A custom car with glass merged into an unrelated body mesh is not inferred.
- GPS retains v1.1's preceding GPS update: mission lines match actual blip RGBA,
  including dim colours; manual waypoints stay red. Disabled roads and native
  one-way lanes are respected; complete routes and alternate endpoints used.
  Mission target selection remains a nearest-visible-objective heuristic.

## Display backend for this release

This v1.3 diagnostic candidate builds OpenGL only. Graphics > Screen Format has Fullscreen,
Windowed and Borderless. Changes are applied with Enter from the main menu
before loading a game. Enhanced Options no longer contains a duplicate Borderless
selector on the GLFW backend. [VideoMode] ScreenFormat (0/1/2) is authoritative;
legacy Enhanced/Borderless and VideoMode/Windowed are migrated only when absent.

Borderless selects a non-exclusive RW video mode before creating the GL window.
This avoids the old exclusive-first, windowed-later startup transition. Capture
compatibility is not yet verified. No driver settings are modified. The D3D9
backend remains in source but is not built or bundled for this user-requested run.
On Windows only, Borderless requests monitor width + 1 (height unchanged).
[VideoMode] BorderlessCaptureProbe defaults to 1; setting 0 and restarting
allows the exact-width comparison in the same EXE. Windowed/exclusive modes
are untouched by this probe. A column may extend onto an adjacent monitor.
re3-display.log records window/framebuffer size and monitor attachment once
per startup/mode change. No per-frame logging or driver changes.
See docs/OPENGL-CAPTURE.md; other v1.1 issues in docs/BUGS.md remain open.

## Mod support

The package includes the existing x64 Ultimate ASI Loader (winmm.dll), CLEO Redux
and CLEO plugins. ASIs: scripts/ or plugins/. JS/TS: CLEO/. FXT: CLEO/CLEO_TEXT/.
Original GTA III x86 ASIs and legacy CS scripts are not generally compatible.
Disable duplicate quicksave/GPS/windscreen reference mods when using native ones.
No proprietary GTA III game data or reference mod binaries are incorporated.

## Validation

GitHub Actions builds both x64 backends and runs geometry, radar scaling, graph
routing and SCM operand-decoder tests. Release packaging checks matching source
commits, EXE/PDB GUID+age, x64 runtime files and SHA-256 manifest entries.
Gameplay, save/load, screenshot layout, capture and vehicle-specific shots need
in-game verification. The archive is a complete mod distribution, not a full
licensed GTA III installation.

## Windows OpenGL fullscreen

Graphics → Screen Format → Fullscreen presents the OpenGL frame through DXGI
in a borderless window that covers the monitor. If DXGI setup or presentation
fails, the game continues with native OpenGL in the same window and records
the failure in `re3-display.log`. Borderless uses the OpenGL presentation
path and its existing width probe. To recover without opening the menu, launch
with `-safe-display`. The previous exclusive DXGI path is available for
diagnosis with `-dxgi-exclusive-fullscreen`.
