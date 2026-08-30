#include "tl_system.h"
#include <stdio.h>


#include <cstring>
#include <corecrt_malloc.h>
#include <Windows.h>
#include <Psapi.h>

tlSystemCallbacks tlCurSystemCallbacks;
char tlHostPrefix[256];
int tlMemAllocCounter;
int tlScratchPadRefCount;
bool tlScratchpadLocked;
void *tlScratchPadPtr;

void *tlStackBegin;
void *tlStackEnd;

float tlPcTicksPerMS = 1.0f;

void __cdecl tlSetSystemCallbacks(const tlSystemCallbacks *Callbacks)
{
  memcpy(&tlCurSystemCallbacks, Callbacks, sizeof(tlCurSystemCallbacks));
}

void __cdecl tlSetFileServerRootPC(const char *Path)
{
  strcpy(tlHostPrefix, Path);
}

void __cdecl tlMemFree(void *Ptr)
{
  --tlMemAllocCounter;
  if ( tlCurSystemCallbacks.MemFree )
    tlCurSystemCallbacks.MemFree(Ptr);
  else
    _aligned_free(Ptr);
}

void __cdecl tlReleaseFile(tlFileBuf *File)
{
  unsigned __int8 *Buf; // eax

  if ( tlCurSystemCallbacks.ReleaseFile )
  {
    tlCurSystemCallbacks.ReleaseFile(File);
  }
  else
  {
    Buf = File->Buf;
    --tlMemAllocCounter;
    if ( tlCurSystemCallbacks.MemFree )
      tlCurSystemCallbacks.MemFree(Buf);
    else
      _aligned_free(Buf);
    File->Buf = 0;
    File->Size = 0;
    File->UserData = 0;
  }
}

_LARGE_INTEGER __cdecl tlPcGetTick()
{
  _LARGE_INTEGER li; // [esp+0h] [ebp-8h] BYREF

  QueryPerformanceCounter(&li);
  return li;
}

void __cdecl tlPrint(const char *txt)
{
  OutputDebugStringA(txt);
}

int __cdecl tlGetVersion()
{
  return 66560;
}

void __cdecl tlStackRangeInit()
{
  tlStackBegin = 0;
  tlStackEnd = 0;
}

char __cdecl tlFatalHandler(const char *Msg)
{
  if ( tlCurSystemCallbacks.CriticalError )
  {
    tlCurSystemCallbacks.CriticalError(Msg);
    return 0;
  }
  else
  {
    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(Msg);
    OutputDebugStringA("\n");
    return 1;
  }
}

void __cdecl tlDebugPrint(const char *txt)
{
  if ( tlCurSystemCallbacks.DebugPrint )
    tlCurSystemCallbacks.DebugPrint(txt);
  else
    OutputDebugStringA(txt);
}

void __cdecl tlVPrintf(const char *Format, char *args)
{
  char Work[512]; // [esp+0h] [ebp-204h] BYREF

#ifdef KISAK_NX
  vsprintf(Work, Format, *(va_list *)args); // callers pass a real va_list through char*
#else
  vsprintf(Work, Format, args);
#endif
  if ( tlCurSystemCallbacks.DebugPrint )
    tlCurSystemCallbacks.DebugPrint(Work);
  else
    OutputDebugStringA(Work);
}

void tlPrintf(const char *Format, ...)
{
  char Work[512]; // [esp+0h] [ebp-204h] BYREF
  va_list ap; // [esp+210h] [ebp+Ch] BYREF

  va_start(ap, Format);
  vsprintf(Work, Format, ap);
  if ( tlCurSystemCallbacks.DebugPrint )
    tlCurSystemCallbacks.DebugPrint(Work);
  else
    OutputDebugStringA(Work);
}

char __cdecl _tlAssert(const char *file, int line, const char *expr, const char *desc)
{
  char Buf[256]; // [esp+0h] [ebp-104h] BYREF

  _snprintf(Buf, 0x100u, "ASSERT in %s(%d):\n\"%s\" - %s", file, line, expr, desc);
  Buf[255] = 0;
  if ( tlCurSystemCallbacks.CriticalError )
  {
    tlCurSystemCallbacks.CriticalError(Buf);
    return 0;
  }
  else
  {
    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(Buf);
    OutputDebugStringA("\n");
    return 1;
  }
}

void tlFatal(const char *Format, ...)
{
  char Work[512]; // [esp+0h] [ebp-204h] BYREF
  va_list ap; // [esp+210h] [ebp+Ch] BYREF

  va_start(ap, Format);
  vsprintf(Work, Format, ap);
  if ( tlCurSystemCallbacks.CriticalError )
  {
    tlCurSystemCallbacks.CriticalError(Work);
  }
  else
  {
    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(Work);
    OutputDebugStringA("\n");
    __debugbreak();
  }
}

void *__cdecl tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags)
{
  unsigned int v3; // edi
  void *v4; // eax
  void *v5; // esi

  v3 = Align;
  if ( !Align && (Size & 0xF) == 0 )
    v3 = 0;
  ++tlMemAllocCounter;
  if ( tlCurSystemCallbacks.MemAlloc )
    v4 = tlCurSystemCallbacks.MemAlloc(Size, v3, Flags);
  else
    v4 = _aligned_malloc(Size, v3);
  v5 = v4;
  if ( (Flags & 2) == 0 && !v4 && Size )
    tlFatal("Memory allocation failed. %d bytes, %d align", Size, v3);
  return v5;
}

