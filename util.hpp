#pragma once

#include <cstdint>

#define naked __declspec(naked)

inline void SetImmediateJump(uintptr_t p, void *j, char opcode = 0xE9)
{
	*(char*)p = opcode;
	*(uintptr_t*)((char*)p+1) = (uintptr_t)j - ((uintptr_t)p + 5);
}

inline void SetImmediateJump(void *p, void *j, char opcode = 0xE9) { SetImmediateJump(reinterpret_cast<uintptr_t>(p), j, opcode); }

inline void SetImmediateCall(uintptr_t p, void *j) {SetImmediateJump(p, j, 0xE8);}
inline void SetImmediateCall(void* p, void *j) {SetImmediateJump(p, j, 0xE8);}