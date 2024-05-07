#include "util.hpp"
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include "inih/cpp/INIReader.h"
#include "kal.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// #define DIRECTINPUT_VERSION 0x0800
// #include <dinput.h>
#include <algorithm>

// New Widescreen Fix
namespace Fix_WidescreenFOV {
    struct CStruct2 {
        uint32_t unk0, unk1, unk2, width, height;
    };

    struct CRendThing {
        uint32_t unk0; CStruct2 *viewport; uint32_t unk2, unk3;
        uint32_t unk4, unk5, unk6, unk7;
        uint32_t unk8, unk9; float oldFOV, oldRatio;

    };

    void naked __cdecl P_58FFB0(uint32_t unk, float *ss)
    {
        __asm {
            mov eax, 0x58FFB0
            jmp eax
        }
    }

    void __cdecl CalcProjCamera(CRendThing *self, float newFOV, uint8_t flag)
    {
        if(flag == 0) {
            if(fabs(self->oldFOV - newFOV) < 0.003f)
                return;
        }
        if(self->viewport && self->unk2) {
            static const float deg_to_rad = (float)M_PI / 180.0f;

            float ratio = (float)self->viewport->width / (float)self->viewport->height;
            self->oldFOV = newFOV;
            self->oldRatio = 1.0f/ratio;
            //self->oldRatio = 0.0f;

            float ss[2];
            ss[1] = tanf(0.5f * newFOV * deg_to_rad) * 0.75f; // *0.75f to get same ys from 4:3 res
            ss[0] = ratio * ss[1];
            P_58FFB0(self->unk4, ss);
        }
    }

    void naked P_CalcProjCamera()
    {
        __asm {
            push [esp+8]
            push [esp+8]
            push ecx
            call CalcProjCamera
            add esp, 12
            ret 8
        }
    }

    void apply_fix() {
        SetImmediateJump(0x00414B70, P_CalcProjCamera);
    }
};

// Display all resolutions in the Settings
namespace Fix_AllResolutionsInSettings {
    void apply_fix() {
        memset((void*)0x52830D, 0x90, 2);
    }
}

// Mouse input
namespace Fix_ForceMouseInput {
    // set mouse enabled boolean to 1 just after being deserialized in CKInput
    void naked set_mouse_support_to_1() {
        __asm {
            lea eax, [ebp+0x11]
            push 1
            push eax
            mov ecx, ebx
            call dword ptr [edx+0x14]
            mov al, 1
            mov [ebp+0x11], al
            mov eax, 0x4365C1
            jmp eax
        }
    }

    void apply_fix() {
        SetImmediateJump(0x4365B6, set_mouse_support_to_1);
    }
}

// // Gamepad Camera Control Fix
// namespace Fix_GamepadCameraAxis_Combo {
//     void naked cam_axis_y() {
//         __asm {
//             mov eax, [esp+4+0x18] // rglSlider[0]
//             add eax, [esp+4+0x08] // lZ
//             add eax, [esp+4+0x10] // lRy
//             test eax, eax
//             mov ecx, 0x432D11
//             jmp ecx
//         }
//     }

//     void naked cam_axis_x() {
//         __asm {
//             mov eax, [esp+4+0x14] // lRz
//             add eax, [esp+4+0x0C] // lRx
//             test eax, eax
//             mov ecx, 0x432D25
//             jmp ecx
//         }
//     }

//     void apply_fix() {
//         SetImmediateJump(0x432D0B, cam_axis_y);
//         SetImmediateJump(0x432D1F, cam_axis_x);
//     }
// }

// Gamepad Camera Control Fix
namespace Fix_GamepadCameraAxis {
    static const char * const diAxisNames[8] = {"x","y","z","rx","ry","rz","slider0","slider1"};
    bool rightStickX[8], rightStickY[8];

