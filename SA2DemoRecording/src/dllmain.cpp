#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "StageSelectMode.h"

static_assert (sizeof(DemoInput) == 0x2c, "Size is not correct");

#define PROLOG_EPILOG(x) __asm \
{                              \
    __asm pushfd               \
    __asm pushad               \
    __asm call x               \
    __asm popad                \
    __asm popfd                \
    __asm ret                  \
}

int nextDemoState = 0;

extern char replayName[256];
char* replayNamePointer = (char*)&replayName;
char replayNameInGame[256] = { 0 };
char* replayNameInGamePointer = (char*)&replayNameInGame;

extern ReplayMeta currentReplay;

void* ADDR_004421c3 = reinterpret_cast<void*>(0x004421c3);
void* ADDR_00454594 = reinterpret_cast<void*>(0x00454594);
void* ADDR_0045459e = reinterpret_cast<void*>(0x0045459e);
void* ADDR_004545af = reinterpret_cast<void*>(0x004545af);

uint32_t replay_offset;
bool isLoadingCustomDemo;

const char* customDemoString = "Demos/%s.bin";

struct PolygonPoint {
    NJS_POINT3 pos;
    NJS_COLOR color;
};

PolygonPoint coolSquare[4] = { 0 };

void __declspec(naked dllexport) menu_stage_select_case6_thunk()
{
    __asm push 0x00672ae1
    PROLOG_EPILOG(menu_stage_select_case6)
}

void __declspec(naked dllexport) menu_stage_select_case7_thunk()
{
    __asm push 0x00672ae1
    PROLOG_EPILOG(menu_stage_select_case7)
}


void write_replay_buffer_thunk()
{
    if (DemoState == 2)
    {
        constexpr NJS_COLOR text_color = { 0xffffffff };
        DrawCustomText("Writing replay file", 320.f, 240.f, 1, &text_color);

        std::ostringstream file_name;
        const std::time_t current_time = std::time(nullptr);
        std::tm test_time;
        gmtime_s(&test_time, &current_time);

        std::ios state(nullptr);
        state.copyfmt(std::cout);
        file_name << "demo" << std::setfill('0') <<  std::setw(2) << CurrentLevel << "_" << std::put_time(&test_time, "%F_%H-%M-%S");
        std::cout.copyfmt(state);
        const std::string demo_folder = "resource/gd_PC/Demos/";
        const std::string meta_name = demo_folder + file_name.str() + ".ini";
        const std::string replay_name = demo_folder + file_name.str() + ".bin";
        PrintDebug("Writing demo file to %s", replay_name.c_str());
        WriteDemoBufferToFile(replay_name.c_str());
        PrintDebug("Writing demo metafile to %s", meta_name.c_str());
        ReplayMeta::write_replay_metafile("test", "test", currentReplay.upgradeBitfield, CurrentCharacter, CurrentLevel, currentReplay.framecount, file_name.str().c_str(), meta_name.c_str());
        DemoState = 0;
    }
}

void update_reset_replay_offset()
{
    if (GameState == 3)
    {
        replay_offset = 0;
    }
    else
    {
        replay_offset += FrameCountIngame;
    }
    FrameCountIngame = 0;
}