void *__cdecl tlMemRealloc(void *Ptr, unsigned int Size, unsigned int Align, unsigned int Flags)
{
  unsigned int v4; // eax
  void *v5; // eax
  void *v6; // esi

  v4 = Align;
  if ( !Align && (Size & 0xF) == 0 )
    v4 = 0;
  if ( tlCurSystemCallbacks.MemRealloc )
    v5 = tlCurSystemCallbacks.MemRealloc(Ptr, Size, v4, Flags);
  else
    v5 = _aligned_realloc(Ptr, Size, v4);
  v6 = v5;
  if ( (Flags & 2) == 0 && !v5 && Size )
    tlFatal("Memory reallocation failed.");
  return v6;
}

unsigned int __cdecl tlGetFreeMemory()
{
  unsigned int v0; // edi
  unsigned int dwTotalPhys; // ebx
  DWORD CurrentProcessId; // eax
  HANDLE v3; // esi
  _PROCESS_MEMORY_COUNTERS pmc; // [esp+Ch] [ebp-48h] BYREF
  _MEMORYSTATUS stat; // [esp+34h] [ebp-20h] BYREF

  v0 = 0;
  GlobalMemoryStatus(&stat);
  dwTotalPhys = stat.dwTotalPhys;
  GetCurrentProcess();
  CurrentProcessId = GetCurrentProcessId();
  v3 = OpenProcess(0x410u, 0, CurrentProcessId);
  if ( GetProcessMemoryInfo(v3, &pmc, 40) )
    v0 = dwTotalPhys - pmc.PeakWorkingSetSize;
  CloseHandle(v3);
  return v0;
}

void *__cdecl tlScratchPadInit()
{
  void *v2; // eax
  void *v3; // esi

  if ( tlScratchPadRefCount++ )
    return tlScratchPadPtr;
  ++tlMemAllocCounter;
  if ( tlCurSystemCallbacks.MemAlloc )
    v2 = tlCurSystemCallbacks.MemAlloc(0x4000u, 0x10u, 0);
  else
    v2 = _aligned_malloc(0x4000u, 0x10u);
  v3 = v2;
  if ( !v2 )
    tlFatal("Memory allocation failed. %d bytes, %d align", 0x4000, 16);
  tlScratchPadPtr = v3;
  return v3;
}

void __cdecl tlScratchPadReset()
{
  if ( tlScratchPadRefCount < 1
    && _tlAssert("source/tl_system.cpp", 830, "tlScratchPadRefCount >= 1", "Scratchpad reset underflow.") )
  {
    __debugbreak();
  }
  if ( !--tlScratchPadRefCount )
  {
    --tlMemAllocCounter;
    if ( tlCurSystemCallbacks.MemFree )
      tlCurSystemCallbacks.MemFree(tlScratchPadPtr);
    else
      _aligned_free(tlScratchPadPtr);
    tlScratchPadPtr = 0;
  }
}

void tlWarning(const char *Format, ...)
{
  char Work[512]; // [esp+0h] [ebp-204h] BYREF
  va_list ap; // [esp+210h] [ebp+Ch] BYREF

  va_start(ap, Format);
  vsprintf(Work, Format, ap);
  if ( tlCurSystemCallbacks.Warning )
    tlCurSystemCallbacks.Warning(Work);
  else
    tlPrintf("%s", Work);
}

bool __cdecl tlReadFile(const char *FileName, tlFileBuf *File, unsigned int Align, unsigned int Flags)
{
  int v5; // eax
  char v6; // cl
  unsigned int v7; // eax
  char *v8; // edi
  char *i; // eax
  HANDLE FileA; // eax
  void *v12; // esi
  unsigned int FileSize; // eax
  unsigned __int8 *v14; // eax
  unsigned int Size; // [esp-Ch] [ebp-21Ch]
  DWORD bytesRead; // [esp+8h] [ebp-208h] BYREF
  char Work[512]; // [esp+Ch] [ebp-204h] BYREF

  if ( tlCurSystemCallbacks.ReadFile )
    return tlCurSystemCallbacks.ReadFile(FileName, File, Align, Flags);
  if ( !strncmp(FileName, tlHostPrefix, strlen(tlHostPrefix)) )
  {
    Work[0] = 0;
  }
  else
  {
    v5 = 0;
    do
    {
      v6 = tlHostPrefix[v5];
      Work[v5++] = v6;
    }
    while ( v6 );
  }
  v7 = strlen(FileName) + 1;
  v8 = (char *)&bytesRead + 3;
  while ( *++v8 )
    ;
  memcpy(v8, FileName, v7);
  for ( i = Work; *i; ++i )
  {
    if ( *i == 47 )
      *i = 92;
  }
  FileA = CreateFileA(Work, 0x80000000, 1u, 0, 3u, 0x8000080u, 0);
  v12 = FileA;
  if ( FileA == (HANDLE)-1 )
    return 0;
  FileSize = GetFileSize(FileA, 0);
  File->Size = FileSize;
  v14 = (unsigned __int8 *)tlMemAlloc(FileSize, Align, Flags);
  Size = File->Size;
  File->Buf = v14;
  ReadFile(v12, v14, Size, &bytesRead, 0);
  CloseHandle(v12);
  return 1;
}

