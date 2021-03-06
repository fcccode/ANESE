<p align="center">
  <a href="https://prilik.com/ANESE">
    <img height="128px" src="resources/icons/anese.ico" alt="ANESE Logo">
  </a>
</p>
<p align="center">
  <a href="https://ci.appveyor.com/project/daniel5151/anese">
    <img src="https://ci.appveyor.com/api/projects/status/qgy19m8us3ss6ilt?svg=true" alt="Build Status Windows">
  </a>
  <a href="https://travis-ci.org/daniel5151/ANESE">
    <img src="https://travis-ci.org/daniel5151/ANESE.svg?branch=master" alt="Build Status macOS/Linux">
  </a>
</p>

**ANESE** (**A**nother **NES** **E**mulator) is a Nintendo Entertainment System
Emulator being written for fun and learning.

While accuracy is a long-term goal, ANESE's primary goal is to play some of the
more popular titles. As of now, most basic Mappers have been implemented, so
popular titles should be working! :smile:

ANESE is built with _cross-platform_ in mind, and is regularly built on all
major platforms (macOS, Windows, and Linux). ANESE doesn't use any
vendor-specific language extensions, and is compiled with strict compiler flags.
It is also linted (fairly) regularly.

ANESE strives for clean and _interesting_ C++11 code, with an emphasis on
readability and maintainability. With that said, performance is important, but
it's not ANESE's primary focus.

## Downloads

Right now, there are no official releases of ANESE, but there are ways to get
nightly releases.

