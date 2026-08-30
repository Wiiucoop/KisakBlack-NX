#include "ddl_api.h"
#include <universal/dvar.h>
#include <cstdarg>
#include <stdio.h>
#include <qcommon/common.h>
#include "ddl_converter.h"
#include "ddl_buffer.h"
#include <database/db_registry.h>
#include "ddl_lookup.h"
#include <cstring>

const dvar_t *ddl_verbose;

void __cdecl DDL_Init()
{
    ddl_verbose = _Dvar_RegisterBool("ddl_verbose", 0, 0, "Verbose DDL output.");
}

int __cdecl DDL_GetTotalBufferBitSize(ddlDef_t *ddl)
{
    return ddl->size + 320;
}

void DDL_Printf(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    if ( ddl_verbose )
    {
        if ( ddl_verbose->current.enabled )
        {
            _vsnprintf(msg, 0x100u, fmt, va);
            Com_Printf(29, msg);
        }
    }
}

void DDL_NoCheckPrintf(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(msg, 0x100u, fmt, va);
    Com_Printf(29, msg);
}

void DDL_PrintWarning(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    if ( ddl_verbose )
    {
        if ( ddl_verbose->current.enabled )
        {
            _vsnprintf(msg, 0x100u, fmt, va);
            Com_PrintWarning(29, msg);
        }
    }
}

void DDL_NoCheckPrintWarning(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(msg, 0x100u, fmt, va);
    Com_PrintWarning(29, msg);
}

void DDL_PrintError(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    if ( ddl_verbose )
    {
        if ( ddl_verbose->current.enabled )
        {
            _vsnprintf(msg, 0x100u, fmt, va);
            Com_PrintError(29, msg);
        }
    }
}

void DDL_NoCheckPrintError(const char *fmt, ...)
{
    char msg[260]; // [esp+4h] [ebp-108h] BYREF
    va_list va; // [esp+118h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(msg, 0x100u, fmt, va);
    Com_PrintError(29, msg);
}