    void readAxesChoice(const std::string &inp, bool *out) {
        static const char * const delimiter = " \t.,;:-/+*";
        std::string line(inp);
        char *lineptr = (char*)line.data();
        char *token = strtok(lineptr, delimiter);
        while(token) {
            auto it = std::find_if(std::begin(diAxisNames), std::end(diAxisNames), [token](const char *a) {return stricmp(a, token) == 0;});
            if(it != std::end(diAxisNames)) {
                out[it - std::begin(diAxisNames)] = true;
            }
            token = strtok(nullptr, delimiter);
        }

        // // debug
        // char msg[9];
        // for(int i = 0; i < 8; i++)
        //     msg[i] = out[i] ? '1' : '0';
        // msg[8] = 0;
        // MessageBox(NULL, msg, "dbg", 64);
    }

    int32_t __stdcall getAxisValue(int32_t *joy, bool *input) {
        int32_t res = 0;
        for(int i = 0; i < 8; i++)
            if(input[i])
                res += joy[i];
        return res;
    }

    void naked cam_axis_y() {
        __asm {
            lea eax, [esp+4]
            push edx
            push offset rightStickY
            push eax
            call getAxisValue
            pop edx

            test eax, eax
            mov ecx, 0x432D11
            jmp ecx
        }
    }

    void naked cam_axis_x() {
        __asm {
            lea eax, [esp+4]
            push edx
            push offset rightStickX
            push eax
            call getAxisValue
            pop edx

            test eax, eax
            mov ecx, 0x432D25
            jmp ecx
        }
    }

    void apply_fix() {
        SetImmediateJump(0x432D0B, cam_axis_y);
        SetImmediateJump(0x432D1F, cam_axis_x);
    }
}

// Restore all languages in Polish CD Projekt Patch
namespace Fix_RestoreAllLanguages {
    void apply_fix() {
        memcpy((void*)0x4090C8, "\x8B\x74\x24\x10", 4);
    }
}

// Windowed mode
namespace Fix_WindowedMode {
    int32_t initialWidth = 800, initialHeight = 600;

    void apply_fix() {
        // NOP-out instruction that sets fullscreen flag to 1 and keep it 0
        memset((void*)0x47A5C3, 0x90, 5);

        *(int32_t*)0x47A5F5 = initialWidth;
        *(int32_t*)0x47A5FC = initialHeight;
    }
}

// Memory limit increase
namespace Fix_ChangeMemoryLimit {
    int32_t heapSize = 34816000;

    void apply_fix() {
        *(int32_t*)0x47A206 = heapSize;
    }
}

// Infinite potion
namespace Cheat_InfinitePotion {
    // 005040F0 - D9 80 9C000000  - fld dword ptr [eax+0000009C]
    // 00504137 - 89 90 9C000000  - mov [eax+0000009C],edx
    // 0051BAA7 - D9 80 9C000000  - fld dword ptr [eax+0000009C]
    // 0047DE91 - D9 80 9C000000  - fld dword ptr [eax+0000009C]
    // 0049A371 - D9 80 9C000000  - fld dword ptr [eax+0000009C]

    float potionTimerValue = 300.0f;

    void naked reset_potion_timer() {
        __asm {
            push ecx
            mov eax, [esp+8]
            push ebx

            mov edx, [ecx+0x40]
            test edx, edx
            jz noplayer
            mov ebx, potionTimerValue
            mov [edx+0x9C], ebx
        noplayer:
            mov ebx, 0x00504086
            jmp ebx
        }
    }

    void naked reset_potion_after_fx() {
        __asm {
            mov eax, 0x6621F4
            mov eax, [eax] // Yellow pages
            mov eax, [eax+0x8C]
            mov eax, [eax+0x40]
            mov edi, potionTimerValue
            mov [eax+0x9C], edi

            pop edi
            pop esi
            retn 4
        }
    }

    void apply_fix() {
        //SetImmediateJump(0x00504080, reset_potion_timer);
        memset((void*)0x4FEDA4, 0x90, 0x16); // Disable potion event

        ////memset((void*)0x47DEAA, 0x90, 2); // FX
        //memset((void*)0x51BABA, 0x90, 6); // UI
        //memset((void*)0x49A386, 0x90, 5); // Invincibility (except breaking metal crates!)

        SetImmediateJump(0x48445B, reset_potion_after_fx);
    }
}

