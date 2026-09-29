#pragma once
#include "demo_common.h"
#include <tl/jobqueue/jobqueue_all.h>
#include <qcommon/sv_msg_write_mp.h>

struct uploadSendInfo // sizeof=0x8
{                                       // XREF: uploadStreamData_t/r
    int ms;                             // XREF: Demo_RecordSentPacket(uint)+46/w
                                        // Demo_ThrottleStream(uint)+189/r ...
    unsigned int size;                  // XREF: Demo_RecordSentPacket(uint)+53/w
                                        // Demo_ThrottleStream(uint)+20A/r ...
};

struct uploadStreamData_t // sizeof=0x6033C
{                                       // XREF: .data:s_uploadStreamData/r
    int controllerIndex;                // XREF: Demo_StartStreaming(int)+21/w
                                        // Demo_SaveToStreamBuffer(void const *,int)+CA/r ...
    unsigned __int8 buffer[393216];     // XREF: Demo_SaveToStreamBuffer(void const *,int)+73/o
                                        // Demo_SaveToStreamBuffer(void const *,int)+128/o ...
    volatile int writer;                // XREF: Demo_StartStreaming(int)+A/w
                                        // Demo_GetCurrUsedBuffer(void)+8/r ...
    volatile int reader;                // XREF: Demo_StartStreaming(int)+14/w
                                        // Demo_GetCurrUsedBuffer(void)+3/r ...
    volatile bool active;               // XREF: Demo_StartStreaming(int)+3/w
                                        // Demo_StopStreaming(void)+3/w ...
    // padding byte
    // padding byte
    // padding byte
    volatile int lastWrite;             // XREF: Demo_SaveToStreamBuffer(void const *,int)+1B4/w
    bool overflow;                      // XREF: Demo_StartStreaming(int)+26/w
                                        // Demo_SaveToStreamBuffer(void const *,int)+1DA/r ...
    // padding byte
    // padding byte
    // padding byte
    uploadSendInfo sendHistory[100];    // XREF: Demo_StartStreaming(int)+45/o
                                        // Demo_RecordSentPacket(uint)+46/w ...
    int sendHistoryIndex;               // XREF: Demo_StartStreaming(int)+52/w
                                        // Demo_RecordSentPacket(uint)+4/r ...
};

struct uploadStreamDebugData_t // sizeof=0x8
{                                       // XREF: uploadStreamHistory_t/r
    bool isDataAdded;                   // XREF: Demo_AppendToStreamingHistory(int,bool)+2F/r
                                        // Demo_AppendToStreamingHistory(int,bool)+50/w
    // padding byte
    // padding byte
    // padding byte
    int value;                          // XREF: Demo_AppendToStreamingHistory(int,bool)+3D/r
                                        // Demo_AppendToStreamingHistory(int,bool)+59/w
};

struct uploadStreamHistory_t // sizeof=0x1E8
{                                       // XREF: .data:s_uploadStreamUsage/r
    uploadStreamDebugData_t history[60];
                                        // XREF: Demo_AppendToStreamingHistory(int,bool)+2F/r
                                        // Demo_AppendToStreamingHistory(int,bool)+3D/r ...
    bool changed;                       // XREF: Demo_StartStreaming(int)+2D/w
                                        // Demo_AppendToStreamingHistory(int,bool)+5E/w
    // padding byte
    // padding byte
    // padding byte
    int highwater;                      // XREF: Demo_StartStreaming(int)+34/w
                                        // Demo_AppendToStreamingHistory(int,bool)+6D/r ...
};