int __cdecl DDL_IsStateReadyForIO(const ddlState_t *state)
{
    if ( state->member )
    {
        if ( DDL_Converter_IsLeaf(state->member) )
        {
            if ( state->absoluteOffset < state->ddl->size )
            {
                return 1;
            }
            else
            {
                DDL_PrintError("DDL: Buffer offset is out of the DDL buffer limit for member %s.\n", state->member->name);
                return 0;
            }
        }
        else
        {
            DDL_PrintError("DDL: Attempt to read or write a value to a non-leaf member.\n");
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Member not found in path.\n");
        return 0;
    }
}

int __cdecl DDL_AssociateBuffer(char *buffer, int bufferSize, ddlDef_t *ddl)
{
    ddlBufferHeader_t header; // [esp+0h] [ebp-34h] BYREF
    int totalDDLBitSize; // [esp+2Ch] [ebp-8h]
    int totalDDLByteSize; // [esp+30h] [ebp-4h]

    totalDDLBitSize = ddl->size + 320;
    if ( totalDDLBitSize % 8 )
        totalDDLByteSize = totalDDLBitSize / 8 + 1;
    else
        totalDDLByteSize = totalDDLBitSize / 8;
    if ( 8 * bufferSize < totalDDLBitSize )
    {
        Com_Error(
            ERR_DROP,
            "DDL: Buffer validation error - buffer is not large enough to accommodate this DDL. %d bytes required.\n",
            totalDDLByteSize);
        return 0;
    }
    if ( 8 * bufferSize > totalDDLBitSize )
        DDL_Printf("DDL: Buffer is currently using %d bytes.\n", totalDDLByteSize);
    DDL_Buffer_ReadHeader(buffer, &header);
    if ( header.version )
    {
        if ( header.version != ddl->version )
        {
            DDL_PrintError("DDL: Buffer validation error - buffer is already associated with a different DDL version.\n");
            return 0;
        }
    }
    else
    {
        DDL_PrintWarning("DDL: Buffer was not associated with any version. Associating it with version %d.\n", ddl->version);
        header.version = ddl->version;
        DDL_Buffer_WriteHeader(buffer, &header);
    }
    return 1;
}

int __cdecl DDL_FixBufferVersion(
                char *buffer,
                ddlDef_t *ddlTo,
                const char *ddlName,
                char *backupBuffer,
                int bufferSize)
{
    ddlBufferHeader_t header; // [esp+0h] [ebp-30h] BYREF
    ddlDef_t *ddlFrom; // [esp+2Ch] [ebp-4h]

    DDL_Buffer_ReadHeader(buffer, &header);
    ddlFrom = DDL_LoadAssetWithVersion(ddlName, header.version);
    if ( ddlFrom )
        return DDL_Converter_Versions(ddlFrom, ddlTo, buffer, backupBuffer, bufferSize);
    DDL_PrintError("DDL: Could not convert versions. Old version not found.\n");
    return 0;
}

ddlDef_t *__cdecl DDL_LoadAssetWithVersion(const char *fileName, int version)
{
    ddlRoot_t *ddlRoot; // [esp+4h] [ebp-Ch]
    ddlDef_t *currDef; // [esp+8h] [ebp-8h]
    ddlDef_t *maxDef; // [esp+Ch] [ebp-4h]

    ddlRoot = DB_FindXAssetHeader(ASSET_TYPE_DDL, (char*)fileName, 1, -1).ddlRoot;
    if ( !ddlRoot )
        return 0;
    currDef = ddlRoot->ddlDef;
    if ( version )
    {
        while ( currDef )
        {
            if ( currDef->version == version )
                return currDef;
            currDef = currDef->next;
        }
        return 0;
    }
    maxDef = ddlRoot->ddlDef;
    while ( currDef )
    {
        if ( currDef->version > maxDef->version )
            maxDef = currDef;
        currDef = currDef->next;
    }
    return maxDef;
}

ddlDef_t *__cdecl DDL_LoadAsset(const char *fileName)
{
    return DDL_LoadAssetWithVersion(fileName, 0);
}

void __cdecl DDL_CopyFromBaseState(ddlState_t *newState, const ddlState_t *baseState)
{
    *newState = *baseState;
}

void __cdecl DDL_Reset(ddlState_t *searchState, ddlDef_t *ddl)
{
    DDL_Lookup_ClearState(searchState);
    searchState->ddl = ddl;
}

int DDL_MoveTo(const ddlState_t *searchState, ddlState_t *resultState, int argNum, ...)
{
    const char *v3; // kr00_4
    int i; // [esp+0h] [ebp-Ch]
#ifdef KISAK_NX
    va_list args;
#else
    char *args; // [esp+8h] [ebp-4h]
#endif
    va_list va; // [esp+20h] [ebp+14h] BYREF

    va_start(va, argNum);
    *resultState = *searchState;
    va_copy(args, va);
    for ( i = 0; i < argNum; ++i )
    {
        v3 = va_arg(args, const char *);
        if ( !DDL_Lookup_MoveTo(resultState, v3, resultState) )
            return 0;
    }
    return 1;
}

int __cdecl DDL_MoveToPath(const ddlState_t *searchState, ddlState_t *resultState, int depth, const char **ddlPath)
{
    int v4; // eax
    int i; // [esp+0h] [ebp-14h]
    ddlState_t tempState; // [esp+4h] [ebp-10h] BYREF

    if ( !*ddlPath || !DDL_MoveToName(searchState, &tempState, *ddlPath) )
        return 0;
    for ( i = 1; i < depth; ++i )
    {
        if ( ddlPath[i] )
        {
            if ( tempState.member && tempState.member->arraySize > 1 && tempState.member->enumIndex == -1 )
            {
                v4 = atoi(ddlPath[i]);
                if ( !DDL_MoveToIndex(&tempState, &tempState, v4, 1) )
                    return 0;
            }
            else if ( !DDL_MoveToName(&tempState, &tempState, ddlPath[i]) )
            {
                return 0;
            }
        }
    }
    *resultState = tempState;
    return 1;
}

bool __cdecl DDL_MoveToStart(ddlState_t *searchState, ddlDef_t *ddl)
{
    return DDL_Lookup_MoveToStart(searchState, ddl);
}

int __cdecl DDL_MoveToName(const ddlState_t *searchState, ddlState_t *resultState, const char *name)
{
    return DDL_Lookup_MoveTo(searchState, name, resultState);
}

int __cdecl DDL_MoveToIndex(
                const ddlState_t *searchState,
                ddlState_t *resultState,
                int arrayIndex,
                int checkValidIndex)
{
    return DDL_Lookup_MoveToIndex(searchState, arrayIndex, resultState, checkValidIndex);
}

int __cdecl DDL_MoveNext(const ddlState_t *searchState, ddlState_t *resultState)
{
    return DDL_Lookup_MoveNext(searchState, resultState);
}

int __cdecl DDL_IterateNext(const ddlState_t *searchState, ddlState_t *resultState)
{
    if ( searchState->member->arraySize <= 1 )
        return 0;
    if ( searchState->arrayIndex + 1 >= searchState->member->arraySize )
        return 0;
    DDL_CopyFromBaseState(resultState, searchState);
    if ( searchState->arrayIndex != -1 )
        resultState->absoluteOffset += resultState->member->size / resultState->member->arraySize;
    ++resultState->arrayIndex;
    return 1;
}

int __cdecl DDL_IteratePrev(const ddlState_t *searchState, ddlState_t *resultState)
{
    if ( searchState->member->arraySize <= 1 )
        return 0;
    if ( searchState->arrayIndex - 1 < 0 )
        return 0;
    DDL_CopyFromBaseState(resultState, searchState);
    resultState->absoluteOffset -= resultState->member->size / resultState->member->arraySize;
    --resultState->arrayIndex;
    return 1;
}

int __cdecl DDL_IterateFirst(const ddlState_t *searchState, ddlState_t *resultState)
{
    if ( searchState->member->arraySize <= 1 )
        return 0;
    DDL_CopyFromBaseState(resultState, searchState);
    if ( resultState->member->enumIndex == -1 )
    {
        while ( DDL_IteratePrev(resultState, resultState) )
            ;
    }
    else
    {
        resultState->arrayIndex = 0;
    }
    return 1;
}

int __cdecl DDL_SetRawBytes(char *buffer, int offset, int size, const char *valueToInsert)
{
    int i; // [esp+4h] [ebp-8h]

    if ( !DDL_Buffer_ClearBits(buffer, 8 * offset, 8 * size) )
        return 0;
    for ( i = 0; i < size; ++i )
    {
        if ( !DDL_Buffer_WriteBits(buffer, 8 * offset + 8 * i, 8, valueToInsert[i]) )
            return 0;
    }
    DDL_SetValueChanged(buffer);
    return 1;
}

void __cdecl DDL_SetValueChanged(char *buffer)
{
    DDL_SetUserFlag(buffer, 0, 1);
}

bool __cdecl DDL_WriteIntValueInternal(
                ddlMemberDef_t *member,
                int absoluteOffset,
                unsigned int valueToInsert,
                char *buffer)
{
    if ( member->type > 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    859,
                    0,
                    "%s",
                    "member->type == DDL_INT_TYPE || member->type == DDL_SHORT_TYPE || member->type == DDL_BYTE_TYPE") )
    {
        __debugbreak();
    }
    if ( valueToInsert >= member->min && valueToInsert <= member->max )
        return DDL_WriteIntValueIfChanged(
                         buffer,
                         absoluteOffset,
                         member->size / member->arraySize,
                         valueToInsert - member->min) != -1;
    DDL_NoCheckPrintWarning(
        "DDL: Trying to set a value of %d when the valid range is %d to %d\n",
        valueToInsert,
        member->min,
        member->max);
    return 0;
}

