#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "StageSelectMode.h"
#include "Config.h"
#include "Hunting.h"
#include "Upgrade.h"
#include "SA2DemoRecordingUtil.h"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

static_assert (sizeof(DemoInput) == 0x2c, "Size is not correct");

int nextDemoState = 0;

extern char replayName[256];
char* replayNamePointer = (char*)&replayName;
char replayNameInGame[256] = { 0 };
char* replayNameInGamePointer = (char*)&replayNameInGame;

extern ReplayMeta current_replay;

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

Config config;

void write_replay_buffer_thunk() {
    if (DemoState == 2) {
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
        ReplayMeta::write_replay_metafile(config.author.c_str(), current_replay.upgradeBitfield, CurrentCharacter, CurrentLevel, current_replay.framecount, file_name.str().c_str(), meta_name.c_str());
        DemoState = 0;
    }
}

void update_reset_replay_offset() {
    if (GameState == 3) {
        replay_offset = 0;
    }
    else {
        replay_offset += FrameCountIngame;
    }
    FrameCountIngame = 0;
}

void update_replay_offset() {
    replay_offset += FrameCountIngame;
    FrameCountIngame = 0;
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away() {
    __asm push 0x0044caf2 // return address
    PROLOG_EPILOG(update_reset_replay_offset)
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away2() {
    __asm push 0x0043d837 // return address
    PROLOG_EPILOG(update_replay_offset)
}

void __declspec(naked) go_to_next_level_thunk() {
    __asm push 0x0043d5d0 // return address
    PROLOG_EPILOG(write_replay_buffer_thunk)
}

int __declspec(naked) add_replay_offset() {
    __asm {
        mov eax, dword ptr [FrameCountIngame]
        mov eax, [eax]
        add eax, [replay_offset]
        ret
    }
}

void __declspec(naked) buffer_with_replay_offset() {
    __asm {
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

void __declspec(naked) write_file_with_replay_offset() {
    __asm {
        call add_replay_offset
        push edx
        mov edx, dword ptr [FrameCountIngame]
        mov [edx], eax
        pop edx
        push 0x0043a75a // return address
    }
    PROLOG_EPILOG(byteswap_replay_buffer_wrapper)
}

// This hook is right before a sprintf call. This sets up the appropriate
// strings/variables depending on whether it's a custom demo or not.
__declspec(naked) void init_demo_string() {
    __asm {
        pushfd
        pushad
    }
    if (isLoadingCustomDemo) {
        // Push dummy argument, followed by the name of the replay name and the
        // demo format string. Loading replayNameInGamePointer into edx because
        // it will get pushed after jumping back.
        __asm {
            popad
            popfd
            push eax
            push replayNamePointer
            mov edx, dword ptr replayNameInGamePointer
            push customDemoString
            jmp ADDR_0045459e
        }
    }
    else {
        // Do the thing we overwrote and jump back
        __asm {
            popad
            popfd
            push 0x0194086c
            jmp ADDR_00454594
        }
    }
}

// This hook is before calling read_file, making sure the correct string is
// sent to the function (placed in esi).
__declspec(naked) void read_demo_hook() {
    __asm {
        xor edi, edi
        pushfd
        pushad
    }
    if (isLoadingCustomDemo) {
        isLoadingCustomDemo = false;
        __asm {
            popad
            popfd
            mov esi, dword ptr replayNameInGamePointer
        }
    }
    else {
        __asm {
            popad
            popfd
            lea esi, [esp + 0x20]
        }
    }
    __asm {
        jmp ADDR_004545af
    }
}

void __declspec(naked) upgrade_text_skip() {
    __asm {
        pushfd
        pushad
    }
    if (Controllers[0].press & Buttons_A) {
        __asm {
            popad
            popfd
            push 0x006b71ad
            ret
        }
    }
    else {
        __asm {
            popad
            popfd
            lea eax, [eax * 0x4 + 0x4]
            push 0x006b719a
            ret
        }
    }
}

void replay_watermark_helper() {
    constexpr NJS_COLOR textColor = { .argb = { 0xff, 0xff, 0xff, 0x7f } };
    // Draw every 32 frames
    constexpr uint8_t timer = 0x1;
    switch (DemoState) {
    case 1: {
        DrawCustomText("Playback", 0.f, 0.f, timer, &textColor);
        break;
    }
    case 2: {
        DrawCustomText("Recording", 0.f, 0.f, timer, &textColor);
        break;
    }
    default: break;
    }
    return;
}

void __declspec(naked) replay_watermark() {
    __asm push 0x0043d1f3 // return address
    PROLOG_EPILOG(replay_watermark_helper)
}

/*
void __declspec(naked) skip_results_screen() {
    __asm {
        pushfd
        pushad
    }
    Cheats_ExitStage = 1;
    __asm {
        popad
        popfd
        ret
    }
}
*/

extern "C" {
    __declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };

    __declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions) {
        //WriteCall(reinterpret_cast<void*>(0x43e80e), skip_results_screen);

        config = Config("mods/SA2DemoRecording/config.ini");

        Hunting::Init();
        Upgrade::Init();
        StageSelectMode::Init();

        coolSquare[0] = { 100.0f, 100.0f, .9f, 0xa00000ff };
        coolSquare[1] = { 100.0f, 380.0f, .9f, 0xa00000ff };
        coolSquare[2] = { 540.0f, 100.0f, .9f, 0xa00000ff };
        coolSquare[3] = { 540.0f, 380.0f, .9f, 0xa00000ff };

        replay_offset = 0;
        isLoadingCustomDemo = false;

        // Add call to write replay file in single player mode
        WriteCall((void*)0x0043bc77, go_to_next_level_thunk);

        // Write call to grab the frame count before it resets
        WriteJump((void*)0x0044cae8, fuckin_get_the_level_counter_before_it_goes_away);
        WriteJump((void*)0x0043d82d, fuckin_get_the_level_counter_before_it_goes_away2);

        // Write/Read from the demo buffer based on replay_offset
        WriteJump((void*)0x004421be, buffer_with_replay_offset);

        // Write to file based on replay_offset
        WriteJump((void*)0x0043a755, write_file_with_replay_offset);

        // Skip code that prevents pausing
        WriteJump(reinterpret_cast<void*>(0x0043d063), reinterpret_cast<void*>(0x0043d0ae));

        // Flash my own custom text when recording/playing back
        WriteJump((void*)0x0043d15f, replay_watermark);

        // Function controlling the logic for loading title demos vs. custom demos
        WriteJump((void*)0x0045458f, init_demo_string);
        WriteJump((void*)0x004545a9, read_demo_hook);

        WriteJump((void*)0x006b7193, upgrade_text_skip);

        // Control during upgrade text
        WriteData<7>((void*)0x006d89db, (char)0x90);
    }
}