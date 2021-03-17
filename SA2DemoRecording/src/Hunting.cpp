#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "Hunting.h"

int framecount_out = 0;
int framecount_enter = 0;
int framecount_diff = 0;

extern ReplayMeta current_replay;

uint8_t rand_table[91] = { 0 }; // Stores the number of rand calls before emerald generation to get the expected 1024 set

void __declspec(naked) set_num_rand_calls_hunting() {
    __asm {
        push 0x007380c5
        pushfd
        pushad
    }
    switch (CurrentDemoState) {
    case DemoState_None: {
        framecount_out = FrameCount;
        break;
    }
    case DemoState_Recording: {
        if (TimesRestartedOrDied == 0)
            current_replay.framecount = FrameCount;
        [[fallthrough]];
    }
    case DemoState_Playback: {
        if (TimesRestartedOrDied == 0) {
            uint8_t num_rand_calls = rand_table[static_cast<unsigned short>(CurrentLevel)];
            for (int i = 0; i < num_rand_calls; i++)
                sa2_rand();
            framecount_enter = FrameCount;
        }
        framecount_diff = FrameCount - framecount_enter;
        framecount_out = current_replay.framecount + framecount_diff;
        break;
    }
    default: break;
    }
    __asm {
        popad
        popfd
        mov edi, framecount_out
        ret
    }
}

void Hunting::Init() {
    rand_table[LevelIDs_PumpkinHill] = 99;
    rand_table[LevelIDs_AquaticMine] = 101;
    rand_table[LevelIDs_SecurityHall] = 109;
    rand_table[LevelIDs_WildCanyon] = 103;
    rand_table[LevelIDs_DryLagoon] = 104;
    rand_table[LevelIDs_DeathChamber] = 102;
    rand_table[LevelIDs_EggQuarters] = 101;
    rand_table[LevelIDs_MeteorHerd] = 101;
    rand_table[LevelIDs_MadSpace] = 108;

    // The following writes/jumps hook right before the rand calls for emerald generation
    // Remove cmp/jump for if CurrentDemoState != 0
    WriteData<7>(reinterpret_cast<void*>(0x007380b0), unsigned char{ 0x90 }); // NOP cmp [CurrentDemoState] 0x0
    WriteData<2>(reinterpret_cast<void*>(0x007380bd), unsigned char{ 0x90 }); // NOP jnz 0x007380da

    WriteJump(reinterpret_cast<void*>(0x007380bf), set_num_rand_calls_hunting); // sets edi to the desired value

    WriteData<1>(reinterpret_cast<void*>(0x007380c4), unsigned char{ 0x90 }); // NOP leftover byte
}