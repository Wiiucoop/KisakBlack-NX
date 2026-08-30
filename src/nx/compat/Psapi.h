// Psapi.h -- process memory statistics stub (tl_system.cpp).
#ifndef NX_COMPAT_PSAPI_H
#define NX_COMPAT_PSAPI_H

#include "windows.h"

typedef struct _PROCESS_MEMORY_COUNTERS {
    DWORD  cb;
    DWORD  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS, *PPROCESS_MEMORY_COUNTERS;

BOOL GetProcessMemoryInfo(HANDLE process, PPROCESS_MEMORY_COUNTERS counters, DWORD cb);

#endif // NX_COMPAT_PSAPI_H