int __cdecl DDL_WriteIntValueIfChanged(char *buffer, int offset, int size, unsigned int valueToInsert)
{
    unsigned int oldValue; // [esp+0h] [ebp-4h] BYREF

    if ( !DDL_Buffer_ReadBits(buffer, offset, size, &oldValue) )
        return -1;
    if ( oldValue == valueToInsert )
        return 0;
    if ( !DDL_Buffer_WriteBits(buffer, offset, size, valueToInsert) )
        return -1;
    DDL_SetValueChanged(buffer);
    return 1;
}

bool __cdecl DDL_WriteInt64ValueInternal(
                ddlMemberDef_t *member,
                int absoluteOffset,
                unsigned __int64 valueToInsert,
                char *buffer)
{
    int size; // [esp+4h] [ebp-8h]

    if ( member->type != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    893,
                    0,
                    "%s",
                    "member->type == DDL_INT64_TYPE") )
    {
        __debugbreak();
    }
    if ( member->size / member->arraySize != 64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    894,
                    0,
                    "%s",
                    "member->size / member->arraySize == DDL_INT64_SIZE") )
    {
        __debugbreak();
    }
    size = member->size / member->arraySize / 2;
    return DDL_WriteIntValueIfChanged(buffer, absoluteOffset, size, valueToInsert) != -1
            && DDL_WriteIntValueIfChanged(buffer, absoluteOffset + 32, size, HIDWORD(valueToInsert)) != -1;
}

