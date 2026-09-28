#pragma once

#include "cscr_main.h"
#include <universal/mem_userhunk.h>
#include <stddef.h>

#undef GetObject // windows aids

#define VAR_MASK 0x1F
#define CLASS_NUM_COUNT 5
#define VAR_NAME_BITS 8

#define VAR_NAME_LOW_MASK 0x00FFFFFF


#define VARIABLELIST_PARENT_BEGIN 1

enum $DC39398868F8F7F398F8DA27178506AD : __int32
{
    VAR_UNDEFINED         = 0x0,
    VAR_BEGIN_REF         = 0x1,
    VAR_POINTER           = 0x1,
    VAR_STRING            = 0x2,
    VAR_ISTRING           = 0x3,
    VAR_VECTOR            = 0x4,
    VAR_END_REF           = 0x5,
    VAR_FLOAT             = 0x5,
    VAR_INTEGER           = 0x6,
    VAR_CODEPOS           = 0x7,
    VAR_PRECODEPOS        = 0x8,
    VAR_FUNCTION          = 0x9,
    VAR_STACK             = 0xA,
    VAR_ANIMATION         = 0xB,
    VAR_DEVELOPER_CODEPOS = 0xC,
    VAR_THREAD            = 0xD,
    VAR_NOTIFY_THREAD     = 0xE,
    VAR_TIME_THREAD       = 0xF,
    VAR_CHILD_THREAD      = 0x10,
    VAR_OBJECT            = 0x11,
    VAR_DEAD_ENTITY       = 0x12,
    VAR_ENTITY            = 0x13,
    VAR_ARRAY             = 0x14,
    VAR_DEAD_THREAD       = 0x15,
    VAR_COUNT             = 0x16,
    VAR_THREAD_LIST       = 0x17,
    VAR_ENDON_LIST        = 0x18,
};

struct scr_classStruct_t // sizeof=0x10
{                                       // XREF: .data:scr_classStruct_t * gServerClassMap/r
    unsigned int id;
    unsigned int entArrayId;
    char charId;
    // padding byte
    // padding byte
    // padding byte
    const char *name;
};

union ObjectInfo_u // sizeof=0x4
{                                                                             // XREF: ObjectInfo/r
        unsigned __int16 entnum;
        unsigned __int16 size;
        unsigned int nextEntId;
        unsigned int self;
};

struct ObjectInfo // sizeof=0x8
{                                                                             // XREF: VariableValueInternal::<unnamed_type_u>/r
        unsigned __int16 refCount;
        // padding byte
        // padding byte
        ObjectInfo_u u;
};

struct __declspec(align(4)) VariableStackBuffer // sizeof=0x10
{
        const char *pos;
        unsigned __int16 size;
        unsigned __int16 bufLen;
        unsigned int localId;
        unsigned __int8 time;
        char buf[1];
        // padding byte
        // padding byte
};

union VariableUnion // sizeof=0x4
{                                                                             // XREF: Scr_EmitAnimationInternal+8E/w
    //VariableUnion(float f)
    //{
    //    floatValue = f;
    //}
    //VariableUnion(int i)
    //{
    //    intValue = i;
    //}
    //VariableUnion(unsigned int i)
    //{
    //    intValue = (int)i;
    //}
    //VariableUnion(char *str)
    //{
    //    codePosValue = str;
    //}
    //VariableUnion(const char *str)
    //{
    //    codePosValue = str;
    //}
    //VariableUnion()
    //{
    //    intValue = 0;
    //}

    int intValue;
    float floatValue;
    unsigned int stringValue;
    const float *vectorValue;
    const char *codePosValue;
    unsigned int pointerValue;
    VariableStackBuffer *stackValue;
    unsigned int entityOffset;
};

// nx-port: the archived stack after a VariableStackBuffer's header is packed,
// one entry per stack value: a type byte, then the whole VariableUnion. On x86
// that was 1 + 4 bytes after a 13-byte header, and the VM spelled both as
// literals (5 * size + 13, stackValue + 4, + 13 ...); on LP64 the union holds
// 8-byte pointers -- vectors, code positions, stack buffers -- and the header
// is the struct's own. Entries are unaligned: read and write them with memcpy.
enum {
    SCR_STACKBUF_HEADER = offsetof(VariableStackBuffer, buf),
    SCR_STACKBUF_ENTRY  = 1 + sizeof(VariableUnion),
};

