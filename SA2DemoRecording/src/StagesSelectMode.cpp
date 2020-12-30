#include <vector>

#include "SA2ModLoader.h"
#include "ReplayMeta.h"

#include "StageSelectMode.h"

std::vector<ReplayMeta> replayList;
size_t replayListIndex;
int previousIndex;

char replayName[256] = { 0 };

ReplayMeta currentReplay;

extern int nextDemoState;
extern bool isLoadingCustomDemo;


/* Create list of demos */
void menu_stage_select_case6()
{
	replayList.clear();
	replayList = ReplayMeta::create_replaymeta_list("resource/gd_PC/Demos/");

	std::vector<ReplayMeta>::iterator it = replayList.begin();

	while (it != replayList.end())
	{
		if (it->level != StageSelectLevels[StageSelectLevelSelection].Level)
		{
			it = replayList.erase(it);
		}
		else
		{
			it++;
		}
	}

	replayListIndex = 0;
	previousIndex = -1;

	CurrentSubMenu = 7;
}

/* Select from list of above demos */
void menu_stage_select_case7()
{
	if (MenuButtons_Pressed[0] & Buttons_B)
	{
		nextDemoState = 0;
		CurrentSubMenu = 3;
	}
	else if (MenuButtons_Pressed[0] & Buttons_A)
	{
		currentReplay = replayList[replayListIndex];
		strncpy_s(replayName, 256, currentReplay.file.c_str(), currentReplay.file.length() + 1);
		nextDemoState = 1;
		isLoadingCustomDemo = true;
		CurrentSubMenu = 5;
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
		replayListIndex -= 1;
		if (replayListIndex < 0)
		{
			replayListIndex = 0;
		}
	}

	if (replayListIndex != previousIndex)
	{
		PrintDebug("%s", replayList[replayListIndex].string().c_str());
		previousIndex = replayListIndex;
	}
}