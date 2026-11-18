# Enhanced roadmap — 18 November 2026

User feedback: `re3 notlar.txt` and Enhanced Options screenshot.
Status below means implementation, not a claim of in-game verification.

## Revision 2 — implemented, build/play-test pending

- [x] Normal Graphics menu font/scales, 20-pixel rows and footer clearance.
- [x] Remove Enhanced first person and ragdoll, including runtime hooks/module.
- [x] Autosave only on SCM REGISTER_MISSION_PASSED; wait for safe free-roam
  state, never on a timer or on loading a save. Keep isolated/checksummed file.
- [x] GPS: manual waypoint first, otherwise visible mission coordinate objectives,
  then entity objectives. Red route below all icons and player arrow.
- [x] World-map player arrow rotates with native north-up heading math.
- [x] Hold T for frame-time based smooth radar zoom, smaller blips and more tiles.
- [x] LOD selection in render callbacks; suppress low meshes and component culling.
- [x] GLFW borderless desktop fullscreen on current monitor; restore old mode.
- [x] Bullet-breakable windshield: visible-mesh triangle tests, native glass and
  damage. Instant-hit/shotgun/drive-by and simulated sniper bullets. Honor
  bulletproof/script protection. No ASI binary code or vehicles.col replacement.

## Required revision 2 play-tests

- [ ] Menu font/spacing/mouse/Back at 16:9 and 4:3.
- [ ] Mission passed autosave, failed mission, load autosave, normal slots intact.
- [ ] Multi-marker mission target choice, moving targets and waypoint override.
- [ ] Red line under all markers; player arrow north/east/south/west.
- [ ] Hold/release T at low/high FPS; shrinking blips and complete map coverage.
- [ ] LOD toggles without restart; original/custom models at long range.
- [ ] Borderless ON/OFF, restart, Alt-Tab, resolution changes, multiple monitors.
- [ ] Windscreen weapons/occlusion/protected vehicles/repair/save-load.
- [ ] x64 ASI and CLEO Redux startup with compatible scripts.

## Follow-up work

- [ ] Mission-specific selection when SCM displays multiple candidate objectives;
  nearest-marker selection is a heuristic, not full mission understanding.
- [ ] Refine glass fragment basis for unusual custom meshes. Reference mod's
  occupant-hit behavior, custom CLEO opcode and animation options are separate.
- [ ] First person/ragdoll stay withdrawn unless the user requests them again.
