#include "live_counter.h"
#include <DW/dwUtils.h>
#include <universal/com_tasks.h>
#include <universal/com_stringtable.h>
#include <DW/dwLogOn_pc.h>
#include <win32/win_shared.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>

const TaskDefinition task_downloadAllCounters[1] =
{
  {
    2uLL,
    "downloadAllCounters",
    0,
    (void(*)(TaskRecord*))LiveCounter_DownloadAllCountersComplete,
    (void(*)(TaskRecord*))LiveCounter_DownloadAllCountersFailure,
    NULL
  }
};

cmd_function_s LiveCounter_IncrementCounter_f_VAR;
cmd_function_s LiveCounter_Dump_f_VAR;

const dvar_t *counterUploadInterval;
const dvar_t *counterDownloadInterval;

bool s_countersInit;
bool s_countersReadyForUpload;
int s_nextCounterUploadTime;
int s_nextCounterDownloadTime;

bdCounterValue s_serverCounters[100];
bdCounterValue s_localCounters[100];
bdCounterValue s_uploadCounters[100];

bool s_countersRequested = true;
unsigned int s_serverCountersTotal = 100;



// *WARNING* One or more selections were skipped as they could not be interpreted as c data


TaskRecord *__cdecl LiveCounter_IncrementCounters(
                const TaskDefinition *definition,
                int controllerIndex,
                bdCounterValue *const counterIncrements,
                unsigned int numEntries)
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *v5; // eax
    bdReference<bdCommonAddr> v6; // [esp+1Ch] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-8h]
    bdCounter *counter; // [esp+24h] [ebp-4h]

    counter = dwGetCounter(controllerIndex);
    if ( !counter )
        return 0;
    task = TaskManager2_TaskGetInProgress(definition);
    if ( !task )
    {
        task = TaskManager2_CreateTask(definition, controllerIndex, 0, 0);
        v5 = (const bdReference<bdCommonAddr> *)bdCounter::incrementCounters(counter, (int)&v6, (int)counterIncrements, numEntries);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v5);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v6);
        s_numEntriesUploaded = numEntries;
        task->payload = &s_numEntriesUploaded;
        TaskManager2_StartTask(task);
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl LiveCounter_GetCounterTotals(
                const TaskDefinition *definition,
                int controllerIndex,
                bdCounterValue *results,
                unsigned int numCounterIDs)
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *CounterTotals; // eax
    bdReference<bdCommonAddr> v6; // [esp+1Ch] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-8h]
    bdCounter *counter; // [esp+24h] [ebp-4h]

    counter = dwGetCounter(controllerIndex);
    if ( !counter )
        return 0;
    task = TaskManager2_TaskGetInProgress(definition);
    if ( !task )
    {
        task = TaskManager2_CreateTask(definition, controllerIndex, 0, 0);
        CounterTotals = (const bdReference<bdCommonAddr> *)bdCounter::getCounterTotals(counter, (int)&v6, results, numCounterIDs);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, CounterTotals);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v6);
        TaskManager2_StartTask(task);
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl LiveCounter_UploadAllCounters(int controllerIndex)
{
#ifdef KISAK_LIVE_STUBS
    unsigned int v1; // eax
    signed int v3; // eax
    signed int j; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    unsigned int uploadCount; // [esp+Ch] [ebp-4h]

    uploadCount = 0;
    for ( i = 0; i < (int)s_serverCountersTotal; ++i )
    {
        if ( LODWORD(s_localCounters[i].m_counterValue) || HIDWORD(s_localCounters[i].m_counterValue) )
        {
            s_uploadCounters[uploadCount].m_counterID = s_localCounters[i].m_counterID;
            v1 = uploadCount;
            LODWORD(s_uploadCounters[v1].m_counterValue) = s_localCounters[i].m_counterValue;
            HIDWORD(s_uploadCounters[v1].m_counterValue) = HIDWORD(s_localCounters[i].m_counterValue);   // nx-port: was a store through the x86 address of the high half
            ++uploadCount;
        }
    }
    if ( !uploadCount )
        return 0;
    for ( j = 0; j < (int)s_serverCountersTotal; ++j )
    {
        v3 = j;
        LODWORD(s_localCounters[v3].m_counterValue) = 0;
        HIDWORD(s_localCounters[v3].m_counterValue) = 0;   // nx-port: was a store through the x86 address of the high half
    }
    return LiveCounter_IncrementCounters(task_uploadAllCounters, controllerIndex, s_uploadCounters, uploadCount);
#else
    return NULL;
#endif
}

void __cdecl LiveCounter_UploadAllCountersComplete()
{
    Com_Printf(16, "Upload global counters.\n");
}

