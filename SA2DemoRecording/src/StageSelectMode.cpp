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

DemoState next_demo_state;
extern bool isLoadingCustomDemo;

extern PolygonPoint coolSquare[4];

extern bool from_restart;

DataPointer(char*, keepMessage_str, 0x008cb09c);

/* Create a keepMessage task and draw it to the screen. I just copied how it was done in some menu string drawing function. */
void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color) {
    ef_message* message = ef_message_New(str, 1, 1, 4);

    if (message != nullptr) {
        InitMessage(message, color, x, y, 1.f, 1.f);

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
        CreateAndDrawMessage("\tNo replays for this stage.", &pos, 1.f, &text_color);
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
    CreateAndDrawMessage(replay_list.at(replay_list_index).string().c_str(), &pos, 1.f, &textColor);
    CreateAndDrawMessage("\tSelect a replay.", &pos_2, 1.f, &textColor);
    if (MenuButtons_Pressed[0] & Buttons_B) {
        next_demo_state = DemoState_None;
        CurrentSubMenu = 3;
        PlaySoundProbably(0x8009, 0, 0, 0);
    }
    else if (MenuButtons_Pressed[0] & Buttons_A) {
        current_replay = replay_list.at(replay_list_index);
        strncpy_s(replayName, 256, current_replay.file.c_str(), current_replay.file.length() + 1);
        next_demo_state = DemoState_Playback;
        isLoadingCustomDemo = true;
        from_restart = current_replay.restart;

        // Clean up the menu stuff a-la case 3's normal process.
        FUN_00673ae0();
        MenuControlsVisible = 0;
        *(int*)StageSelectData = 0;
        *(int*)(StageSelectData + 4) = 0;
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
        next_demo_state = DemoState_Recording;
        current_replay.restart = false;
        isLoadingCustomDemo = true;
        CurrentSubMenu = 5;
        from_restart = false;

        __asm {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
    else {
        next_demo_state = DemoState_None;
        CurrentSubMenu = 5;
        current_replay.restart = false;
        from_restart = false;
        __asm {
            popad
            popfd
            call sub_00673ae0
            ret
        }
    }
}

void __cdecl set_demo_state_helper() {
    CurrentDemoState = next_demo_state;
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