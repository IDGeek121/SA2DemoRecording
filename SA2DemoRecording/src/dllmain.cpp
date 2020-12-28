#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "SA2ModLoader.h"
#include "ReplayMeta.h"
#include "StageSelectMode.h"

int nextDemoState = 0;

extern char replayName[256];
char* replayNamePointer = (char*)&replayName;
char replayNameInGame[256] = { 0 };
char* replayNameInGamePointer = (char*)&replayNameInGame;

FunctionPointer(int, sa2_sprintf, (char*, const char*, ...), 0x7a7c6d);
FunctionPointer(void, FUN_0043d5d0, (), 0x0043d5d0);

DataPointer(char*, demoString, 0x008b9110);

void* ADDR_0044caf2 = (void*)0x0044caf2;
void* ADDR_004421c3 = (void*)0x004421c3;
void* ADDR_0043a75a = (void*)0x0043a75a;
void* ADDR_0043d837 = (void*)0x0043d837;
void* ADDR_00678407 = (void*)0x00678407;
void* ADDR_00454594 = (void*)0x00454594;
void* ADDR_0045459e = (void*)0x0045459e;
void* ADDR_00672a66 = (void*)0x00672a66;

void* ADDR_00672ae1 = (void*)0x00672ae1;

void* ADDR_004545af = (void*)0x004545af;

uint32_t replayOffset;
bool isLoadingCustomDemo;

const char* customDemoString = "Demos/%s.bin";

static const void* const write_replay_buffer_ptr = (void*)0x0043a730;
static inline void write_replay_buffer(const char* filename)
{
	__asm
	{
		mov edi, [filename]
		call write_replay_buffer_ptr
	}
}

static const void* const byteswap_replay_buffer_ptr = (void*)0x00454450;
static inline void byteswap_replay_buffer(void* buffer)
{
	__asm
	{
		mov eax, [buffer]
		call byteswap_replay_buffer_ptr
	}
}

void __declspec(naked dllexport) menu_stage_select_case6_thunk()
{
	__asm
	{
		pushfd
		pushad
		call menu_stage_select_case6
		popad
		popfd
		jmp ADDR_00672ae1
	}
}

void __declspec(naked dllexport) menu_stage_select_case7_thunk()
{
	__asm
	{
		pushfd
		pushad
		call menu_stage_select_case7
		popad
		popfd
		jmp ADDR_00672ae1
	}
}


void __cdecl write_replay_buffer_thunk()
{
	if (DemoState == 2)
	{
		std::ostringstream filename;
		std::time_t currentTime = std::time(NULL);
		std::tm testTime;
		gmtime_s(&testTime, &currentTime);

		std::ios state(nullptr);
		state.copyfmt(std::cout);
		filename << "demo" << std::setfill('0') <<  std::setw(2) << CurrentLevel << "_" << std::put_time(&testTime, "%F_%H-%M-%S");
		std::cout.copyfmt(state);
		std::string demoFolder = "resource/gd_PC/Demos/";
		std::string metaname = demoFolder + filename.str() + ".ini";
		std::string replayname = demoFolder + filename.str() + ".bin";
		PrintDebug("Writing demo file to %s", replayname.c_str());
		write_replay_buffer(replayname.c_str());
		PrintDebug("Writing demo metafile to %s", metaname.c_str());
		ReplayMeta::write_replay_metafile("test", "test", MainCharObj2[0]->Upgrades, CurrentCharacter, CurrentLevel, FrameCount, filename.str().c_str(), metaname.c_str());
		DemoState = 0;
	}
}

void __cdecl update_reset_replay_offset()
{
	if (GameState == 3)
	{
		replayOffset = 0;
	}
	else
	{
		replayOffset += FrameCountIngame;
	}
	FrameCountIngame = 0;
}