void __cdecl LiveCounter_UploadAllCountersFailure(TaskRecord *task)
{
    unsigned __int64 v1; // kr00_8
    int v2; // edx
    int j; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int triedToUploadCount; // [esp+8h] [ebp-4h]

    triedToUploadCount = *(unsigned int *)task->payload;
    for ( i = 0; i < triedToUploadCount; ++i )
    {
        for ( j = 0; j < (int)s_serverCountersTotal; ++j )
        {
            if ( s_localCounters[j].m_counterID == s_uploadCounters[i].m_counterID )
            {
                v1 = __PAIR64__(HIDWORD(s_uploadCounters[i].m_counterValue), s_uploadCounters[i].m_counterValue)
                     + __PAIR64__(HIDWORD(s_localCounters[j].m_counterValue), s_localCounters[j].m_counterValue);
                v2 = j;
                LODWORD(s_localCounters[v2].m_counterValue) = LODWORD(s_uploadCounters[i].m_counterValue)
                                                                                                        + LODWORD(s_localCounters[j].m_counterValue);
                HIDWORD(s_localCounters[v2].m_counterValue) = HIDWORD(v1);   // nx-port: was a store through the x86 address of the high half
                break;
            }
        }
    }
    s_countersReadyForUpload = 1;
    s_nextCounterUploadTime = 60000 * counterUploadInterval->current.integer + Sys_Milliseconds();
    Com_PrintError(16, "Failed to upload global counters.\n");
}

TaskRecord *__cdecl LiveCounter_DownloadAllCounters(int controllerIndex)
{
    return LiveCounter_GetCounterTotals(
                     task_downloadAllCounters,
                     controllerIndex,
                     s_serverCounters,
                     s_serverCountersTotal);
}

void __cdecl LiveCounter_DownloadAllCountersComplete()
{
    Com_Printf(16, "Downloaded all global counters.\n");
}

void __cdecl LiveCounter_DownloadAllCountersFailure()
{
    Com_PrintError(16, "Failed to download all global counters.\n");
}

void __cdecl LiveCounter_IncrementCounterValueByName(const char *counterType, unsigned __int64 increment)
{
    int counterID; // [esp+0h] [ebp-4h]

    if ( !counterType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_counter.cpp",
                    196,
                    0,
                    "%s",
                    "counterType != NULL") )
    {
        __debugbreak();
    }
    if ( counterType )
    {
        counterID = LiveCounter_CounterStringToID(counterType);
        if ( counterID != -1 )
            LiveCounter_IncrementCounterValue(counterID, increment);
    }
}

void __cdecl LiveCounter_IncrementCounterValue(int counterID, __int64 increment)
{
    __int64 v2; // kr00_8
    int v3; // edx
    int i; // [esp+4h] [ebp-4h]

    if ( s_countersInit && increment )
    {
        if ( !counterID
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_counter.cpp", 218, 0, "%s", "counterID != 0") )
        {
            __debugbreak();
        }
        for ( i = 0; i < 1600; ++i )
        {
            if ( s_localCounters[i].m_counterID == counterID )
            {
                v2 = increment + __PAIR64__(HIDWORD(s_localCounters[i].m_counterValue), s_localCounters[i].m_counterValue);
                v3 = i;
                LODWORD(s_localCounters[v3].m_counterValue) = increment + LODWORD(s_localCounters[i].m_counterValue);
                HIDWORD(s_localCounters[v3].m_counterValue) = HIDWORD(v2);   // nx-port: was a store through the x86 address of the high half
                if ( !s_countersReadyForUpload )
                {
                    s_countersReadyForUpload = 1;
                    s_nextCounterUploadTime = 60000 * counterUploadInterval->current.integer + Sys_Milliseconds();
                }
                return;
            }
        }
    }
}

int __cdecl LiveCounter_GetCounterTotalValue(int counterID)
{
    if ( !s_countersInit )
        return 0;
    s_countersRequested = 1;
    return s_serverCounters[counterID].m_counterValue;
}

int __cdecl LiveCounter_CounterStringToID(const char *counterType)
{
    const char *counterID; // [esp+10h] [ebp-8h]
    const StringTable *table; // [esp+14h] [ebp-4h] BYREF

    if ( !counterType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_counter.cpp",
                    250,
                    0,
                    "%s",
                    "counterType != NULL") )
    {
        __debugbreak();
    }
    if ( !counterType )
        return -1;
    StringTable_GetAsset("mp/globalcounters.csv", (XAssetHeader *)&table);
    if ( table )
    {
        counterID = StringTable_Lookup(table, 1, counterType, 0);
        if ( strlen(counterID) )
            return atoi(counterID);
        else
            return -1;
    }
    else
    {
        Com_PrintError(
            16,
            "Could not find global counter value as string table '%s' could not be found.\n",
            "mp/globalcounters.csv");
        return -1;
    }
}

