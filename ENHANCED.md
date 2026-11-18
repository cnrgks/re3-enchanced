# re3 Enhanced — revision 2

Based on Hezkore/hez-gta-re3 upstream 0145967e46f64c671aa22ed7406131cd3132f5b5.
Options: Options > Enhanced Options. Settings: [Enhanced] in re3.ini.
Native modules: src/extras/Enhanced.* and EnhancedVehicle.cpp; window integration
in src/skel/glfw/glfw.cpp. No fixed game addresses or class/save layout changes.

## Features

- F5 native save menu, in safe stationary free roam on foot, without wanted level.
- Autosave only after SCM mission-pass registration. Waits for the same safe state;
  no interval timer. GTA3sf-enhanced-auto1.b is isolated from eight normal slots.
  Temporary file is closed and checksum validated before atomic Windows replace.
  Load with confirmation from Enhanced Options. Loading never queues an autosave.
- Hold T for smooth radar zoom. Icons/traces shrink; extra tiles cover wide range.
- GPS: manual waypoint overrides mission routing. Otherwise prefer the nearest
  visible coordinate objective, then visible entity objective. Mission routes
  use the native objective blip's exact colour, including dim and literal RGBA
  colours; manual waypoints remain red. Lines draw below every icon.
  Read-only graph search respects disabled roads and native one-way lane data,
  considers nearby target entrances and preserves the entire route. Short
  endpoint connectors require clear building/object line of sight. Walking and
  driving use separate graphs and recalculate when entering/leaving a vehicle.
  Multi-goal mission selection remains a heuristic; no voice navigation.
- Native pause map with radar-tile fallback when menu-map artwork is unavailable.
- Detailed vehicle/ped models selected directly in render callbacks; low-detail
  variants suppressed. Native spawning, world visibility and streaming retained.
- Heal after successful manual save: at least 100, also serialized. Failed save
  restores previous health. Autosave and mission retry do not heal.
- Borderless fullscreen: current monitor desktop size via GLFW. Turning it off
  restores prior window/fullscreen mode and geometry. Persisted through native ini.
- Breakable windscreen: real bullet segment/render-mesh triangle intersections,
  native glass effects and panel damage. No supplied vehicles.col replacement.
  Protected vehicles and Rhino turret excluded. Reference mod's occupant hit-through,
  extra CLEO opcode, custom animations and all fragment options are not reproduced.

Enhanced first person and ragdoll have been removed at the user's request.
Their old ini keys are ignored. Old nonzero autosave/radar modes migrate to ON.

## Mod support and testing

Keep x64 winmm.dll ASI loader and x64 CLEO Redux. ASIs: scripts/ or plugins/;
JS/TS: CLEO/; FXT: CLEO/CLEO_TEXT/. Disable old quick_save.js for native F5.
Original GTA III x86 ASIs and CS scripts are not made compatible.
Keep EXE with its matching PDB. See ROADMAP.md for play-test status/limitations.
Compilation and PDB checks do not replace in-game verification.