void __cdecl update_replay_offset()
{
	replayOffset += FrameCountIngame;
	FrameCountIngame = 0;
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away()
{
	__asm
	{
		pushfd
		pushad
		call update_reset_replay_offset
		popad
		popfd
		jmp ADDR_0044caf2
	}
}

void __declspec(naked) fuckin_get_the_level_counter_before_it_goes_away2()
{
	__asm
	{
		pushfd
		pushad
		call update_replay_offset
		popad
		popfd
		jmp ADDR_0043d837
	}
}

void __declspec(naked) go_to_next_level_thunk()
{
	__asm
	{
		pushfd
		pushad
		call write_replay_buffer_thunk
		popad
		popfd
		jmp FUN_0043d5d0
	}
}

int __declspec(naked) add_replay_offset()
{
	__asm
	{
		mov eax, dword ptr [FrameCountIngame]
		mov eax, [eax]
		add eax, [replayOffset]
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
	int endOfValidBuffer = FrameCountIngame;
	int* buffer = (int*)0x024cfe20;
	for (int x = 0; x < endOfValidBuffer; x++)
	{
		byteswap_replay_buffer(buffer);
		buffer += 11;
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
		pushfd
		pushad
		call byteswap_replay_buffer_wrapper
		popad
		popfd
		jmp ADDR_0043a75a
	}
}

void __cdecl set_demo_state()
{
	DemoState = nextDemoState;
}

void __cdecl set_next_demo_state()
{
	if (MenuButtons_Held[0] & Buttons_Y)
	{
		CurrentSubMenu = 6;
	}
	else if (MenuButtons_Held[0] & Buttons_X)
	{
		nextDemoState = 2;
		CurrentSubMenu = 5;
	}
	else
	{
		nextDemoState = 0;
		CurrentSubMenu = 5;
	}
}

void __declspec(naked) set_next_demo_state_thunk()
{
	__asm
	{
		pushfd
		pushad
		call set_next_demo_state
		popad
		popfd
		jmp ADDR_00672a66
	}
}

void __declspec(naked) set_demo_state_thunk()
{
	__asm
	{
		pushfd
		pushad
		call set_demo_state
		popad
		popfd
		jmp ADDR_00678407
	}
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
			//mov [esp + 0x14], edx
			//lea edx, [esp + 0x14]
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

extern "C"
{
	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };

	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		replayOffset = 0;
		isLoadingCustomDemo = false;

		// Add call to write replay file in single player mode
		WriteCall((void*)0x0043bc77, go_to_next_level_thunk);

		// Write call to grab the frame count before it resets
		WriteJump((void*)0x0044cae8, fuckin_get_the_level_counter_before_it_goes_away);
		WriteJump((void*)0x0043d82d, fuckin_get_the_level_counter_before_it_goes_away2);

		// Write/Read from the demo buffer based on replayOffset
		WriteJump((void*)0x004421be, buffer_with_replay_offset);

		// Write to file based on replayOffset
		WriteJump((void*)0x0043a755, write_file_with_replay_offset);

		// Remove code that prevents pausing and flashes the Start text and exits on death
		WriteJump((void*)0x0043d063, (void*)0x0043d0ae);
		WriteJump((void*)0x0043d15f, (void*)0x0043d1f3);

		// Set demo state on level start depending on button inputs
		WriteJump((void*)0x00678401, set_demo_state_thunk);
		// Set demo state on level start depending on button inputs
		WriteJump((void*)0x00672a5c, set_next_demo_state_thunk);

		// Function controlling the logic for loading title demos vs. custom demos
		WriteJump((void*)0x0045458f, load_demo_thunk);
		WriteJump((void*)0x004545a9, load_demo_thunk_2);

		// NOP all code that forces upgrade values
		WriteData<7>((void*)0x00717090, (char)0x90); // Sonic
		WriteData<7>((void*)0x007175f0, (char)0x90); // Shadow

		WriteData<3>((void*)0x0072840c, (char)0x90); // Knuckles
		WriteData<7>((void*)0x00728855, (char)0x90); // Rouge

		WriteData<7>((void*)0x00740e47, (char)0x90); // EWalker
		WriteData<7>((void*)0x007410a7, (char)0x90); // TWalker

		void (*case6_ptr)() = &menu_stage_select_case6_thunk;
		void (*case7_ptr)() = &menu_stage_select_case7_thunk;
		WriteData((void*)0x00672b04, &case6_ptr, 4);
		WriteData((void*)0x00672b08, &case7_ptr, 4);

		// Increase size of jump table for stage select menu
		WriteData<1>((void*)0x0067276f, (char)0x07);
	}
}