void update_replay_offset()
{
    replay_offset += FrameCountIngame;
    FrameCountIngame = 0;
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away()
{
    __asm push 0x0044caf2 // return address
    PROLOG_EPILOG(update_reset_replay_offset)
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away2()
{
    __asm push 0x0043d837 // return address
    PROLOG_EPILOG(update_replay_offset)
}

void __declspec(naked) go_to_next_level_thunk()
{
    __asm push 0x0043d5d0 // return address
    PROLOG_EPILOG(write_replay_buffer_thunk)
}

int __declspec(naked) add_replay_offset()
{
    __asm
    {
        mov eax, dword ptr [FrameCountIngame]
        mov eax, [eax]
        add eax, [replay_offset]
        ret
    }
}

void __declspec(naked) buffer_with_replay_offset()
{
    __asm
    {
        call add_replay_offset
        jmp ADDR_004421c3
    }
}

void __cdecl byteswap_replay_buffer_wrapper()
{
    const int end_of_valid_buffer = FrameCountIngame;
    DemoInput* buffer = reinterpret_cast<DemoInput*>(&DemoBuffer);
    for (int i = 0; i < end_of_valid_buffer; i++)
    {
        ByteswapDemoInput(buffer);
        buffer++;
    }
}

void __declspec(naked) write_file_with_replay_offset()
{
    __asm
    {
        call add_replay_offset
        push edx
        mov edx, dword ptr [FrameCountIngame]
        mov [edx], eax
        pop edx
        push 0x0043a75a // return address
    }
    PROLOG_EPILOG(byteswap_replay_buffer_wrapper)
}

void __cdecl set_demo_state()
{
    DemoState = nextDemoState;
}

FunctionPointer(void, sub_00673ae0, (), 0x00673ae0);

void __declspec(naked) set_next_demo_state()
{
    __asm
    {
        pushfd
        pushad
    }
    if (MenuButtons_Held[0] & Buttons_Y)
    {
        CurrentSubMenu = 6;
        __asm
        {
            popad
            popfd
            mov ebx, 0x1
            ret
        }
    }
    else if (MenuButtons_Held[0] & Buttons_X)
    {
        nextDemoState = 2;
        CurrentSubMenu = 5;

        __asm
        {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
    else
    {
        nextDemoState = 0;
        CurrentSubMenu = 5;
        __asm
        {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
}

/*
void __declspec(naked) set_next_demo_state_thunk()
{
    __asm push 0x00672a66 // return address
    PROLOG_EPILOG(set_next_demo_state)
}
*/

void __declspec(naked) set_demo_state_thunk()
{
    __asm push 0x00678407 // return address
    PROLOG_EPILOG(set_demo_state)
}

void __declspec(naked) load_demo_thunk()
{
    __asm
    {
        pushfd
        pushad
    }
    if (isLoadingCustomDemo)
    {
        __asm
        {
            popad
            popfd
            push eax
            push replayNamePointer
            mov edx, dword ptr replayNameInGamePointer
            push customDemoString
            jmp ADDR_0045459e
        }
    }
    else
    {
        __asm
        {
            popad
            popfd
            push 0x0194086c
            jmp ADDR_00454594
        }
    }
}

void __declspec(naked) load_demo_thunk_2()
{
    __asm
    {
        xor edi, edi
        pushfd
        pushad
    }
    if (isLoadingCustomDemo)
    {
        isLoadingCustomDemo = false;
        __asm
        {
            popad
            popfd
            mov esi, dword ptr replayNameInGamePointer
        }
    }
    else
    {
        __asm
        {
            popad
            popfd
            lea esi, [esp + 0x20]
        }
    }
    __asm
    {
        jmp ADDR_004545af
    }
}

void upgrade_assignment()
{
    switch (DemoState)
    {
    case 1:
    {
        if (isLoadingCustomDemo)
        {
            // Apply upgrades to the character
            MainCharObj2[0]->Upgrades = currentReplay.upgradeBitfield;

            // To get the upgrade platforms to work, we need to set the appropriate byte
            // in the save data
            UINT32 workingBitfield = currentReplay.upgradeBitfield;
            char* upgradePointer = nullptr;
            char* upgradeEnd = nullptr;

            // Set the start/end pointer for UpgradeGot bytes, and shift the upgrade
            // bitfield to the starting bit of the appropriate character's upgrades
            switch (MainCharObj2[0]->CharID)
            {
            case Characters_Sonic:
            {
                upgradePointer = &SonicLightShoesGot;
                upgradeEnd = upgradePointer + 6;
                break;
            }
            case Characters_Shadow:
            {
                upgradePointer = &ShadowAirShoesGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 16;
                break;
            }
            case Characters_Knuckles:
            {
                upgradePointer = &KnucklesShovelClawGot;
                upgradeEnd = upgradePointer + 5;
                workingBitfield >>= 10;
                break;
            }
            case Characters_Rouge:
            {
                upgradePointer = &RougePickNailsGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 25;
                break;
            }
            case Characters_MechTails:
            {
                upgradePointer = &TailsBoosterGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 6;
                break;
            }
            case Characters_MechEggman:
            {
                upgradePointer = &EggmanJetEngineGot;
                upgradeEnd = upgradePointer + 5;
                workingBitfield >>= 20;
                break;
            }
            default: break;
            }

            // Upgrades map 1:1 between bitfield and byte flags. Set byte to
            // bit and shift the working bitfield
            for (upgradePointer; upgradePointer < upgradeEnd; upgradePointer++)
            {
                *upgradePointer = (char)(workingBitfield & 0x1);
                workingBitfield >>= 1;
            }
        }
        else
        {
            switch (MainCharObj2[0]->CharID)
            {
            case Characters_Sonic:
            {
                MainCharObj2[0]->Upgrades = 0x1;
                break;
            }
            case Characters_Shadow:
            {
                MainCharObj2[0]->Upgrades = 0x10000;
                break;
            }
            case Characters_Knuckles:
            {
                MainCharObj2[0]->Upgrades = 0x400;
                break;
            }
            case Characters_Rouge:
            {
                MainCharObj2[0]->Upgrades = 0x2000000;
                break;
            }
            case Characters_MechTails:
            {
                MainCharObj2[0]->Upgrades = 0x40;
                break;
            }
            case Characters_MechEggman:
            {
                MainCharObj2[0]->Upgrades = 0x100000;
                break;
            }
            default: break;
            }
        }
        break;
    }
    case 2:
    {
        currentReplay.upgradeBitfield = MainCharObj2[0]->Upgrades;
        break;
    }
    default: break;
    }
}

void __declspec(naked) upgrade_assignment_helper()
{
    PROLOG_EPILOG(upgrade_assignment)
}

void __declspec(naked) upgrade_assignment_helper_andknuckles()
{
    __asm
    {
        // Replace clobbered instructions
        pop edi
        pop esi

        push 0x00728411 // return address

        jmp upgrade_assignment_helper
    }
}

void __declspec(naked) upgrade_text_skip()
{
    __asm
    {
        pushfd
        pushad
    }
    if (Controllers[0].press & Buttons_A)
    {
        __asm
        {
            popad
            popfd
            push 0x006b71ad
            ret
        }
    }
    else
    {
        __asm
        {
            popad
            popfd
            lea eax, [eax * 0x4 + 0x4]
            push 0x006b719a
            ret
        }
    }
}

int tempFramecount = 0;
int tempGlobalFramecount = 0;
int tempGlobalFramecountDiff = 0;

VoidFunc(sa2_rand, 0x7A89D8);

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
        tempFramecount = FrameCount;
        break;
    }
    case 2:
    {
        if (TimesRestartedOrDied == 0)
        {
            currentReplay.framecount = FrameCount;
        }
        [[fallthrough]];
    }
    case 1:
    {
        if (TimesRestartedOrDied == 0)
        {
            for (int i = 0; i < rand_table[static_cast<unsigned short>(CurrentLevel)]; i++)
                sa2_rand();
            tempGlobalFramecount = FrameCount;
        }
        tempGlobalFramecountDiff = FrameCount - tempGlobalFramecount;
        tempFramecount = currentReplay.framecount + tempGlobalFramecountDiff;
        break;
    }
    default: break;
    }
    __asm
    {
        popad
        popfd
        mov edi, tempFramecount
        ret
    }
}

void replay_watermark_helper()
{
    constexpr NJS_COLOR textColor = { .argb = { 0xff, 0xff, 0xff, 0x7f } };
    // Draw every 32 frames
    if ((replay_offset & 0xff) == 0)
    {
        constexpr uint8_t timer = 0xff;
        switch (DemoState)
        {
        case 1:
        {
            DrawCustomText("Playback", 0.f, 0.f, timer, &textColor);
            break;
        }
        case 2:
        {
            DrawCustomText("Recording", 0.f, 0.f, timer, &textColor);
            break;
        }
        default: break;
        }
    }
}

void __declspec(naked) replay_watermark()
{
    __asm push 0x0043d1f3 // return address
    PROLOG_EPILOG(replay_watermark_helper)
}

DataPointer(ef_message*, MissionMessage, 0x1A5A3F4);

extern "C"
{
    __declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };

    __declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
    {
        rand_table[5] = 99;
        rand_table[7] = 101;
        rand_table[8] = 109;
        rand_table[16] = 103;
        rand_table[18] = 104;
        rand_table[25] = 102;
        rand_table[26] = 101;
        rand_table[32] = 101;
        rand_table[44] = 108;

        coolSquare[0] = { 100.0f, 100.0f, .9f, 0xa00000ff };
        coolSquare[1] = { 100.0f, 380.0f, .9f, 0xa00000ff };
        coolSquare[2] = { 540.0f, 100.0f, .9f, 0xa00000ff };
        coolSquare[3] = { 540.0f, 380.0f, .9f, 0xa00000ff };

        replay_offset = 0;
        isLoadingCustomDemo = false;

        // Add call to write replay file in single player mode
        WriteCall((void*)0x0043bc77, reinterpret_cast<void *>(go_to_next_level_thunk));

        // Write call to grab the frame count before it resets
        WriteJump((void*)0x0044cae8, fuckin_get_the_level_counter_before_it_goes_away);
        WriteJump((void*)0x0043d82d, fuckin_get_the_level_counter_before_it_goes_away2);

        // Write/Read from the demo buffer based on replay_offset
        WriteJump((void*)0x004421be, buffer_with_replay_offset);

        // Write to file based on replay_offset
        WriteJump((void*)0x0043a755, write_file_with_replay_offset);

        // Skip code that prevents pausing
        WriteJump((void*)0x0043d063, (void*)0x0043d0ae);

        // Flash my own custom text when recording/playing back
        WriteJump((void*)0x0043d15f, replay_watermark);

        // Set demo state on level start depending on button inputs
        WriteJump((void*)0x00678401, set_demo_state_thunk);
        // Set demo state on level start depending on button inputs
        //WriteJump((void*)0x00672a5c, set_next_demo_state_thunk);

        // Function controlling the logic for loading title demos vs. custom demos
        WriteJump((void*)0x0045458f, load_demo_thunk);
        WriteJump((void*)0x004545a9, load_demo_thunk_2);

        /*************************************************************************/
        /* Call our custom function for setting upgrades, and NOP as appropriate */
        /*************************************************************************/
        WriteCall((void*)0x00717090, upgrade_assignment_helper); // Sonic
        WriteData<2>((void*)0x00717095, (char)0x90);

        WriteCall((void*)0x007175f0, upgrade_assignment_helper); // Shadow
        WriteData<2>((void*)0x007175f5, (char)0x90);

        WriteJump((void*)0x0072840c, upgrade_assignment_helper_andknuckles); // Knuckles needs a little help :)

        WriteCall((void*)0x00728855, upgrade_assignment_helper); // Rouge
        WriteData<2>((void*)0x0072885a, (char)0x90);

        WriteCall((void*)0x00740e47, upgrade_assignment_helper); // EWalker
        WriteData<2>((void*)0x00740e4c, (char)0x90);

        WriteCall((void*)0x007410a7, upgrade_assignment_helper); // TWalker
        WriteData<2>((void*)0x007410ac, (char)0x90);
        /***************************************************************************/
        /* Modify jump table to add states 6 and 7 to the Stage Select state table */
        /***************************************************************************/
        void (*case6_ptr)() = &menu_stage_select_case6_thunk;
        void (*case7_ptr)() = &menu_stage_select_case7_thunk;
        WriteData((void*)0x00672b04, &case6_ptr, 4);
        WriteData((void*)0x00672b08, &case7_ptr, 4);
        WriteData<1>((void*)0x0067276f, (char)0x07);

        WriteJump((void*)0x006b7193, upgrade_text_skip);

        // Control during upgrade text
        WriteData<7>((void*)0x006d89db, (char)0x90);

        /***************************************************************************/
        /* Emerald stuff                                                           */
        /***************************************************************************/
        // Remove cmp/jump for if DemoState != 0
        WriteData<7>((void*)0x007380b0, (char)0x90); // NOP cmp [DemoState] 0x0
        WriteData<2>((void*)0x007380bd, (char)0x90); // NOP jnz 0x007380da

        WriteJump((void*)0x007380bf, set_num_rand_calls_hunting); // sets edi to the desired value

        WriteData<1>((void*)0x007380c4, (char)0x90); // NOP leftover byte

        // Some unloading stuff...
        WriteData<10>((void*)0x00672a5c, (char)0x90);
        WriteCall((void*)0x0672a37, set_next_demo_state);
    }
}