struct __declspec(align(4)) demoSaveCmd // sizeof=0xC
{                                       // XREF: ?Demo_Save@@YAXPAEH_N@Z/r
    unsigned __int8 *data;              // XREF: Demo_Save(uchar *,int,bool)+16/w
    int size;                           // XREF: Demo_Save(uchar *,int,bool)+1C/w
    bool writeInfoFile;                 // XREF: Demo_Save(uchar *,int,bool)+22/w
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl Demo_StartSaveProcess();
void __cdecl Demo_Save(unsigned __int8 *data, int size, bool writeInfoFile);
void __cdecl Demo_SaveInternal(unsigned __int8 *data, int size, bool writeInfoFile);
void __cdecl Demo_StartStreaming(int controllerIndex);
void __cdecl Demo_StopStreaming();
bool __cdecl Demo_AreDifferencesValid(int diffBefore, int diffAfter);
void __cdecl Demo_AppendToStreamingHistory(int val, bool isAdded);
int __cdecl Demo_GetCurrUsedBuffer();
int __cdecl Demo_CurrAvailableBuffer();
void __cdecl Demo_SaveToStreamBuffer(unsigned __int8 *data, unsigned int dataSize);
void __cdecl Demo_RecordSentPacket(unsigned int size);
bool __cdecl Demo_ShouldThrottle();
unsigned int __cdecl Demo_ThrottleStream(unsigned int dataSize);
int __cdecl Demo_WriteToStream(unsigned __int8 *data, unsigned int dataSize);
int __cdecl Demo_SaveCallback(jqBatch *batch);
unsigned __int8 *__cdecl Demo_GetActiveMemoryBuffer();
unsigned __int8 *__cdecl Demo_SwapActiveMemoryBuffer();
void __cdecl Demo_SetIntermissionPoint(float *origin, float *angles);
void __cdecl Demo_GetIntermissionPointOrigin(float (*origin)[3]);
void __cdecl Demo_GetIntermissionPointAngles(float (*angles)[3]);
void __cdecl Demo_RecordPlayerSessionState(int clientNum, int state);
bool __cdecl Demo_IsPlayerSpectating(int clientNum);
bool __cdecl Demo_IsInFinalKillcam();
bool __cdecl Demo_IsPlayerDead(int clientNum);
void __cdecl Demo_ClientConnected(int clientNum);
int __cdecl Demo_GetDemoClientIndex();
void __cdecl Demo_SetDemoClientIndex(int index);
bool __cdecl Demo_ShouldWriteToDemoFile();
bool __cdecl Demo_ShouldBuildDemoSnapshot();
void __cdecl Demo_WriteHeader(msg_t *msg, bool isDemoClip);
void __cdecl Demo_BuildDemoSnapshot();
void __cdecl Demo_WriteSnapshotStartData(
                int localClientNum,
                msg_t *msg,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                unsigned __int8 lastFrame,
                unsigned __int8 snapFlags);
void __cdecl Demo_WriteGamestateToBuffer(int clientNum, int demoMsgType, msg_t *msg, int msgSequence);
bool __cdecl Demo_ShouldFilterServerCommand(int clientNum, const char *cmd);
void __cdecl Demo_WriteServerCommands(msg_t *msg);
void __cdecl Demo_WriteMatchState(msg_t *msg);
void __cdecl Demo_WritePlayerStates(msg_t *msg);
void __cdecl Demo_WritePacketEntities(msg_t *msg);
void __cdecl Demo_WritePacketClients(msg_t *msg);
void __cdecl Demo_PopulateStaticInfoData(demoMetaInfo *info);
unsigned int __cdecl Demo_WriteToBuffer(
                unsigned __int8 *to,
                unsigned __int8 *from,
                unsigned int size,
                int bytesWritten,
                int maxBufferSize);
unsigned int __cdecl Demo_WriteInfoDataToBuffer(demoMetaInfo *info, unsigned __int8 *buffer, int maxBufferSize);
void __cdecl Demo_WriteInfoData(int handle, demoMetaInfo *info);
void __cdecl Demo_InfoPlayerConnected(int clientNum);
void __cdecl Demo_InfoPlayerDisconnected(int clientNum);
void __cdecl Demo_AddBookmark(int type, int time, int clientNum1, int clientNum2);


extern SnapshotInfo_s g_snapInfo;
extern int g_democlientindex;