int __cdecl DDL_WriteStringValueInternal(ddlMemberDef_t *member, int absoluteOffset, const char *value, char *buffer)
{
    unsigned int v5; // [esp+4h] [ebp-20h]
    unsigned int i; // [esp+14h] [ebp-10h]
    unsigned int len; // [esp+18h] [ebp-Ch]
    unsigned int maxChars; // [esp+20h] [ebp-4h]

    if ( member->type != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    925,
                    0,
                    "%s",
                    "member->type == DDL_STRING_TYPE") )
    {
        __debugbreak();
    }
    v5 = strlen(value);
    len = v5;
    maxChars = member->size / member->arraySize / 8;
    if ( v5 > maxChars )
    {
        Com_PrintError(29, "DDL: '%s' too long. Can be up to %d chars but was %d chars. Truncating.\n", value, maxChars, v5);
        len = maxChars;
    }
    if ( !DDL_Buffer_ClearBits(buffer, absoluteOffset, member->size / member->arraySize) )
        return 0;
    for ( i = 0; i < len; ++i )
    {
        if ( !DDL_Buffer_WriteBits(buffer, absoluteOffset + 8 * i, 8, value[i]) )
            return 0;
    }
    DDL_SetValueChanged(buffer);
    return 1;
}

int __cdecl DDL_WriteValueInternal(ddlMemberDef_t *member, int absoluteOffset, const char *valueToInsert, char *buffer)
{
    unsigned int v4; // eax
    int result; // eax
    unsigned __int64 v6; // rax

    switch ( member->type )
    {
        case 0:
        case 1:
        case 2:
            v4 = atoi(valueToInsert);
            result = DDL_WriteIntValueInternal(member, absoluteOffset, v4, buffer);
            break;
        case 3:
            v6 = I_atoi64(valueToInsert);
            result = DDL_WriteInt64ValueInternal(member, absoluteOffset, v6, buffer);
            break;
        case 5:
            result = DDL_WriteStringValueInternal(member, absoluteOffset, valueToInsert, buffer);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl DDL_SetInt(const ddlState_t *searchState, unsigned int value, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 988, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 989, 0, "%s", "buffer") )
        __debugbreak();
    return DDL_IsStateReadyForIO(searchState)
            && DDL_WriteIntValueInternal(searchState->member, searchState->absoluteOffset, value, buffer);
}

bool __cdecl DDL_SetInt64(const ddlState_t *searchState, unsigned __int64 value, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1046, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1047, 0, "%s", "buffer") )
        __debugbreak();
    return DDL_IsStateReadyForIO(searchState)
            && DDL_WriteInt64ValueInternal(searchState->member, searchState->absoluteOffset, value, buffer);
}

int __cdecl DDL_SetString(const ddlState_t *searchState, const char *value, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1104, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1105, 0, "%s", "buffer") )
        __debugbreak();
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_WriteStringValueInternal(searchState->member, searchState->absoluteOffset, value, buffer);
    else
        return 0;
}

int __cdecl DDL_SetValue(const ddlState_t *searchState, const char *value, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1163, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1164, 0, "%s", "buffer") )
        __debugbreak();
    if ( searchState->member->arraySize > 1 && searchState->arrayIndex == -1 )
        return 0;
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_WriteValueInternal(searchState->member, searchState->absoluteOffset, value, buffer);
    return 0;
}

void __cdecl DDL_SetUserFlag(char *buffer, int flagIndex, bool state)
{
    ddlBufferHeader_t header; // [esp+0h] [ebp-2Ch] BYREF

    if ( flagIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    1282,
                    0,
                    "%s",
                    "flagIndex < DDL_MAX_HEADER_FLAGS") )
    {
        __debugbreak();
    }
    DDL_Buffer_ReadHeader(buffer, &header);
    if ( state )
        header.flags |= 1 << flagIndex;
    else
        header.flags &= ~(1 << flagIndex);
    DDL_Buffer_WriteHeader(buffer, &header);
}

unsigned int __cdecl DDL_ReadIntValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer)
{
    unsigned int uValue; // [esp+4h] [ebp-4h] BYREF

    if ( DDL_Buffer_ReadBits(buffer, absoluteOffset, member->size / member->arraySize, &uValue) )
        return member->min + uValue;
    else
        return 0;
}

unsigned __int64 __cdecl DDL_ReadInt64ValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer)
{
    unsigned __int64 result; // rax
    unsigned int low; // [esp+4h] [ebp-10h] BYREF
    unsigned int high; // [esp+8h] [ebp-Ch] BYREF
    unsigned __int64 value; // [esp+Ch] [ebp-8h]

    value = 0;
    if (!DDL_Buffer_ReadBits(buffer, absoluteOffset, member->size / member->arraySize / 2, &low)
        || !DDL_Buffer_ReadBits(buffer, absoluteOffset + 32, member->size / member->arraySize / 2, &high))
    {
        return value;
    }
    result = (unsigned __int64)high << 32;
    LODWORD(result) = low;
    return result;
}