union VariableValueInternal_u // sizeof=0x8
{                                                                             // XREF: VariableValueInternal/r
        unsigned int next;
        VariableUnion u;
        ObjectInfo o;
};

union VariableValueInternal_w // sizeof=0x4
{                                                                             // XREF: VariableValueInternal/r
        unsigned int status;
        unsigned int type;
        unsigned int name;
        unsigned int classnum;
        unsigned int notifyName;
        unsigned int waitTime;
        unsigned int parentLocalId;
};

union VariableValueInternal_v // sizeof=0x4
{                                                                             // XREF: VariableValueInternal/r
        unsigned int next;
        unsigned int index;
};

union Variable_u // sizeof=0x4
{                                                                             // XREF: MakeVariableExternal+3E8/w
                                                                                // MakeVariableExternal+403/r ...
        unsigned int prev;
        unsigned int prevSibling;
};

struct Variable // sizeof=0x8
{                                                                             // XREF: VariableValueInternal/r
                                                                                // MakeVariableExternal/r
        unsigned int id;                                        // XREF: MakeVariableExternal+3E5/w
                                                                                // MakeVariableExternal+3FE/r
        Variable_u u;             // XREF: MakeVariableExternal+3E8/w
                                                                                // MakeVariableExternal+403/r
};

struct VariableValueInternal // sizeof=0x1C
{
        Variable hash;
        VariableValueInternal_u u;
        VariableValueInternal_w w;
        VariableValueInternal_v v;
        unsigned int nextSibling;
};

struct VariableValue // sizeof=0x8
{                                                                             // XREF: CompareArrayIndices+79/r
        VariableUnion u;                                        // XREF: Scr_EmitAnimationInternal+8E/w
        int type;                                                     // XREF: Scr_EmitAnimationInternal+84/w
};

struct scr_entref_t // sizeof=0x6
{                                                                             // XREF: ?Scr_GetPathnode@@YAPAUpathnode_t@@IW4scriptInstance_t@@@Z/r
        unsigned __int16 entnum;                        // XREF: CScr_PlaySoundOnEntity+12/r
        unsigned __int16 classnum;                    // XREF: CScr_PlaySoundOnEntity+6/r
        unsigned __int16 client;                        // XREF: CScr_PlaySoundOnEntity:loc_4EB534/r

        scr_entref_t()
        {
            entnum = 0;
            classnum = 0;
        }
        scr_entref_t(int i)
        {
            entnum = i;
            classnum = i;
        }
};

