# re3 Enhanced guide

The [README](README.md) lists features and installation steps. [v1.27](https://github.com/cnrgks/re3-enchanced/releases/tag/v1.27) is the Windows x64 OpenGL package. See the [roadmap](ROADMAP.md) for checks that still need in-game testing.

## Mod support

The OpenGL x64 package ships Ultimate ASI Loader 10.0.3 as `winmm.dll`
with its configuration, and CLEO Redux 1.5.1 as `cleo_redux64.asi` with
seven x64 CLEO plugins. Keep the matching `re3.pdb` next to `re3.exe` so
CLEO Redux can find re3 symbols. Put compatible ASIs in `scripts/` or
`plugins/`, JS/TS in `CLEO/` and FXT in `CLEO/CLEO_TEXT/`.

ASI plugins must target x64 and the re3 executable and ABI. Old GTA III
x86 ASIs and plugins using fixed `gta3.exe` addresses require porting.
Compiled CLEO scripts depend on the target game and opcodes used. Check
each mod individually and avoid duplicate quicksave or GPS mods.
For a clean x64 test, keep any legacy x86 CLEO installation in a separate
folder. Package integrity confirms the binaries are present and match
the architecture. Runtime initialization and script execution are
covered by the separate release verification list in [ROADMAP.md](ROADMAP.md).

[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader#usage) ·
[CLEO Redux re3 support](https://re.cleo.li/docs/en/troubleshooting.html) ·
[CLEO Redux script formats](https://re.cleo.li/docs/en/scripts.html)

## Windows OpenGL fullscreen

Graphics → Screen Format → Fullscreen presents the OpenGL frame through
exclusive DXGI fullscreen. If DXGI setup or presentation fails, the game
falls back to native OpenGL fullscreen and logs the failure in
`re3-display.log`. Borderless uses the OpenGL presentation path and its
existing width probe. To recover without opening the menu, launch with
`-safe-display`. To compare with DXGI presentation in a monitor-sized
borderless window, launch with `-dxgi-composed-fullscreen`.