**Windows:** You can download nightly versions of ANESE from
[AppVeyor](https://ci.appveyor.com/project/daniel5151/anese)'s build artifacts
page.

**macOS:** Nightly ANESE.app bundles are uploaded from Travis to
[this GDrive folder](https://drive.google.com/drive/folders/1GWiinQ4tjDSZlhjReVMdElwK1b-Zvagf).

## Building

### Dependencies

While ANESE's emulation core (src/nes) doesn't have any major dependencies,
there are a couple of libraries the UI uses. Most of these dependencies are
bundled with ANESE (see: /thirdparty), although some do require additional
installation:

- **SDL2** (video/audio/controls)
  - _Linux_: `apt-get install libsdl2-dev` (on Ubuntu)
  - _MacOS_: `brew install SDL2`
  - _Windows_:
    - Download dev libs from [here](https://www.libsdl.org/download-2.0.php) and
      unzip them somewhere.
    - EITHER: Set the `SDL` environment variable to point to the dev libs
    - OR: Unzip them to `C:\sdl2\` (Where I put them)
    - OR: Modify the `SDL2_MORE_INCLUDE_DIR` variable in `CMakeLists.txt` to
      point to the SDL2 dev libs

### Generating + Compiling

ANESE builds with **CMake**. Nothing too fancy here.

```bash
# in ANESE root
mkdir build
cd build
cmake ..
make
make install # on macOS: creates ANESE.app in ANESE/bin/
```

On Windows, building is very similar:
```bat
mkdir build
cd build
cmake ..
msbuild anese.sln /p:Configuration=Release
```

If you're interested in looking under the hood of the PPU, you can pass the
`-DDEBUG_PPU` flag to cmake and have ANESE show PPU debug windows.

## Running

ANESE can run from the shell using `anese [rom.nes]` syntax.

If no ROM is provided, a simple dialog window pops-up prompting the user to
select a valid NES rom.

For a full list of switches, run `anese -h`

**Windows Users:** make sure the executable can find `SDL2.dll`! Download the
runtime DLLs from the SDL website, and plop them in the same directory as
anese.exe

## Mappers

There aren't too many mappers implemented at the moment, but the ones that are
cover a sizable chunk of the popular NES library.

 \#  | Name  | Some Games
-----|-------|--------------------------------------------------
 000 | NROM  | Super Mario Bros. 1, Donkey Kong, Duck Hunt
 001 | MMC1  | Legend of Zelda, Dr. Mario, Metroid
 002 | UxROM | Megaman, Contra, Castlevania
 003 | CNROM | Arkanoid, Cybernoid, Solomon's Key
 004 | MMC3  | Super Mario Bros 2 & 3, Kirby's Adventure
 007 | AxROM | Marble Madness, Battletoads
 009 | MMC2  | Punch Out!!

If a game you love doesn't work in ANESE, feel free to implement it's mapper
and open a PR for it :D

## Controls

Currently hard-coded to the following:

Button | Key         | Controller
-------|-------------|------------
A      | Z           | X
B      | X           | A
Start  | Enter       | Start
Select | Right Shift | Select
Up     | Up arrow    | D-Pad
Down   | Down arrow  | D-Pad
Left   | Left arrow  | D-Pad
Right  | Right arrow | D-Pad

Any xbox-compatible controller should work.

There are also a couple of emulator actions:

Action             | Keys
-------------------|--------
Exit               | Esc
Reset              | Ctrl - R
Power Cycle        | Ctrl - P
Toggle CPU logging | Ctrl - C
Speed++            | Ctrl - =
Speed--            | Ctrl - -
Fast-Forward       | Space
Make Save-State    | Ctrl - (1-4)
Load Save-State    | Ctrl - Shift - (1-4)

(there are 4 save-state slots)

## DISCLAIMERS

- The CPU is _instruction-cycle_ accurate, but not _sub-instruction cycle_
accurate. While this inaccuracy doesn't affect most games, there are some that
that rely on sub-instruction level timings (eg: Solomon's Key).
  - The `--alt-nmi-timing` flag might fix some of these games.
- The APU is _not my code_. I wanted to get ANESE at least partially up and
running before new-years 2018, so I'm using Blargg's venerable `nes_snd_emu`
library to handle sound (for now). Once I polish up some of the other aspects
of the emulator, I'll try to revisit my own APU implementation (which is
currently a stub)

## TODO

This is a rough list of things I would like to accomplish, with those closer to
the top higher on my priority list:

- [ ] _Implement_: My own APU (don't use Blarrg's)
- [ ] _Refactor_: Push common mapper behavior to Base Mapper (eg: bank chunking)
- [x] _Refactor_: Modularize `main.cc` - push everything into `src/ui/`
- [ ] _CMake_: Make building macOS bundles less brittle
- [ ] _Implement_: LibRetro Core
- [ ] _Implement_: Sub-instruction cycle accurate CPU
- [ ] _Cleanup_: Unify naming conventions (either camelCase or snake_case)
- [ ] _Cleanup_: Comment the codebase _even more_
- [ ] _Security_: Actually bounds-check files lol
- [ ] _Cleanup_: Conform to the `.fm2` movie format better
- [ ] _Cleanup_: Remove fatal asserts (?)
- [ ] _Cleanup_: Switch to a better logging system
               (\*cough\* not fprintf \*cough\*)

## Roadmap

### Key Milestones

- [x] Parse iNES files
- [x] Create Cartridges (iNES + Mapper interface)
- [x] CPU
  - [x] Set Up Memory Map
  - [x] Hardware Structures (registers)
  - [x] Core Loop / Basic Functionality
    - [x] Read / Write RAM
    - [x] Addressing Modes
    - [x] Fetch - Decode - Execute
  - [x] Official Opcodes Implemented
  - [x] Handle Interrupts
- [x] PPU
  - [x] Set Up Basic Rendering Context (SDL)
  - [x] Implement Registers + Memory Map them
  - [x] Implement DMA
  - [x] Generate NMI -> CPU
  - [x] Core rendering loop
    - [x] Background Rendering
    - [x] Sprite Rendering - _currently not hardware accurate_
    - [x] Proper Background / Foreground blending
  - [x] Sprite Zero Hit
  - [ ] Misc PPU flags (emphasize RGB, Greyscale, etc...)
- [ ] APU - ***Uses `nes_snd_emu` by Blargg - crashes sometimes***
  - [x] Implement Registers + Memory Map them
  - [ ] Frame Timer IRQ
  - [ ] Set Up Basic Sound Output Context (SDL)
  - [ ] Channels
    - [ ] PCM 1
    - [ ] PCM 2
    - [ ] Triangle
    - [ ] Noise
    - [ ] DMC
  - [ ] DMC DMA
- [ ] Joypads
  - [x] Basic Controller
  - [ ] Zapper
  - [ ] NES Four Score

### Secondary Milestones

- [x] Loading Files with picker
- [x] Reset / Power-cycle
- [x] Fast Forward
- [ ] Run / Pause / Step
- Saving
  - [x] Battery Backed RAM - Saves to `.sav`
  - [x] Save-states
    - [ ] Dump to file
- [ ] Config File
  - [ ] Remap controls
- [x] Running NESTEST (behind a flag)
- [x] Controller support - _currently very basic_
- [ ] A proper GUI
  - imgui maybe?

### Tertiary Milestones (Fun Features!)

- [x] Zipped ROM support
- [ ] Rewind
- [ ] Game Genie
- [x] Movie recording and playback
- [ ] More ROM formats (not just iNES)
- [ ] Proper PAL handling?
- [ ] Proper NTSC artifacting?
- Multiple Front-ends
  - [x] SDL Standalone
  - [ ] LibRetro
- [ ] Debugger!
  - There is a lot of great infrastructure in place that could make ANESE a
    top-tier NES debugger, primarily the fact that all memory-interfaced
    objects _must_ implement `peek`, which enables non-destructive looks at
    arbitrary objects!
  - [ ] CPU
    - [x] Step through instructions - _super jank, no external flags_
  - [ ] PPU Views
    - [x] Static Palette
    - [x] Palette Memory
    - [x] Pattern Tables
    - [x] Nametables
    - [ ] OAM memory

### Accuracy & Compatibility

- More Mappers!
- CPU
  - [ ] Implement Unofficial Opcodes
  - [ ] Pass More Tests yo
  - [ ] _\(Stretch\)_ Switch to sub-instruction level cycle-based emulation
        (vs instruction level)
- PPU
  - [x] Make the sprite rendering pipeline more accurate (fetch-timings)
    - This _should_ fix _Punch Out!!_ **UPDATE:** it totally did.
  - [ ] Pass More Tests yo
  - [ ] Make value in PPU <-> CPU bus decay

## Attributions

- A big shout-out to [LaiNES](https://github.com/AndreaOrru/LaiNES) and
[fogleman/nes](https://github.com/fogleman/nes), two solid NES emulators that I
referenced while implementing some particularly tricky parts of the PPU). While
I actively avoided looking at the source codes of other NES emulators
as I set about writing my initial implementations of the CPU and PPU, I did
sneak a peek at how others did some things when I got very stuck.
- Blargg's [nes_snd_emu](http://www.slack.net/~ant/libs/audio.html) makes ANESE
  sound as good as it does :smile:
- These awesome libraries make ANESE a lot nicer to use:
  - [sdl2](https://www.libsdl.org/)
  - [args](https://github.com/Taywee/args)
  - [miniz](https://github.com/richgel999/miniz)
  - [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
