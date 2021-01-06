#include <vector>

#include "SA2ModLoader.h"
#include "ReplayMeta.h"

#include "StageSelectMode.h"

std::vector<ReplayMeta> replayList;
unsigned int replayListIndex;
int previousIndex;

char replayName[256] = { 0 };

ReplayMeta currentReplay;

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


DataPointer(char*, task_c_str, 0x008b9528);
DataPointer(char*, keepMessage_str, 0x008cb09c);

FunctionPointer(ef_message*, ef_message_new, (const char* str, int language, short a3, short a4), 0x6B7F40);

/* Create a keepMessage task and draw it to the screen. I just copied how it was done in some menu string drawing function. */
void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color)
{
    ef_message* message = ef_message_new(str, 1, -1, -1);
    InitMenuString(message, color, x, y, 1.f, 1.f);

    ObjectMaster* test = nullptr;
    if (test = AllocateObjectMaster(keepMessage, 2, keepMessage_str); test != nullptr)
    {
        void* test2 = MemoryManager__Allocate(0x34, task_c_str, 0x523);
        Data1Ptr actionStruct = { (void*)((int*)test2 + 1) };
        *(int*)test2 = 0x12345678;
        memset(actionStruct.Undefined, 0, 0x30);
        if (actionStruct.Undefined == nullptr)
        {
            DeleteObject_(test);
        }
        test->Data1 = actionStruct;
        test->EntityData2 = (UnknownData2 *)message;
        *(byte*)(test->Data1.Undefined) = uint8_t{ duration };
        *((byte *)test->Data1.Undefined + 0x3) = 0;
        test->DeleteSub = keepMessage_Delete;
    }
    return;
}

/* Create list of demos */
void menu_stage_select_case6()
{
    replayList.clear();
    replayList = ReplayMeta::create_replaymeta_list("resource/gd_PC/Demos/");

    std::vector<ReplayMeta>::iterator it = replayList.begin();

    while (it != replayList.end())
    {
        if (it->level != StageSelectLevels[static_cast<unsigned char>(StageSelectLevelSelection)].Level)
        {
            it = replayList.erase(it);
        }
        else
        {
            it++;
        }
    }

    replayListIndex = 0;
    previousIndex = 0;

    CurrentSubMenu = 7;
}

FunctionPointer(void, FUN_00673ae0, (), 0x00673ae0);

/* Select from list of above demos */
void menu_stage_select_case7()
{
    DrawPolygon(4, 1, coolSquare);
    constexpr NJS_COLOR textColor = { 0xffffffff };
    DrawCustomText(replayList[replayListIndex].string().c_str(), 100.f, 100.f, uint8_t{ 1 }, &textColor);
    if (MenuButtons_Pressed[0] & Buttons_B)
    {
        nextDemoState = 0;
        CurrentSubMenu = 3;
        PlaySoundProbably(0x8009, 0, 0, 0);
    }
    else if (MenuButtons_Pressed[0] & Buttons_A)
    {
        currentReplay = replayList[replayListIndex];
        strncpy_s(replayName, 256, currentReplay.file.c_str(), currentReplay.file.length() + 1);
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
    else if (MenuButtons_Pressed[0] & Buttons_Down)
    {
        replayListIndex += 1;
        if (replayListIndex >= replayList.size())
        {
            replayListIndex = replayList.size() - 1;
        }
    }
    else if (MenuButtons_Pressed[0] & Buttons_Up)
    { 
        if (replayListIndex != 0)
        {
            replayListIndex -= 1;
        }
    }

    if (replayListIndex != previousIndex)
    {
        previousIndex = replayListIndex;
        PlaySoundProbably(0x8000, 0, 0, 0);
    }
}