unsigned __int8 stringResult[68];
char *__cdecl DDL_ReadStringValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer)
{
    int v4; // [esp+0h] [ebp-14h]
    int i; // [esp+8h] [ebp-Ch]
    unsigned int uValue; // [esp+Ch] [ebp-8h] BYREF
    int endIndex; // [esp+10h] [ebp-4h]

    memset(stringResult, 0, 0x40u);
    if ( member->size / 8 < 64 )
        v4 = member->size / 8;
    else
        v4 = 64;
    endIndex = v4;
    for ( i = 0; i < endIndex; ++i )
    {
        if ( !DDL_Buffer_ReadBits(buffer, absoluteOffset + 8 * i, 8, &uValue) )
        {
            memset(stringResult, 0, 0x40u);
            return (char *)stringResult;
        }
        stringResult[i] = uValue;
    }
    return (char *)stringResult;
}

int __cdecl DDL_ReadValueInternal(ddlMemberDef_t *member, int absoluteOffset, ddlResult_t *result, char *buffer)
{
    int v4; // eax
    char *StringValueInternal; // eax

    if (member)
    {
        if (DDL_Converter_IsLeaf(member))
        {
            switch (member->type)
            {
            case 0:
            case 1:
            case 2:
                result->intValue = DDL_ReadIntValueInternal(member, absoluteOffset, buffer);
                v4 = 1;
                break;
            case 3:
                result->int64Value = DDL_ReadInt64ValueInternal(member, absoluteOffset, buffer);
                v4 = 1;
                break;
            case 5:
                StringValueInternal = DDL_ReadStringValueInternal(member, absoluteOffset, buffer);
                I_strncpyz((char *)result, StringValueInternal, 64);
                v4 = 1;
                break;
            default:
                v4 = 0;
                break;
            }
        }
        else
        {
            DDL_PrintError("DDL: Invalid member, or attempt to read from a non-leaf member.\n");
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Member not found in path.\n");
        return 0;
    }
    return v4;
}

unsigned __int64 __cdecl DDL_GetInt64(const ddlState_t *searchState, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1444, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1445, 0, "%s", "buffer") )
        __debugbreak();
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_ReadInt64ValueInternal(searchState->member, searchState->absoluteOffset, buffer);
    else
        return 0;
}

unsigned int __cdecl DDL_GetInt(const ddlState_t *searchState, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1507, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1508, 0, "%s", "buffer") )
        __debugbreak();
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_ReadIntValueInternal(searchState->member, searchState->absoluteOffset, buffer);
    else
        return 0;
}

char *__cdecl DDL_GetString(const ddlState_t *searchState, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1570, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1571, 0, "%s", "buffer") )
        __debugbreak();
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_ReadStringValueInternal(searchState->member, searchState->absoluteOffset, buffer);
    else
        return (char *)"";
}

int __cdecl DDL_GetValue(const ddlState_t *searchState, ddlResult_t *result, char *buffer)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1631, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1632, 0, "%s", "buffer") )
        __debugbreak();
    if ( DDL_IsStateReadyForIO(searchState) )
        return DDL_ReadValueInternal(searchState->member, searchState->absoluteOffset, result, buffer);
    else
        return 0;
}

int __cdecl DDL_GetBits(char *buffer, int bitOffset, unsigned int bitSize, unsigned int *value)
{
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp", 1653, 0, "%s", "buffer") )
        __debugbreak();
    if ( bitSize > 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    1654,
                    0,
                    "%s",
                    "bitSize <= ( sizeof( unsigned int ) * 8 )") )
    {
        __debugbreak();
    }
    return DDL_Buffer_ReadBits(buffer, bitOffset, bitSize, value);
}

int __cdecl DDL_GetHeaderVersion(char *buffer)
{
    ddlBufferHeader_t header; // [esp+0h] [ebp-2Ch] BYREF

    DDL_Buffer_ReadHeader(buffer, &header);
    return header.version;
}

bool __cdecl DDL_GetUserFlag(char *buffer, int flagIndex)
{
    ddlBufferHeader_t header; // [esp+0h] [ebp-2Ch] BYREF

    if ( flagIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_api.cpp",
                    1721,
                    0,
                    "%s",
                    "flagIndex < DDL_MAX_HEADER_FLAGS") )
    {
        __debugbreak();
    }
    DDL_Buffer_ReadHeader(buffer, &header);
    return ((1 << flagIndex) & header.flags) != 0;
}

