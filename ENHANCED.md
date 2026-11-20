# re3 Enhanced guide

The [README](README.md) lists features and installation steps. [v1.27](https://github.com/cnrgks/re3-enchanced/releases/tag/v1.27) is the Windows x64 OpenGL package. See the [roadmap](ROADMAP.md) for checks that still need in-game testing.

## Mod support

The base package runs without ASI Loader or CLEO Redux. To add mods, get the
x64 Ultimate ASI Loader and place its DLL beside `re3.exe` as `winmm.dll`.
The v1.27 release has an optional `winmm.ini` configuration ZIP. Install the
official CLEO Redux x64 portable package separately if a mod needs CLEO.
Keep the matching `re3.pdb` next to `re3.exe` for CLEO symbol lookup.

Keep the `re3.exe` name when enabling CLEO Redux. Renaming the executable
to `gta3.exe` can select the classic GTA III host and cause an early crash
in `Events64.cleo`. If you use `gta3.exe` for the Steam entry, keep CLEO
disabled. Overwriting an existing installation with the base ZIP leaves
previously installed mod loaders in place.

Put compatible ASIs in `scripts/` or `plugins/`, JS/TS in `CLEO/` and FXT
in `CLEO/CLEO_TEXT/`.

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
