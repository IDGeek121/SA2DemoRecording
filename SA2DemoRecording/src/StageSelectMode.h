#pragma once
#ifndef _STAGE_SELECT_MODE_H
#define _STAGE_SELECT_MODE_H
#include "SA2ModLoader.h"

extern char replayName[256];

//void __usercall DrawGameText(const char* str@<ecx>, const NJS_VECTOR* pos@<eax>, float scale, const NJS_COLOR* color)
static const void* const DrawGameText_ptr = (void*)0x667410;
static inline void DrawGameText(const char* str_in, const NJS_VECTOR* pos, float scale, const NJS_COLOR* color)
{
    __asm
    {
        push[color]
        push[scale]
        mov eax, [pos]
        mov ecx, [str_in]
        call DrawGameText_ptr
        add esp, 0x8
    }
}

void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color);

class StageSelectMode {
public:
    static void Init();
};

#endif