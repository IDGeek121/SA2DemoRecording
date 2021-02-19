#include "SA2ModLoader.h"

#include "SA2DemoRecordingUtil.h"
#include "ReplayMeta.h"
#include "Upgrade.h"

extern ReplayMeta current_replay;
extern bool isLoadingCustomDemo;

void upgrade_assignment() {
    switch (DemoState) {
    case 1: {
        if (isLoadingCustomDemo) {
            // Apply upgrades to the character
            MainCharObj2[0]->Upgrades = current_replay.upgradeBitfield;

            // To get the upgrade platforms to work, we need to set the appropriate byte
            // in the save data
            UINT32 workingBitfield = current_replay.upgradeBitfield;
            char* upgradePointer = nullptr;
            char* upgradeEnd = nullptr;

            // Set the start/end pointer for UpgradeGot bytes, and shift the upgrade
            // bitfield to the starting bit of the appropriate character's upgrades
            switch (MainCharObj2[0]->CharID) {
            case Characters_Sonic: {
                upgradePointer = &SonicLightShoesGot;
                upgradeEnd = upgradePointer + 6;
                break;
            }
            case Characters_Shadow: {
                upgradePointer = &ShadowAirShoesGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 16;
                break;
            }
            case Characters_Knuckles: {
                upgradePointer = &KnucklesShovelClawGot;
                upgradeEnd = upgradePointer + 5;
                workingBitfield >>= 10;
                break;
            }
            case Characters_Rouge: {
                upgradePointer = &RougePickNailsGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 25;
                break;
            }
            case Characters_MechTails: {
                upgradePointer = &TailsBoosterGot;
                upgradeEnd = upgradePointer + 4;
                workingBitfield >>= 6;
                break;
            }
            case Characters_MechEggman: {
                upgradePointer = &EggmanJetEngineGot;
                upgradeEnd = upgradePointer + 5;
                workingBitfield >>= 20;
                break;
            }
            default: break;
            }

            // Upgrades map 1:1 between bitfield and byte flags. Set byte to
            // bit and shift the working bitfield
            for (upgradePointer; upgradePointer < upgradeEnd; upgradePointer++) {
                *upgradePointer = (char)(workingBitfield & 0x1);
                workingBitfield >>= 1;
            }
        }
        else {
            switch (MainCharObj2[0]->CharID)
            {
            case Characters_Sonic: {
                MainCharObj2[0]->Upgrades = 0x1;
                break;
            }
            case Characters_Shadow: {
                MainCharObj2[0]->Upgrades = 0x10000;
                break;
            }
            case Characters_Knuckles: {
                MainCharObj2[0]->Upgrades = 0x400;
                break;
            }
            case Characters_Rouge: {
                MainCharObj2[0]->Upgrades = 0x2000000;
                break;
            }
            case Characters_MechTails: {
                MainCharObj2[0]->Upgrades = 0x40;
                break;
            }
            case Characters_MechEggman: {
                MainCharObj2[0]->Upgrades = 0x100000;
                break;
            }
            default: break;
            }
        }
        break;
    }
    case 2: {
        current_replay.upgradeBitfield = MainCharObj2[0]->Upgrades;
        break;
    }
    default: break;
    }
}

void __declspec(naked) upgrade_assignment_helper() {
    PROLOG_EPILOG(upgrade_assignment)
}

void __declspec(naked) upgrade_assignment_helper_andknuckles() {
    __asm {
        // Replace clobbered instructions
        pop edi
        pop esi

        push 0x00728411 // return address

        jmp upgrade_assignment_helper
    }
}

void Upgrade::Init() {
    // Call our custom function for setting upgrades, and NOP as appropriate
    WriteCall(reinterpret_cast<void*>(0x00717090), upgrade_assignment_helper); // Sonic
    WriteData<2>(reinterpret_cast<void*>(0x00717095), unsigned char{ 0x90 });

    WriteCall(reinterpret_cast<void*>(0x007175f0), upgrade_assignment_helper); // Shadow
    WriteData<2>(reinterpret_cast<void*>(0x007175f5), unsigned char{ 0x90 });

    WriteJump(reinterpret_cast<void*>(0x0072840c), upgrade_assignment_helper_andknuckles); // Knuckles needs a little help :)

    WriteCall(reinterpret_cast<void*>(0x00728855), upgrade_assignment_helper); // Rouge
    WriteData<2>(reinterpret_cast<void*>(0x0072885a), unsigned char{ 0x90 });

    WriteCall(reinterpret_cast<void*>(0x00740e47), upgrade_assignment_helper); // EWalker
    WriteData<2>(reinterpret_cast<void*>(0x00740e4c), unsigned char{ 0x90 });

    WriteCall(reinterpret_cast<void*>(0x007410a7), upgrade_assignment_helper); // TWalker
    WriteData<2>(reinterpret_cast<void*>(0x007410ac), unsigned char{ 0x90 });
}