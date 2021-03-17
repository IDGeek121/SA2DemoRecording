#pragma once
#ifndef _STAGE_SELECT_MODE_H
#define _STAGE_SELECT_MODE_H
#include "SA2ModLoader.h"

extern char replayName[256];

void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color);

class StageSelectMode {
public:
    static void Init();
};

#endif