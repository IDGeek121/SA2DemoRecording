# SA2 Demo Recording

__Replay metafile structure is subject to change, though it should be trivial to convert to a new format without affecting the replay contents.__

__Carts do not work. Chao is not supported. Cannons Core is not tested, but probably doesn't work.__

This is a mod which hooks into the existing code used for the attract mode demos to create and play back your own custom demos.

## Install
1. Install the [SA2 Mod Loader](https://gamebanana.com/tools/6333).
1. Download the [latest release](https://github.com/IDGeek121/SA2DemoRecording/releases).
1. Extract SA2DemoRecording.zip into `$SA2_DIR\mods\`.
1. Create the folder `$SA2_DIR\resource\gd_PC\Demos\`. This is where your demos will be saved to.
1. Open `SA2ModManager.exe`
    1. Under the "Mods" tab, check the "Demo Recording" mod to enable it.

## Usage
You can create/play back replays from the stage select screen.

### Record
1. Hold the X button while confirming the mission to set the game to record.

Once the level is completed, a replay file (struct compatible with the attract mode demos) is saved to `$SA2_DIR\resource\gd_PC\Demos\`. Along with the .bin file, an associated .ini file containing meta information about the replay is saved as well. This contains information such as the character, level, the frame used for RNG seeding, and the upgrades.

### Playback
1. Select a stage for which you have a replay.
2. Then, hold the Y button while confirming a mission.
    - __It will throw an error and crash the game if you don't have any replays for the stage you've selected. This will be fixed at a later date__.
3. Scroll up and down through all available replays, and press A to select a replay.

## Build
1. Install VS2019
1. Open [SA2DemoRecording.sln](https://github.com/IDGeek121/SA2DemoRecording/blob/main/SA2DemoRecording.sln)
1. Build for x86

You will need to create a folder in `$SA2_DIR\mods\` for the DLL and create the `mod.ini`. You can steal it from the [latest release](https://github.com/IDGeek121/SA2DemoRecording/releases) for now.