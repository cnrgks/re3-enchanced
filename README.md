<img src="res/images/logo_1024.png" alt="re3 logo" width="200">

## Intro

re3 Enhanced is a GTA III fork of [Hezkore's re3](https://github.com/Hezkore/hez-gta-re3), itself based on the original re3 project. This fork adds gameplay and quality-of-life options. The current release is **v1.31 for Windows x64 with OpenGL**.

## Hezkore's changes

* The game updates at 30 FPS and interpolates rendered frames. Animation smoothing also covers cutscenes and can be turned off.
* Motion blur is framerate independent and has Off, Faint and Strong settings. Limit FPS offers a list of framerates.
* Menus draw at the full framerate and avoid flashing the previous page during transitions.
* Mouse sensitivity has a finer range and uses consistent horizontal and vertical camera speeds.
* Wayland startup and left-click firing are supported.
* Visual defaults enable rim lights, lightmaps, road gloss, high map memory usage, maximum draw distance and VSync.
* Ultrawide displays keep the HUD, subtitles and loading screens within 16:9. Extend HUD moves the HUD to the screen edges.

## Enhanced changes

* **Saving.** F5 opens the manual save menu during safe free roam and saves your current location. Optional Mission Autosave uses the current island's safehouse exit and the original exit orientation. Its separate slot appears below the eight manual slots in Load Game. Loading an autosave or a new F5 save skips stale script help from the saved session while allowing new gameplay prompts.
* **Map and GPS.** Press M to open the map. Manual waypoints show a route, with optional routing to mission objectives. Shop discovery is remembered in the save and radar icons scale with zoom.
* **Weapon and radio wheels.** Enable these in Gameplay Setup. Hold Tab on foot or R in a vehicle, then release to select. Scrolling in a vehicle retains the classic radio display and station cycling. The radio wheel uses the game's station icons and normal retune delay and sounds.
* **Trainer.** F8 opens a keyboard-driven menu for vehicle spawning and other tools while the game continues.
* **Gameplay and graphics.** Optional intro skipping and expanded traffic and pedestrian draw distance and density. Screen Format offers Fullscreen, Windowed and Borderless. The Windows OpenGL path includes capture compatibility changes and a fix for losing input when switching away during loading.

## Installation

1. Start with an existing GTA III installation and download [re3 Enhanced v1.31 ZIP](https://github.com/cnrgks/re3-enchanced/releases/download/v1.31/re3-enhanced-v1.31.zip).
2. Back up your saves, settings and any existing re3 installation. Extract the ZIP directly into the GTA III directory.
3. Run `re3.exe` and keep the matching `re3.pdb` beside it. The archive includes the runtime libraries.

## Modding

The base game ZIP runs without mod loaders. Install these separately if a mod needs them.

* **Ultimate ASI Loader x64.** Rename `dinput8.dll` to `winmm.dll` and place it next to `re3.exe`. ASI plugins must target re3 x64. [Official download](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases)
* **CLEO Redux x64.** Extract the portable package beside `re3.exe` with ASI Loader installed. Keep the matching `re3.pdb` alongside it. If you run the game as `gta3.exe` from Steam, leave CLEO disabled to avoid the startup crash in `Events64.cleo`. [Official download](https://github.com/cleolibrary/CLEO-Redux/releases)

See the [mod support guide](ENHANCED.md#mod-support) for script formats and mod folders.

The [roadmap](ROADMAP.md) and [bug tracker](docs/BUGS.md) cover current work and gameplay verification.

See [workspace recovery](docs/WORKSPACE-RECOVERY.md) for the maintained GitHub Actions and packaging setup.

## Building from Source  

When using premake, you may want to point GTA_III_RE_DIR environment variable to GTA3 root folder if you want the executable to be moved there via post-build script.

Clone the repository with `git clone --recursive https://github.com/cnrgks/re3-enchanced.git`. Then `cd re3-enchanced` into the cloned repository.

<details><summary>Linux Premake</summary>

You need the development files for glfw, OpenAL, libsndfile, mpg123 and OpenGL. On Debian and Ubuntu that is:
```
sudo apt install libglfw3-dev libopenal-dev libsndfile1-dev libmpg123-dev libgl1-mesa-dev
```
Then generate the makefiles and build:
```
./premake5Linux --with-librw gmake2
make -C build config=release_linux-amd64-librw_gl3_glfw-oal
```
The binary ends up in `bin/linux-amd64-librw_gl3_glfw-oal/Release`. Swap `amd64` for `x86`, `arm` or `arm64`, and `release` for `debug`.

Reading the keyboard from the X server needs glfw 3.4 or newer. On anything older the build stops with an error, add `--no-x11-keyboard` to premake and GLFW handles the keys instead.

</details>

<details><summary>Linux Conan</summary>

Install python and conan, and then run build.
```
conan export vendor/librw librw/master@
mkdir build
cd build
conan install .. re3/master@ -if build -o re3:audio=openal -o librw:platform=gl3 -o librw:gl3_gfxlib=glfw --build missing -s re3:build_type=RelWithDebInfo -s librw:build_type=RelWithDebInfo
conan build .. -if build -bf build -pf package
```
</details>

<details><summary>MacOS Premake</summary>

The old instructions went with the upstream wiki. premake generates the projects the same way it does on Linux, though this has not been built on a Mac in a long time.

</details>

<details><summary>FreeBSD</summary>

The old instructions went with the upstream wiki. premake generates the projects the same way it does on Linux, though this has not been built on FreeBSD in a long time.

</details>

<details><summary>Windows</summary>

Assuming you have Visual Studio 2015/2017/2019:
- Run one of the `premake-vsXXXX.cmd` variants on root folder.
- Open build/re3.sln with Visual Studio and compile the solution.

Visual Studio 2022 builds the vs2019 solution as long as the v142 toolset is installed, which is how the nightlies are built.

Microsoft recently discontinued its downloads of the DX9 SDK. You can download an archived version here: https://archive.org/details/dxsdk_jun10

**If you choose OpenAL on Windows** the exe needs `OpenAL32.dll` and `libmpg123-0.dll` next to it. Both ship with the repository, in `vendor/openal-soft/dist/Win64` and `vendor/mpg123/dist/Win64`.
</details>

> :information_source: premake has an `--with-lto` option if you want the project to be compiled with Link Time Optimization.

> :information_source: There are various settings in [config.h](src/core/config.h), you may want to take a look there.

> :information_source: re3 uses completely homebrew RenderWare-replacement rendering engine; [librw](https://github.com/aap/librw/). librw comes as submodule of re3, but you also can use LIBRW enviorenment variable to specify path to your own librw.

If you feel the need, you can also use CodeWarrior 7 to compile re3 using the supplied codewarrior/re3.mcp project - this requires the original RW33 libraries, and the DX8 SDK. The build is unstable compared to the MSVC builds though, and is mostly meant to serve as a reference.

## Contributing

See the [coding style](CODING_STYLE.md), [roadmap](ROADMAP.md) and [bug tracker](docs/BUGS.md). Report the release version, display mode and steps to reproduce when filing a bug.

## History

re3 was started sometime in the spring of 2018,
initially as a way to test reversed collision and physics code
inside the game.
This was done by replacing single functions of the game
with their reversed counterparts using a dll.

After a bit of work the project lay dormant for about a year
and was picked up again and pushed to github in May 2019.
At the time I (aap) had reversed around 10k lines of code and estimated
the final game to have around 200-250k.
Others quickly joined the effort (Fire_Head, shfil, erorcun and Nick007J
in time order, and Serge a bit later) and we made very quick progress
throughout the summer of 2019
after which the pace slowed down a bit.

Due to everyone staying home during the start of the Corona pandemic
everybody had a lot of time to work on re3 again and
we finally got a standalone exe in April 2020 (around 180k lines by then).

After the initial excitement and fixing and polishing the code further,
reVC was started in early May 2020 by starting from re3 code,
not by starting from scratch replacing functions with a dll.
After a few months of mostly steady progress we considered reVC
finished in December.

Since then we have started reLCS, which is currently work in progress.


## License

We don't feel like we're in a position to give this code a license.\
The code should only be used for educational, documentation and modding purposes.\
We do not encourage piracy or commercial use.\
Please keep derivate work open source and give proper credit.
