#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "Hunting.h"

int framecount_out = 0;
int framecount_enter = 0;
int framecount_diff = 0;

extern ReplayMeta current_replay;

uint8_t rand_table[91] = { 0 };

void __declspec(naked) set_num_rand_calls_hunting()
{
    __asm
    {
        push 0x007380c5
        pushfd
        pushad
    }
    switch (DemoState)
    {
    case 0:
    {
        framecount_out = FrameCount;
        break;
    }
    case 2:
    {
        if (TimesRestartedOrDied == 0)
        {
            current_replay.framecount = FrameCount;
        }
        [[fallthrough]];
    }
    case 1:
    {
        if (TimesRestartedOrDied == 0)
        {
            for (int i = 0; i < rand_table[static_cast<unsigned short>(CurrentLevel)]; i++)
                sa2_rand();
            framecount_enter = FrameCount;
        }
        framecount_diff = FrameCount - framecount_enter;
        framecount_out = current_replay.framecount + framecount_diff;
        break;
    }
    default: break;
    }
    __asm
    {
        popad
        popfd
        mov edi, framecount_out
        ret
    }
}

void Hunting::Init() {
    rand_table[5] = 99;
    rand_table[7] = 101;
    rand_table[8] = 109;
    rand_table[16] = 103;
    rand_table[18] = 104;
    rand_table[25] = 102;
    rand_table[26] = 101;
    rand_table[32] = 101;
    rand_table[44] = 108;

    /***************************************************************************/
    /* Emerald stuff                                                           */
    /***************************************************************************/
    // Remove cmp/jump for if DemoState != 0
    WriteData<7>((void*)0x007380b0, (char)0x90); // NOP cmp [DemoState] 0x0
    WriteData<2>((void*)0x007380bd, (char)0x90); // NOP jnz 0x007380da

    WriteJump((void*)0x007380bf, set_num_rand_calls_hunting); // sets edi to the desired value

    WriteData<1>((void*)0x007380c4, (char)0x90); // NOP leftover byte
}