// XXL Inspector: Memory watcher and editor for Asterix XXL 1, 2 and OG
// By AdrienTD

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <io.h>
#include <cassert>
#include <cstdint>
//#include "inih/cpp/INIReader.h"
#include "util.hpp"

char title[] = "XXL QoL Patch";

char *exeep, oldepcode[5];
HINSTANCE exehi;
FILE *logfile = 0;

extern void PatchStart_XXL();

void SetMemProtection(void *mem, int flags)
{
	MEMORY_BASIC_INFORMATION mbi; DWORD unused;
	VirtualQuery(mem, &mbi, sizeof(mbi));
	VirtualProtect(mem, mbi.RegionSize, (mbi.Protect&0xFFFFFF00) | flags, &unused);
}

int VerifyVersion()
{
	// TODO
	return 1;
}

void ReadSettings()
{
	//INIReader ini("xxl_qol_settings.txt");
	// ...
}

void PatchStart()
{
	// Verify the version of the game.
	VerifyVersion();

	// Read the settings file (xxl_inspector_config.txt).
	ReadSettings();

	// MessageBox(0, "XXL QOL Patch enabled!", title, 64);
	PatchStart_XXL();

	// Restore entry point code.
	memcpy(exeep, oldepcode, 5);

	// Make the .text section back to non-writable for security reasons.
	SetMemProtection((void*)0x401000, PAGE_EXECUTE_READ);
}

naked void EntryPointInterception()
{
	__asm {
		call PatchStart
		mov eax, exeep
		jmp eax
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	char *mz, *pe;
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		// Make the .text section writable.
		SetMemProtection((void*)0x401000, PAGE_EXECUTE_READWRITE);

		// Find the entry point address in the PE header.
		exehi = GetModuleHandle(0);
		mz = (char*)exehi;
		if(*(uint16_t*)mz != 'ZM') return FALSE;
		pe = mz + *(uint32_t*)(mz+0x3C);
		if(*(uint32_t*)pe != 'EP') return FALSE;
		exeep = (char*)( *(uint32_t*)(pe+0x28) + *(uint32_t*)(pe+0x34) );

		// Save the first 5 bytes of the entry point code.
		memcpy(oldepcode, exeep, 5);

		// Put a jump to our function at the beginning of the entry point code.
		SetImmediateJump(exeep, EntryPointInterception);
	}
	return TRUE;
}
