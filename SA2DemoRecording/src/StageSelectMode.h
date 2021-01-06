#pragma once
#ifndef _STAGE_SELECT_MODE_H
#define _STAGE_SELECT_MODE_H
#include "SA2ModLoader.h"

extern char replayName[256];

/* Create a keepMessage object that lasts 1 frame at the given position */
void DrawCustomText(const char* str, float x, float y, unsigned char duration, const NJS_COLOR* color);

void menu_stage_select_case6();
void menu_stage_select_case7();

#endif