// Changes the starting level number (only works with modding patch?)
namespace Cheat_ChangeInitialLevel {
    int32_t initialLevel = 0;
    void naked fix_lvl_number() {
        __asm {
            //mov eax, initialLevel
            //mov [esi+8], eax
            mov eax, initialLevel
            mov [esi+8], eax
            mov eax, 0xFFFFFFFF
            mov [esi+0x60], eax
            pop esi
            retn 8
        }
    }
    void apply_fix() {
        SetImmediateJump(0x40739B, fix_lvl_number);
    }
}

// Force speech language, independently from text language
namespace Fix_ForceSpeechLanguage {
    int32_t speechLanguage = 0;
    void naked speech_filepath_format_hook() {
        __asm {
            mov eax, speechLanguage
            mov [esp+0x14], eax
            mov [esp+0x18], eax
            mov eax, 0x6096C1 // the real sprintf
            jmp eax
        }
    }
    void apply_fix() {
        SetImmediateCall(0x0043CCE4, speech_filepath_format_hook);
    }
}

void PatchStart_XXL()
{
	/*
	SetImmediateJump((void*)0x47A274, (uint)jmp_47A274);

	gameStartInfo = (SGameStartInfo*)0x665310;
	*(uint*)(0x479E8E) = (uint)hookWndProc;

	// Remove cursor from window class
	memcpy((void*)0x479EAE, "\x83\xC4\x08\x31\xC0\x90", 6); // add esp, 8 \ xor eax, eax
    */

	INIReader ini("xxl_qol_settings.ini");
    static const std::string section = "XXL";

    if(ini.GetBoolean(section, "windowed_mode", false)) {
        Fix_WindowedMode::initialWidth = ini.GetInteger(section, "window_width", 800);
        Fix_WindowedMode::initialHeight = ini.GetInteger(section, "window_height", 600);
        Fix_WindowedMode::apply_fix();
    }

    // FOV Fix
    if(ini.GetBoolean(section, "enable_wide_fov", true))
        Fix_WidescreenFOV::apply_fix();
    if(ini.GetBoolean(section, "allow_all_resolutions_in_settings", true))
        Fix_AllResolutionsInSettings::apply_fix();

    // Gamepad camera analog stick fix
    if(ini.GetBoolean(section, "gamepad_camera_axis_fix", true)) {
        Fix_GamepadCameraAxis::readAxesChoice(ini.GetString(section, "rightstick_horizontal", "rx+rz"), Fix_GamepadCameraAxis::rightStickX);
        Fix_GamepadCameraAxis::readAxesChoice(ini.GetString(section, "rightstick_vertical", "z+ry+slider0"), Fix_GamepadCameraAxis::rightStickY);
        Fix_GamepadCameraAxis::apply_fix();
    }
    // Force enable mouse input
    if(ini.GetBoolean(section, "enable_mouse_input", true))
        Fix_ForceMouseInput::apply_fix();

    if(ini.GetBoolean(section, "polish_multilang_fix", true))
        Fix_RestoreAllLanguages::apply_fix();

    if(auto val = ini.GetInteger(section, "memory_heap_size", 0)) {
        Fix_ChangeMemoryLimit::heapSize = val;
        Fix_ChangeMemoryLimit::apply_fix();
    }

    if(ini.GetBoolean(section, "infinite_potion", false)) {
        Cheat_InfinitePotion::apply_fix();
    }

    Cheat_ChangeInitialLevel::initialLevel = ini.GetInteger(section, "initial_level", -1);
    if(Cheat_ChangeInitialLevel::initialLevel != -1)
        Cheat_ChangeInitialLevel::apply_fix();

    Fix_ForceSpeechLanguage::speechLanguage = ini.GetInteger(section, "force_speech_language", -1);
    if(Fix_ForceSpeechLanguage::speechLanguage != -1)
        Fix_ForceSpeechLanguage::apply_fix();
}