void __cdecl LiveCounter_SetupCounters()
{
    int v0; // ecx
    int v1; // edx
    int v2; // eax
    unsigned int counterID; // [esp+0h] [ebp-14h]
    const char *counter; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    const StringTable *table; // [esp+Ch] [ebp-8h] BYREF
    int rowCount; // [esp+10h] [ebp-4h]

    StringTable_GetAsset("mp/globalcounters.csv", (XAssetHeader *)&table);
    if ( !table )
    {
        Com_PrintError(
            16,
            "Could not find global counter value as string table '%s' could not be found.\n",
            "mp/globalcounters.csv");
        s_countersInit = 0;
    }
    rowCount = StringTable_RowCount(table);
    if ( !rowCount || rowCount >= (int)s_serverCountersTotal )
    {
        Com_PrintError(16, "Global counter string table '%s' had no values.\n", "mp/globalcounters.csv");
        s_countersInit = 0;
    }
    for ( i = 0; i < rowCount; ++i )
    {
        counter = StringTable_GetColumnValueForRow(table, i, 0);
        counterID = atoi(counter);
        if ( counterID )
        {
            s_localCounters[i].m_counterID = counterID;
            s_serverCounters[i].m_counterID = counterID;
            v0 = i;
            LODWORD(s_localCounters[v0].m_counterValue) = 0;
            HIDWORD(s_localCounters[v0].m_counterValue) = 0;   // nx-port: was a store through the x86 address of the high half
            v1 = i;
            LODWORD(s_uploadCounters[v1].m_counterValue) = 0;
            HIDWORD(s_uploadCounters[v1].m_counterValue) = 0;   // nx-port: was a store through the x86 address of the high half
            v2 = i;
            LODWORD(s_serverCounters[v2].m_counterValue) = -1;
            HIDWORD(s_serverCounters[v2].m_counterValue) = -1;   // nx-port: was a store through the x86 address of the high half
        }
        else
        {
            Com_PrintError(
                16,
                "Global counter string table '%s' has an invalid value: %s.\n",
                "mp/globalcounters.csv",
                counter);
        }
    }
    s_countersInit = 1;
}

void __cdecl LiveCounter_Update(int controllerIndex)
{
#ifdef KISAK_LIVE
    if ( s_countersInit && dwGetLogOnStatus(controllerIndex) == 4 )
    {
        if ( (int)Sys_Milliseconds() > s_nextCounterDownloadTime && s_countersRequested )
        {
            s_countersRequested = 0;
            LiveCounter_DownloadAllCounters(controllerIndex);
            s_nextCounterDownloadTime = 60000 * counterDownloadInterval->current.integer + Sys_Milliseconds();
        }
        if ( s_countersReadyForUpload && (int)Sys_Milliseconds() > s_nextCounterUploadTime )
        {
            if ( LiveCounter_UploadAllCounters(controllerIndex) )
                s_countersReadyForUpload = 0;
            else
                s_nextCounterUploadTime = 60000 * counterUploadInterval->current.integer + Sys_Milliseconds();
        }
    }
#endif
}

void __cdecl LiveCounter_Init()
{
    Cmd_AddCommandInternal("incrementcounter", LiveCounter_IncrementCounter_f, &LiveCounter_IncrementCounter_f_VAR);
    Cmd_AddCommandInternal("dumpLiveCounters", LiveCounter_Dump_f, &LiveCounter_Dump_f_VAR);
    counterUploadInterval = _Dvar_RegisterInt(
                                                        "counterUploadInterval",
                                                        15,
                                                        1,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Number of minutes before all the global counters are uploaded");
    counterDownloadInterval = _Dvar_RegisterInt(
                                                            "counterDownloadInterval",
                                                            15,
                                                            1,
                                                            0x7FFFFFFF,
                                                            0,
                                                            "Number of minutes before all the global counters are downloaded");
}

void __cdecl LiveCounter_IncrementCounter_f()
{
    const char *v0; // eax
    const char *counterType; // [esp+18h] [ebp-Ch]
    int counterId; // [esp+1Ch] [ebp-8h]
    int increment; // [esp+20h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        counterType = Cmd_Argv(1);
        v0 = Cmd_Argv(2);
        increment = atoi(v0);
        counterId = LiveCounter_CounterStringToID(counterType);
        if ( counterId == -1 )
            Com_PrintError(16, "Invalid counter string id: %s\n", counterType);
        else
            LiveCounter_IncrementCounterValue(counterId, increment);
    }
    else
    {
        Com_PrintError(16, "Invalid param count. usage: incrementCounter( <counter id>, <increment value> )\n");
    }
}

void __cdecl LiveCounter_Dump_f()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 100; ++i )
    {
        Com_Printf(0, "%d: %llu\n", s_serverCounters[i].m_counterID, s_serverCounters[i].m_counterValue);
    }
}