struct scrVarPub_t // sizeof=0x74
{                                       // XREF: .data:scrVarPub_t * gScrVarPub/r
    //const char *fieldBuffer;            // XREF: Scr_BeginLoadScripts(scriptInstance_t,int)+4D6/w
    char *fieldBuffer;            // XREF: Scr_BeginLoadScripts(scriptInstance_t,int)+4D6/w
                                        // Scr_FindField(char const *,int *,scriptInstance_t)+C/r ...
    unsigned __int16 canonicalStrCount; // XREF: Scr_ArchiveCanonicalStrings(scriptInstance_t)+C1/r
                                        // Scr_ArchiveCanonicalStrings(scriptInstance_t)+F4/r ...
    bool developer;                     // XREF: EmitPreAssignmentPos+9/r
                                        // EmitAssignmentPos+9/r ...
    bool developer_script;              // XREF: LinkThread+13F/r
                                        // SpecifyThread+DF/r ...
    bool evaluate;                      // XREF: CheckThreadPosition+C/r
                                        // Scr_ScriptWatch::EvaluateWatchChildren(scriptInstance_t,Scr_WatchElement_s *)+5F/r ...
    // padding byte
    // padding byte
    // padding byte
    const char *error_message;          // XREF: EvalBinaryOperatorExpression+90/r
                                        // EvalBinaryOperatorExpression+9F/r ...
    int error_index;                    // XREF: Scr_EvalMethod+DA/w
                                        // Scr_EvalMethod+14F/w ...
    unsigned int time;                  // XREF: VM_Notify+676/r
                                        // VM_Notify+745/r ...
    unsigned int timeArrayId;           // XREF: CG_ParseClientSystemStateChange(int,int,char const *):loc_5003B1/r
                                        // Scr_IsThreadAlive(uint,scriptInstance_t)+A/r ...
    unsigned int pauseArrayId;          // XREF: Scr_KillThread(scriptInstance_t,uint)+E7/r
                                        // Scr_KillThread(scriptInstance_t,uint)+2A2/r ...
    unsigned int levelId;               // XREF: Scr_HitAssignmentBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+5FE/r
                                        // Scr_GetValueString(scriptInstance_t,uint,VariableValue *,int,char *)+22D/r ...
    unsigned int gameId;                // XREF: Scr_HitAssignmentBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+661/r
                                        // Scr_EvalPrimitiveExpression+2CC/r ...
    unsigned int animId;                // XREF: Scr_HitAssignmentBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+62E/r
                                        // Scr_GetValueString(scriptInstance_t,uint,VariableValue *,int,char *)+24A/r ...
    unsigned int freeEntList;           // XREF: Scr_FreeEntityNum(int,uint,scriptInstance_t)+1D6/r
                                        // Scr_FreeEntityNum(int,uint,scriptInstance_t)+1FC/r ...
    unsigned int tempVariable;          // XREF: Scr_VM_Init+10D/w
                                        // VM_Shutdown+9/r ...
    bool bInited;                       // XREF: Scr_FreeEntityNum(int,uint,scriptInstance_t)+C/r
                                        // Scr_RemoveClassMap(scriptInstance_t,uint)+A/r ...
    // padding byte
    unsigned __int16 savecount;
    unsigned int checksum;              // XREF: Scr_CreateAnimationTree+11C/r
                                        // Scr_CreateAnimationTree+125/w ...
    unsigned int entId;                 // XREF: Scr_GetVariableFieldIndex(scriptInstance_t,uint,uint)+144/w
                                        // SetVariableFieldValue(scriptInstance_t,uint,VariableValue *)+39/r ...
    unsigned int entFieldName;          // XREF: Scr_GetVariableFieldIndex(scriptInstance_t,uint,uint)+153/w
                                        // SetVariableFieldValue(scriptInstance_t,uint,VariableValue *)+2C/r ...
    HunkUser *programHunkUser;          // XREF: Scr_BeginLoadScripts(scriptInstance_t,int)+420/w
                                        // Scr_BeginLoadScripts(scriptInstance_t,int)+42C/r ...
    const char *programBuffer;          // XREF: ScriptCompile(scriptInstance_t,sval_u,uint,uint,uint,PrecacheEntry *,int)+10D/r
                                        // Scr_FindBreakpointInfo+39/r ...
    const char *endScriptBuffer;        // XREF: Scr_BeginLoadScripts(scriptInstance_t,int)+4AE/w
                                        // Scr_EndLoadAnimTrees(scriptInstance_t)+128/r ...
    unsigned __int16 *saveIdMap;        // XREF: Scr_InitVariables(scriptInstance_t)+85/w
    unsigned __int16 *saveIdMapRev;     // XREF: Scr_InitVariables(scriptInstance_t)+C7/w
    unsigned int numScriptThreads;      // XREF: Scr_VM_Init+1E5/w
                                        // Scr_GetNumScriptThreads(scriptInstance_t)+9/r ...
    unsigned int numScriptValues;       // XREF: Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+547/r
                                        // Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+56A/r ...
    unsigned int numScriptObjects;      // XREF: Scr_InitVariables(scriptInstance_t)+21A/w
                                        // Scr_ShutdownVariables(scriptInstance_t)+A7/r ...
    const char *varUsagePos;            // XREF: Scr_DisplayDebuggerRemote+3B/r
                                        // Scr_DisplayDebuggerRemote+4A/w ...
    int ext_threadcount;                // XREF: Scr_CheckLeaks+FD/r
                                        // Scr_VM_Init+1D5/w ...
    int totalObjectRefCount;            // XREF: AddRefToObject(scriptInstance_t,uint)+4D/r
                                        // AddRefToObject(scriptInstance_t,uint)+5C/w ...
    //volatile int totalVectorRefCount;   // XREF: RemoveRefToVector(scriptInstance_t,float const *)+1A/o
    volatile unsigned int totalVectorRefCount;   // XREF: RemoveRefToVector(scriptInstance_t,float const *)+1A/o
                                        // AddRefToVector(scriptInstance_t,float const *)+1A/o ...
};

