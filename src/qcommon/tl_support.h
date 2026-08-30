#pragma once

void __cdecl Sys_SetupTLCallbacks(int hunkMemSize);
void __cdecl TL_Warning(const char *Text);
bool __cdecl TL_ReadFile();
void __cdecl TL_ReleaseFile();
void __cdecl TL_DebugPrint(char *Text);
void *__cdecl TL_MemAlloc(unsigned int Size, unsigned int Align);
void __cdecl TL_MemFree(void *Ptr);
void __cdecl TL_CriticalError(const char *msg);
void TL_DebugDumpHunk(const char *where);