#include <vector>

#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "SA2DemoRecordingUtil.h"

#include "StageSelectMode.h"

std::vector<ReplayMeta> replay_list;
unsigned int replay_list_index;
int previousIndex;

char replayName[256] = { 0 };

ReplayMeta current_replay;

extern int nextDemoState;
extern bool isLoadingCustomDemo;

struct PolygonPoint {
    NJS_POINT3 pos;
    NJS_COLOR color;
};

extern PolygonPoint coolSquare[4];

static const void* const DrawPolygon_ptr = (void*)0x77F7F0;
static inline void DrawPolygon(int numPoints, int readAlpha_q, const PolygonPoint* pointArray)
{
    __asm
    {
        mov ecx, [numPoints]
        mov eax, [readAlpha_q]
        push[pointArray]
        call DrawPolygon_ptr
        add esp, 0x4
    }
}

static const void* const InitMenuString_ptr = (void*)0x6B5A60;
static inline void InitMenuString(ef_message* message, const NJS_COLOR* color, float x_pos, float y_pos, float x_scale, float y_scale)
{
    __asm
    {
        mov edx, [color]
        mov eax, [message]
        push [y_scale]
        push [x_scale]
        push [y_pos]
        push [x_pos]
        call InitMenuString_ptr
        add esp, 0x10
    }
}

DataPointer(char*, keepMessage_str, 0x008cb09c);

FunctionPointer(ef_message*, ef_message_new, (const char* str, int language, short a3, short a4), 0x6B7F40);

/* Create a keepMessage task and draw it to the screen. I just copied how it was done in some menu string drawing function. */
void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color) {
    ef_message* message = ef_message_new(str, 1, 1, 4);

    if (message != nullptr) {
        InitMenuString(message, color, x, y, 1.f, 1.f);

        ObjectMaster* task_keepMessage = AllocateObjectMaster(keepMessage, 2, keepMessage_str);

        if (task_keepMessage != nullptr) {
            EntityData1* action_struct = { AllocateEntityData1() };
            if (action_struct == nullptr)
                DeleteObject_(task_keepMessage);
            task_keepMessage->Data1.Entity = action_struct;
            task_keepMessage->EntityData2 = reinterpret_cast<UnknownData2*>(message);
            task_keepMessage->Data1.Entity->Action = unsigned char { duration };
            task_keepMessage->DeleteSub = keepMessage_Delete;
        }
    }

    return;
}

/* Create list of demos */
void menu_stage_select_case6() {
    replay_list.clear();
    replay_list = ReplayMeta::create_replaymeta_list("resource/gd_PC/Demos/");

    std::vector<ReplayMeta>::iterator it = replay_list.begin();

    while (it != replay_list.end()) {
        if (it->level != StageSelectLevels[static_cast<unsigned char>(StageSelectLevelSelection)].Level)
            it = replay_list.erase(it);
        else
            it++;
    }

    constexpr NJS_VECTOR pos = { .x = 0.f, .y = 400.f, .z = 0.f };

    if (replay_list.empty()) {
        constexpr NJS_COLOR text_color = { 0xffffffff };
        DrawGameText("  No replays for this stage.", &pos, 1.f, &text_color);
        CurrentSubMenu = 3;
        PlaySoundProbably(0x8009, 0, 0, 0);
        return;
    }

    replay_list_index = 0;
    previousIndex = 0;

    CurrentSubMenu = 7;
}

FunctionPointer(void, FUN_00673ae0, (), 0x00673ae0);

/* Select from list of above demos */
void menu_stage_select_case7() {
    DrawPolygon(4, 1, coolSquare);
    constexpr NJS_COLOR textColor = { 0xffffffff };
    constexpr NJS_VECTOR pos = { .x = 100.f, .y = 100.f, .z = 0.f };
    constexpr NJS_VECTOR pos_2 = { .x = 100.f, .y = 20.f, .z = 0.f };
    DrawGameText(replay_list.at(replay_list_index).string().c_str(), &pos, 1.f, &textColor);
    DrawGameText("  Select a replay.", &pos_2, 1.f, &textColor);
    if (MenuButtons_Pressed[0] & Buttons_B) {
        nextDemoState = 0;
        CurrentSubMenu = 3;
        PlaySoundProbably(0x8009, 0, 0, 0);
    }
    else if (MenuButtons_Pressed[0] & Buttons_A) {
        current_replay = replay_list.at(replay_list_index);
        strncpy_s(replayName, 256, current_replay.file.c_str(), current_replay.file.length() + 1);
        nextDemoState = 1;
        isLoadingCustomDemo = true;

        // Clean up the menu stuff a-la case 3's normal process.
        FUN_00673ae0();
        MenuControlsVisible = 0;
        *(int *)StageSelectData = 0;
        *(int *)(StageSelectData + 4) = 0;
        CurrentSubMenu = 5;
        PlaySoundProbably(0x8001, 0, 0, 0);
    }
    else if (MenuButtons_Pressed[0] & Buttons_Down) {
        replay_list_index += 1;
        if (replay_list_index >= replay_list.size())
            replay_list_index = replay_list.size() - 1;
    }
    else if (MenuButtons_Pressed[0] & Buttons_Up){ 
        if (replay_list_index != 0)
            replay_list_index -= 1;
    }

    if (replay_list_index != previousIndex) {
        previousIndex = replay_list_index;
        PlaySoundProbably(0x8000, 0, 0, 0);
    }
}

void __declspec(naked dllexport) menu_stage_select_case6_thunk() {
    __asm push 0x00672ae1
    PROLOG_EPILOG(menu_stage_select_case6)
}

void __declspec(naked dllexport) menu_stage_select_case7_thunk() {
    __asm push 0x00672ae1
    PROLOG_EPILOG(menu_stage_select_case7)
}

FunctionPointer(void, sub_00673ae0, (), 0x00673ae0);

__declspec(naked) void set_next_demo_state() {
    __asm {
        pushfd
        pushad
    }
    if (MenuButtons_Held[0] & Buttons_Y) {
        CurrentSubMenu = 6;
        __asm {
            popad
            popfd
            mov ebx, 0x1
            ret
        }
    }
    else if (MenuButtons_Held[0] & Buttons_X) {
        nextDemoState = 2;
        CurrentSubMenu = 5;

        __asm {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
    else {
        nextDemoState = 0;
        CurrentSubMenu = 5;
        __asm {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
}

void __cdecl set_demo_state_helper() {
    DemoState = nextDemoState;
}

void __declspec(naked) set_demo_state() {
    __asm push 0x00678407 // return address
    PROLOG_EPILOG(set_demo_state_helper)
}

void StageSelectMode::Init() {
    // Set logic in state 3 to go to either state 5 or state 6 depending on the
    // button you are holding down.
    WriteData<10>((void*)0x00672a5c, (char)0x90);
    WriteCall((void*)0x0672a37, set_next_demo_state);

    // Set demo state on level start depending on button inputs. This is a hook
    // in state 5 of the stage select menu.
    WriteJump((void*)0x00678401, set_demo_state);

    // Modify jump table to add states 6 and 7 to the Stage Select state table
    void (*case6_ptr)() = &menu_stage_select_case6_thunk;
    void (*case7_ptr)() = &menu_stage_select_case7_thunk;
    WriteData((void*)0x00672b04, &case6_ptr, 4);
    WriteData((void*)0x00672b08, &case7_ptr, 4);
    WriteData<1>((void*)0x0067276f, char{ 0x07 });
}