struct __declspec(align(4)) scrVarDebugPub_t // sizeof=0x10
{                                       // XREF: .data:scrVarDebugPub_t * gScrVarDebugPubBuff/r
                                        // scrVarDebugPubArray_t/r
    const char **varUsage;
    unsigned __int16 *extRefCount;
    int *leakCount;
    bool dummy;
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(128)) scrVarGlob_t // sizeof=0x80
{                                       // XREF: .data:scrVarGlob_t * gScrVarGlob/r
    VariableValueInternal *variableList;
                                        // XREF: Scr_GetArrayValues_Vector(long,long,float (*)[3],long,char const *)+114/r
                                        // GScr_AddSpawnPoints(void)+109/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct ThreadDebugInfo // sizeof=0x8C
{                                       // XREF: ?Scr_DumpScriptThreads@@YAXW4scriptInstance_t@@@Z/r
    const char *pos[32];                // XREF: Scr_DumpScriptThreads(scriptInstance_t)+1CA/w
                                        // Scr_DumpScriptThreads(scriptInstance_t)+1E5/w ...
    int posSize;                        // XREF: Scr_DumpScriptThreads(scriptInstance_t)+15C/w
                                        // Scr_DumpScriptThreads(scriptInstance_t)+1C4/r ...
    float varUsage;                     // XREF: Scr_DumpScriptThreads(scriptInstance_t)+2F0/w
                                        // Scr_DumpScriptThreads(scriptInstance_t)+317/r ...
    float endonUsage;                   // XREF: Scr_DumpScriptThreads(scriptInstance_t)+2FD/w
                                        // Scr_DumpScriptThreads(scriptInstance_t)+336/r ...
};

struct VariableDebugInfo // sizeof=0x10
{
    const char *pos;
    const char *fileName;
    const char *functionName;
    int varUsage;
};


void __cdecl Scr_DumpScriptThreads(scriptInstance_t inst);
int __cdecl ThreadInfoCompare(unsigned int *info1, unsigned int *info2);
void __cdecl Scr_DumpScriptVariables(
                scriptInstance_t inst,
                bool spreadsheet,
                bool summary,
                bool total,
                bool functionSummary,
                bool lineSort,
                const char *fileName,
                const char *functionName,
                int minCount);
int __cdecl VariableInfoFileNameCompare(unsigned int *info1, unsigned int *info2);
int __cdecl VariableInfoFunctionCompare(unsigned int *info1, unsigned int *info2);
int __cdecl VariableInfoCountCompare(unsigned int *info1, unsigned int *info2);
int __cdecl VariableInfoFileLineCompare(unsigned int *info1, unsigned int *info2);
void __cdecl Scr_DumpScriptVariablesDefault(scriptInstance_t inst);
void __cdecl Scr_InitVariables(scriptInstance_t inst);
void __cdecl Scr_InitVariableRange(scriptInstance_t inst, unsigned int begin, unsigned int end);
void __cdecl Scr_InitClassMap(scriptInstance_t inst);
void __cdecl Scr_ShutdownVariables(scriptInstance_t inst);
void __cdecl Scr_CheckLeaks(scriptInstance_t inst);
void __cdecl Scr_CheckLeakRange(scriptInstance_t inst, unsigned int begin, unsigned int end);
unsigned int __cdecl Scr_GetNumScriptVars(scriptInstance_t inst);
unsigned int __cdecl Scr_GetNumScriptVarsParent(scriptInstance_t inst);
unsigned int __cdecl Scr_GetNumScriptVarsChild(scriptInstance_t inst);
unsigned int __cdecl GetVariableKeyObject(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl GetVariableIndexInternal(scriptInstance_t inst, unsigned int parentId, unsigned int name);
unsigned int __cdecl GetNewVariableIndexInternal2(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index);
unsigned int __cdecl GetNewVariableIndexInternal3(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index);
void __cdecl ClearObject(scriptInstance_t inst, unsigned int parentId);
void __cdecl ClearObjectInternal(scriptInstance_t inst, unsigned int parentId);
void __cdecl MakeVariableExternal(scriptInstance_t inst, unsigned int index, VariableValueInternal *parentValue);
void __cdecl FreeChildValue(scriptInstance_t inst, unsigned int parentId, unsigned int id);
void __cdecl Scr_SetThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId, unsigned int stringValue);
void __cdecl Scr_StopThread(scriptInstance_t inst, unsigned int threadId);
void __cdecl Scr_ClearThread(scriptInstance_t inst, unsigned int parentId);
void __cdecl Scr_RemoveThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId);
unsigned int __cdecl Scr_GetThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId);
void __cdecl Scr_SetThreadWaitTime(scriptInstance_t inst, unsigned int startLocalId, unsigned int waitTime);
void __cdecl Scr_ClearWaitTime(scriptInstance_t inst, unsigned int startLocalId);
unsigned int __cdecl Scr_GetThreadWaitTime(scriptInstance_t inst, unsigned int startLocalId);
unsigned int __cdecl GetParentLocalId(scriptInstance_t inst, unsigned int threadId);
unsigned int __cdecl GetSafeParentLocalId(scriptInstance_t inst, unsigned int threadId);
unsigned int __cdecl GetStartLocalId(scriptInstance_t inst, unsigned int threadId);
void __cdecl Scr_KillThread(scriptInstance_t inst, unsigned int parentId);
void __cdecl Scr_KillEndonThread(scriptInstance_t inst, unsigned int threadId);
unsigned int __cdecl AllocValue(scriptInstance_t inst);
unsigned int __cdecl AllocObject(scriptInstance_t inst);
unsigned int __cdecl AllocVariable(scriptInstance_t inst);
unsigned int __cdecl Scr_AllocArray(scriptInstance_t inst);
unsigned int __cdecl AllocThread(scriptInstance_t inst, unsigned int self);
unsigned int __cdecl AllocChildThread(scriptInstance_t inst, unsigned int self, unsigned int parentLocalId);
unsigned int __cdecl Scr_GetSelf(scriptInstance_t inst, unsigned int threadId);
void __cdecl FreeValue(scriptInstance_t inst, unsigned int id);
void __cdecl RemoveRefToObject(scriptInstance_t inst, unsigned int id);
void __cdecl FreeVariable(scriptInstance_t inst, unsigned int id);
void __cdecl RemoveRefToEmptyObject(scriptInstance_t inst, unsigned int id);
int __cdecl Scr_GetRefCountToObject(scriptInstance_t inst, unsigned int id);
float *__cdecl Scr_AllocVector(scriptInstance_t inst, float *v);
float *__cdecl Scr_AllocVector(scriptInstance_t inst);
bool __cdecl IsValidArrayIndex(scriptInstance_t inst, unsigned int unsignedValue);
unsigned int __cdecl GetInternalVariableIndex(scriptInstance_t inst, unsigned int unsignedValue);
unsigned int __cdecl FindArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int intValue);
unsigned int __cdecl FindArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl FindVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl FindObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id);
unsigned int __cdecl GetArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl Scr_GetVariableFieldIndex(scriptInstance_t inst, unsigned int parentId, int name);
VariableValue __cdecl Scr_FindVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int name);
unsigned int __cdecl Scr_FindAllVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int *names);
void __cdecl ClearVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int name, VariableValue *value);
unsigned int __cdecl GetArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl GetNewArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl GetNewArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl GetNewVariableIndexInternal(scriptInstance_t inst, unsigned int parentId, unsigned int name);
unsigned int __cdecl GetVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl GetNewVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
unsigned int __cdecl GetObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id);
unsigned int __cdecl GetNewObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id);
unsigned int __cdecl GetNewObjectVariableReverse(scriptInstance_t inst, unsigned int parentId, unsigned int id);
unsigned int __cdecl GetNewVariableIndexReverseInternal(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name);
unsigned int __cdecl GetNewVariableIndexReverseInternal2(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index);
void __cdecl RemoveVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
void __cdecl RemoveNextVariable(scriptInstance_t inst, unsigned int parentId);
void __cdecl RemoveObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id);
void __cdecl RemoveArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
void __cdecl SafeRemoveVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
void __cdecl RemoveVariableValue(scriptInstance_t inst, unsigned int parentId, unsigned int index);
void __cdecl SetNewVariableValue(scriptInstance_t inst, unsigned int id, VariableValue *value);
VariableValueInternal_u *__cdecl GetVariableValueAddress(scriptInstance_t inst, unsigned int id);
void __cdecl SetVariableEntityFieldValue(
                scriptInstance_t inst,
                unsigned int entId,
                unsigned int fieldName,
                VariableValue *value);
