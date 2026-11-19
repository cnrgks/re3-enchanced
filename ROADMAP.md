# re3 Enhanced v1.1 validation

Implementation and runtime verification are recorded separately.

## Implemented

- Pause-map services: typed script metadata, garages, weapon shop pickups/cache.
- Native speed and T radar zoom icon scaling including rotating player marker.
- T target 2.25x (75% of the former 3x target).
- Nested/named windscreen geometry and native seated-occupant bullet hits.
- Remove Load Autosave; mission success queues save-slot menu, no silent write.
- Enhanced save serialization uses the current island safehouse, no live teleport.
- D3D9 non-exclusive borderless desktop presentation; OpenGL fallback provided.
- Vehicle/ped spawn, removal and render distance settings.
- Map crosshair without cursor sprite, clean footer, Back aligned above blue logo.
- Wheel scrolls option viewport independently of selection/value.
- Mission GPS colours, one-way/closed-road routing and complete long paths.
- Complete versioned package with existing x64 ASI Loader/CLEO Redux support.

## Required in-game validation

- [ ] Map shows Pay 'n' Spray, Ammu-Nation, bomb shops and savehouses on all islands.
- [ ] Icons/player scale together when accelerating, holding T and releasing T.
- [ ] Map footer/Back/crosshair at 16:9 and 4:3; viewport scroll and keyboard input.
- [ ] Mission success opens menu once; cancellation does not write a file.
- [ ] F5 and mission saves load at each island's home without moving live player.
- [ ] Ground height at homes; eight slots and mission retry; heal success/failure.
- [ ] Front windscreen/driver shots across original/custom cars, after break/repair,
      occluding walls/vehicles and protected cars. Merged custom glass unsupported.
- [ ] Distance choices reduce pop-in during fast driving within native pool limits.
- [ ] D3D9 startup, main-menu Borderless toggle, Alt-Tab, resize/multiple monitors.
- [ ] Discord whole-desktop stream and Ambilight on user's actual machine.
- [ ] Mission colour variants/moving targets, closed bridges, one-way turns and
      cross-island routing; manual waypoint override, on-foot/car graph changes.
- [ ] x64 ASI and CLEO Redux startup with the new matching PDB on both backends.

First person and ragdoll remain withdrawn as requested in the previous revision.
Full mission-specific multi-goal understanding is still outside GPS scope.