void __cdecl ClearVariableValue(scriptInstance_t inst, unsigned int id);
void __cdecl SetVariableFieldValue(scriptInstance_t inst, unsigned int id, VariableValue *value);
unsigned int __cdecl Scr_EvalVariableObject(scriptInstance_t inst, unsigned int id);
VariableValue __cdecl Scr_EvalVariableEntityField(scriptInstance_t inst, unsigned int entId, unsigned int fieldName);
VariableValue __cdecl Scr_EvalVariableField(scriptInstance_t inst, unsigned int id);
void __cdecl Scr_EvalSizeValue(scriptInstance_t inst, VariableValue *value);
unsigned int __cdecl GetArraySize(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl GetObject(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl GetArray(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl FindObject(scriptInstance_t inst, unsigned int id);
bool __cdecl IsFieldObject(scriptInstance_t inst, unsigned int id);
bool __cdecl Scr_IsThreadAlive(unsigned int thread, scriptInstance_t inst);
void __cdecl Scr_EvalBoolNot(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_EvalBoolComplement(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_CastBool(scriptInstance_t inst, VariableValue *value);
char __cdecl Scr_CastString(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_CastDebugString(scriptInstance_t inst, VariableValue *value);
char __cdecl Scr_GetEntClassId(scriptInstance_t inst, unsigned int id);
int __cdecl Scr_GetEntNum(scriptInstance_t inst, unsigned int id);
void __cdecl Scr_ClearVector(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_CastVector(scriptInstance_t inst, VariableValue *value);
unsigned int __cdecl Scr_EvalFieldObject(scriptInstance_t inst, unsigned int tempVariable, VariableValue *value);
void __cdecl Scr_UnmatchingTypesError(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalOr(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalExOr(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalAnd(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalEquality(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_CastWeakerPair(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalInequality(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalLess(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalGreaterEqual(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalGreater(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalLessEqual(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalShiftLeft(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalShiftRight(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalPlus(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_CastWeakerStringPair(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalMinus(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalMultiply(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalDivide(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalMod(scriptInstance_t inst, VariableValue *value1, VariableValue *value2);
void __cdecl Scr_EvalBinaryOperator(scriptInstance_t inst, int op, VariableValue *value1, VariableValue *value2);
bool __cdecl IsObjectFree(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl GetValueType(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl GetObjectType(scriptInstance_t inst, unsigned int id);
void __cdecl Scr_FreeEntityNum(unsigned int entnum, unsigned int classnum, scriptInstance_t inst);
void __cdecl Scr_FreeEntityList(scriptInstance_t inst);
void __cdecl Scr_FreeObjects(scriptInstance_t inst);
void __cdecl Scr_SetClassMap(scriptInstance_t inst, unsigned int classnum);
void __cdecl Scr_RemoveClassMap(scriptInstance_t inst, unsigned int classnum);
void __cdecl Scr_AddClassField(unsigned int classnum, char *name, unsigned int offset, scriptInstance_t inst);
int __cdecl Scr_GetOffset(unsigned int classnum, char *name, scriptInstance_t inst);
unsigned int __cdecl FindEntityId(scriptInstance_t inst, unsigned int entnum, unsigned int classnum, int clientNum);
unsigned int __cdecl Scr_GetEntityId(scriptInstance_t inst, unsigned int entnum, unsigned int classnum, int clientNum);
unsigned int __cdecl AllocEntity(
                scriptInstance_t inst,
                unsigned int classnum,
                unsigned __int16 entnum,
                unsigned __int8 clientNum);
void __cdecl Scr_EvalArray(scriptInstance_t inst, VariableValue *value, VariableValue *index);
unsigned int __cdecl Scr_FindArrayIndex(scriptInstance_t inst, unsigned int parentId, VariableValue *index);
unsigned int __cdecl Scr_EvalArrayRef(scriptInstance_t inst, unsigned int parentId);
void __cdecl CopyArray(scriptInstance_t inst, unsigned int parentId, unsigned int newParentId);
void __cdecl ClearArray(scriptInstance_t inst, unsigned int parentId, VariableValue *value);
void __cdecl SafeRemoveArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue);
void __cdecl SetEmptyArray(scriptInstance_t inst, unsigned int parentId);
void __cdecl Scr_AddArrayKeys(unsigned int parentId, scriptInstance_t inst);
scr_entref_t __cdecl Scr_GetEntityIdRef(scriptInstance_t inst, unsigned int entId);
void __cdecl Scr_CopyEntityNum(
                unsigned int fromEntnum,
                unsigned int toEntnum,
                unsigned int classnum,
                scriptInstance_t inst);
void __cdecl CopyEntity(scriptInstance_t inst, unsigned int parentId, unsigned int newParentId);
double __cdecl Scr_GetObjectUsage(scriptInstance_t inst, unsigned int parentId);
double __cdecl Scr_GetEntryUsage(scriptInstance_t inst, VariableValueInternal *entryValue);
double __cdecl Scr_GetThreadUsage(scriptInstance_t inst, const VariableStackBuffer *stackBuf, float *endonUsage);
double __cdecl Scr_GetEndonUsage(scriptInstance_t inst, unsigned int parentId);
unsigned int __cdecl Scr_FindField(const char *name, int *type, scriptInstance_t inst);
void __cdecl Scr_AddFields(const char *path, char *extension, scriptInstance_t inst);
void __cdecl Scr_AddFields_LoadObj(scriptInstance_t inst, const char *path, char *extension);
void __cdecl Scr_AddFieldsForFile(scriptInstance_t inst, char *filename);
char *__cdecl Scr_GetSourceFile_LoadObj(char *filename);
char *__cdecl Scr_GetSourceFile_FastFile(const char *filename);
void __cdecl Scr_AddFields_FastFile(scriptInstance_t inst, const char *path, const char *extension);
void __cdecl Scr_FreeValue(scriptInstance_t inst, unsigned int id);
void __cdecl Scr_AllocGameVariable(scriptInstance_t inst);
void __cdecl Scr_FreeGameVariable(scriptInstance_t inst, int bComplete);
int __cdecl Scr_MakeValuePrimitive(scriptInstance_t inst, unsigned int parentId);
int __cdecl Scr_GetClassnumForCharId(scriptInstance_t inst, char charId);
int __cdecl Scr_FindAllThreads(scriptInstance_t inst, unsigned int selfId, unsigned int *threads, unsigned int localId);
unsigned int __cdecl Scr_FindAllEndons(scriptInstance_t inst, unsigned int threadId, unsigned int *names);

static constexpr const char *var_typename[] =
{
  "undefined",
  "object",
  "string",
  "localized string",
  "vector",
  "float",
  "int",
  "codepos",
  "precodepos",
  "function",
  "stack",
  "animation",
  "developer codepos",
  "thread",
  "thread",
  "thread",
  "thread",
  "struct",
  "removed entity",
  "entity",
  "array",
  "removed thread"
};

extern scrVarGlob_t gScrVarGlob[2];
extern scrVarPub_t gScrVarPub[2];
extern scrVarDebugPub_t *gScrVarDebugPub[2];

extern scr_classStruct_t *gScrClassMap[2];