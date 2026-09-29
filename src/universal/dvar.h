#pragma once

enum DvarSetSource : __int32
{                                                                             // XREF: Dvar_SetVariant/r
        DVAR_SOURCE_INTERNAL = 0x0,
        DVAR_SOURCE_EXTERNAL = 0x1,
        DVAR_SOURCE_SCRIPT     = 0x2,
        DVAR_SOURCE_DEVGUI     = 0x3,
};

enum dvarType_t : __int32
{                                                                             // XREF: dvar_s/r
                                                                                 // ?Dvar_ValueInDomain@@YA_NW4dvarType_t@@TDvarValue@@TDvarLimits@@@Z/r ...
         DVAR_TYPE_BOOL = 0x0,
         DVAR_TYPE_FLOAT = 0x1,
         DVAR_TYPE_FLOAT_2 = 0x2,
         DVAR_TYPE_FLOAT_3 = 0x3,
         DVAR_TYPE_FLOAT_4 = 0x4,
         DVAR_TYPE_INT = 0x5,
         DVAR_TYPE_ENUM = 0x6,
         DVAR_TYPE_STRING = 0x7,
         DVAR_TYPE_COLOR = 0x8,
         DVAR_TYPE_INT64 = 0x9,
         DVAR_TYPE_LINEAR_COLOR_RGB = 0xA,
         DVAR_TYPE_COLOR_XYZ = 0xB,
         DVAR_TYPE_COUNT = 0xC,
};

union DvarValue // sizeof=0x10
{                                                                             // XREF: CL_CommitDvarToProfiles+58/w
                                                                                // CL_CommitDvarToProfiles+5E/w ...
        bool enabled;
        int integer;
        unsigned int unsignedInt;
        __int64 integer64;
        unsigned __int64 unsignedInt64;
        float value;
        float vector[4];
        const char *string;
        unsigned __int8 color[4];
};

struct DvarLimits_enum // sizeof=0x8
{                                                                             // XREF: DvarLimits/r
        int stringCount;
        const char **strings;
};

struct DvarLimits_int // sizeof=0x8
{                                                                             // XREF: DvarLimits/r
        int min;
        int max;
};

struct DvarLimits_int64 // sizeof=0x10
{                                                                             // XREF: DvarLimits/r
        __int64 min;
        __int64 max;
};

struct DvarLimits_float // sizeof=0x8
{                                                                             // XREF: DvarLimits/r
        float min;
        float max;
};

struct DvarLimits_vector // sizeof=0x8
{                                                                             // XREF: DvarLimits/r
        float min;
        float max;
};

union DvarLimits // sizeof=0x10
{                                                                             // XREF: Dvar_ValueInDomain(dvarType_t,DvarValue,DvarLimits):$LN17_36/r
                                                                                // Dvar_ValueInDomain(dvarType_t,DvarValue,DvarLimits)+61/r ...
        DvarLimits_enum enumeration;
        DvarLimits_int integer;
        DvarLimits_int64 integer64;
        DvarLimits_float value;
        DvarLimits_vector vector;
};

struct __declspec(align(8)) dvar_s // sizeof=0x70
{                                                                             // XREF: .data:dvarPool/r
                                                                                // dvar_t/r
        const char *name;
        const char *description;
        int hash;
        unsigned int flags;                                 // XREF: Dvar_ResetScriptInfo(void)+4A/r
                                                                                // Dvar_ResetScriptInfo(void)+5C/w
        dvarType_t type;
        bool modified;
        bool loadedFromSaveGame;
        // padding byte
        // padding byte
        DvarValue current;
        DvarValue latched;
        DvarValue reset;
        DvarValue saved;
        DvarLimits domain;
        dvar_s *hashNext;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};
using dvar_t = dvar_s;

struct FastCriticalSection;

void __cdecl Dvar_Sort();
void __cdecl Dvar_ForEach(void (__cdecl *callback)(const dvar_s *, void *), void *userData);
void __cdecl Dvar_ForEachName(void (__cdecl *callback)(const char *));
void __cdecl Dvar_SetInAutoExec(bool inAutoExec);
bool __cdecl Dvar_IsSystemActive();
char __cdecl Dvar_IsValidName(const char *dvarName);
const char *__cdecl Dvar_EnumToString(const dvar_s *dvar);
const char *__cdecl Dvar_IndexStringToEnumString(const dvar_s *dvar, const char *indexString);
const char *__cdecl Dvar_DisplayableValue(const dvar_s *dvar);
const char *__cdecl Dvar_ValueToString(const dvar_s *dvar, DvarValue value);
const char *__cdecl Dvar_DisplayableResetValue(const dvar_s *dvar);
const char *__cdecl Dvar_DisplayableLatchedValue(const dvar_s *dvar);
char __cdecl Dvar_ValueInDomain(dvarType_t type, DvarValue value, DvarLimits domain);
char __cdecl Dvar_VectorInDomain(const float *vector, int components, float min, float max);
char *__cdecl Dvar_DomainToString(dvarType_t type, DvarLimits domain, char *outBuffer, int outBufferLen);
char *__cdecl Dvar_DomainToString_Internal(
                dvarType_t type,
                DvarLimits domain,
                char *outBuffer,
                int outBufferLen,
                int *outLineCount);
void __cdecl Dvar_VectorDomainToString(int components, DvarLimits domain, char *outBuffer, unsigned int outBufferLen);
char *__cdecl Dvar_DomainToString_GetLines(
                dvarType_t type,
                DvarLimits domain,
                char *outBuffer,
                int outBufferLen,
                int *outLineCount);
void __cdecl Dvar_PrintDomain(dvarType_t type, DvarLimits domain);
bool __cdecl Dvar_HasLatchedValue(const dvar_s *dvar);
bool __cdecl Dvar_ValuesEqual(dvarType_t type, DvarValue val0, DvarValue val1);
const dvar_s *__cdecl Dvar_FindVar(const char *dvarName);
dvar_s *__cdecl Dvar_FindMalleableVar(const char *dvarName);
void __cdecl Sys_UnlockRead(FastCriticalSection *critSect);
const dvar_s *__cdecl Dvar_FindVar(int dvarHash);
void __cdecl Dvar_ClearModified(const dvar_s *dvar);
void __cdecl Dvar_SetModified(const dvar_s *dvar);
void __cdecl Dvar_UpdateEnumDomain(dvar_s *dvar, const char **stringTable);
DvarValue *__cdecl Dvar_ClampValueToDomain(
                DvarValue *result,
                dvarType_t type,
                DvarValue value,
                DvarValue resetValue,
                DvarLimits domain);
void __cdecl Dvar_ClampVectorToDomain(float *vector, int components, float min, float max);
bool __cdecl Dvar_GetBool(const char *dvarName);
bool __cdecl Dvar_StringToBool(const char *string);
int __cdecl Dvar_GetInt(const char *dvarName);
int __cdecl Dvar_StringToInt(const char *string);
int __cdecl Dvar_GetInt(const dvar_s *dvar);
int __cdecl Dvar_GetInt(int dvarHash);
__int64 __cdecl Dvar_StringToInt64(const char *string);
double __cdecl Dvar_GetFloat(const char *dvarName);
double __cdecl Dvar_StringToFloat(const char *string);
double __cdecl Dvar_GetFloat(int dvarHash);
const char *__cdecl Dvar_GetString(const char *dvarName);
const char *__cdecl Dvar_GetVariantString(const char *dvarName);
const char *__cdecl Dvar_GetVariantString(int dvarHash);
void __cdecl Dvar_GetUnpackedColor(const dvar_s *dvar, float *expandedColor);
void __cdecl Dvar_StringToColor(const char *string, unsigned __int8 *color);
void __cdecl Dvar_GetUnpackedColorByName(const char *dvarName, float *expandedColor);
void __cdecl Dvar_GetUnpackedColor(int dvarHash, float *expandedColor);
double __cdecl Dvar_GetColorRed(const char *dvarName);
double __cdecl Dvar_GetColorRed(int dvarHash);
double __cdecl Dvar_GetColorGreen(const char *dvarName);
double __cdecl Dvar_GetColorGreen(int dvarHash);
double __cdecl Dvar_GetColorBlue(const char *dvarName);
double __cdecl Dvar_GetColorBlue(int dvarHash);
double __cdecl Dvar_GetColorAlpha(const char *dvarName);
double __cdecl Dvar_GetColorAlpha(int dvarHash);
void __cdecl Dvar_Shutdown();
void __cdecl Dvar_FreeNameString(const char *name);
bool __cdecl Dvar_ShouldFreeCurrentString(dvar_s *dvar);
bool __cdecl Dvar_ShouldFreeLatchedString(dvar_s *dvar);
bool __cdecl Dvar_ShouldFreeResetString(dvar_s *dvar);
void __cdecl Dvar_FreeString(DvarValue *value);
void __cdecl Dvar_ChangeResetValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_UpdateResetValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_AssignResetStringValue(dvar_s *dvar, DvarValue *dest, char *string);
void __cdecl Dvar_CopyString(char *string, DvarValue *value);
void __cdecl Dvar_WeakCopyString(const char *string, DvarValue *value);
void __cdecl Dvar_MakeLatchedValueCurrent(dvar_s *dvar);
void __cdecl Dvar_SetVariant(dvar_s *dvar, DvarValue value, DvarSetSource source);
void __cdecl Dvar_AssignCurrentStringValue(dvar_s *dvar, DvarValue *dest, char *string);
void __cdecl Dvar_SetLatchedValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_AssignLatchedStringValue(dvar_s *dvar, DvarValue *dest, char *string);
char __cdecl Dvar_CanChangeValue(const dvar_s *dvar, DvarValue value, DvarSetSource source);
void __cdecl Dvar_ClearLatchedValue(dvar_s *dvar);
const dvar_s *__cdecl _Dvar_RegisterBool(const char *dvarName, bool value, unsigned __int16 flags, const char *description);
const dvar_s *__cdecl Dvar_RegisterVariant(
                char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain,
                const char *description);
void __cdecl Dvar_Reregister(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue resetValue,
                DvarLimits domain,
                const char *description);
void __cdecl Dvar_MakeExplicitType(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue resetValue,
                DvarLimits domain);
DvarValue *__cdecl Dvar_StringToValue(DvarValue *result, dvarType_t type, DvarLimits domain, const char *string);
void __cdecl Dvar_StringToVec2(const char *string, float *vector);
void __cdecl Dvar_StringToVec3(const char *string, float *vector);
void __cdecl Dvar_StringToVec4(const char *string, float *vector);
int __cdecl Dvar_StringToEnum(const DvarLimits *domain, const char *string);
void __cdecl Dvar_UpdateValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_ReinterpretDvar(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain);
void __cdecl Dvar_PerformUnregistration(dvar_s *dvar);
char *__cdecl Dvar_AllocNameString(char *name);
DvarValue *__cdecl Dvar_GetReinterpretedResetValue(DvarValue *result, dvar_s *__formal, DvarValue value);
const dvar_s *__cdecl Dvar_RegisterNew(
                char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterInt(
                const char *dvarName,
                int value,
                int min,
                int max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterInt64(
                char *dvarName,
                __int64 value,
                __int64 min,
                __int64 max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterFloat(
                const char *dvarName,
                float value,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterVec2(
                const char *dvarName,
                float x,
                float y,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterVec3(
                const char *dvarName,
                float x,
                float y,
                float z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterVec4(
                const char *dvarName,
                float x,
                float y,
                float z,
                float w,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterString(
                const char *dvarName,
                const char *value,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterEnum(
                const char *dvarName,
                const char **valueList,
                int defaultIndex,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterColor(
                const char *dvarName,
                float r,
                float g,
                float b,
                float a,
                unsigned __int16 flags,
                const char *description);
// x/y/z are FLOAT components, not integers -- see the transcription-bug note on the
// definitions in dvar.cpp. Retail moves them with MOVSS into DvarValue.vector[0..2].
const dvar_s *__cdecl _Dvar_RegisterLinearRGB(
                char *dvarName,
                float x,
                float y,
                float z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
const dvar_s *__cdecl _Dvar_RegisterColorXYZ(
                const char *dvarName,
                float x,
                float y,
                float z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description);
void __cdecl Dvar_SetDvarValueFromSource(dvar_s *dvar, DvarValue newValue, DvarSetSource source);
void __cdecl Dvar_SetBoolFromSource(dvar_s *dvar, bool value, DvarSetSource source);
void __cdecl Dvar_SetIntFromSource(dvar_s *dvar, int value, DvarSetSource source);
void __cdecl Dvar_SetInt64FromSource(dvar_s *dvar, __int64 value, DvarSetSource source);
void __cdecl Dvar_SetFloatFromSource(dvar_s *dvar, float value, DvarSetSource source);
void __cdecl Dvar_SetVec2FromSource(dvar_s *dvar, float x, float y, DvarSetSource source);
void __cdecl Dvar_SetVec3FromSource(dvar_s *dvar, float x, float y, float z, DvarSetSource source);
void __cdecl Dvar_SetVec4FromSource(
                dvar_s *dvar,
                float x,
                float y,
                float z,
                float w,
                DvarSetSource source);
void __cdecl Dvar_SetColorFromSource(dvar_s *dvar, float r, float g, float b, float a, DvarSetSource source);
void __cdecl Dvar_SetBool(dvar_s *dvar, bool value);
void __cdecl Dvar_SetInt(dvar_s *dvar, int value);
void __cdecl Dvar_SetInt64(dvar_s *dvar, __int64 value);
void __cdecl Dvar_SetFloat(dvar_s *dvar, float value);
void __cdecl Dvar_SetVec2(dvar_s *dvar, float x, float y);
void __cdecl Dvar_SetVec3(dvar_s *dvar, float x, float y, float z);
void __cdecl Dvar_SetVec4(dvar_s *dvar, float x, float y, float z, float w);
void __cdecl Dvar_SetString(dvar_s *dvar, const char *value);
void __cdecl Dvar_SetStringFromSource(dvar_s *dvar, const char *string, DvarSetSource source);
void __cdecl Dvar_SetColor(dvar_s *dvar, float r, float g, float b, float a);
void __cdecl Dvar_SetFromString(dvar_s *dvar, const char *string);
void __cdecl Dvar_SetFromStringFromSource(dvar_s *dvar, const char *string, DvarSetSource source);
void __cdecl Dvar_SetBoolByName(const char *dvarName, bool value);
void __cdecl Dvar_SetIntByName(const char *dvarName, int value);
void __cdecl Dvar_SetInt64ByName(const char *dvarName, __int64 value);
void __cdecl Dvar_SetFloatByName(const char *dvarName, float value);
void __cdecl Dvar_SetVec2ByName(const char *dvarName, float x, float y);
void __cdecl Dvar_SetVec3ByName(const char *dvarName, float x, float y, float z);
void __cdecl Dvar_SetVec4ByName(const char *dvarName, float x, float y, float z, float w);
void __cdecl Dvar_SetStringByName(const char *dvarName, char *value);
void __cdecl Dvar_SetColorByName(const char *dvarName, float r, float g, float b, float a);
const dvar_s *__cdecl Dvar_SetFromStringByNameFromSource(
                const char *dvarName,
                char *string,
                DvarSetSource source,
                unsigned __int16 flags);
void __cdecl Dvar_SetFromStringByName(const char *dvarName, char *string);
void __cdecl Dvar_SetCommand(const char *dvarName, char *string);
void __cdecl Dvar_AddFlags(const dvar_s *dvar, int flags);
void __cdecl Dvar_Reset(dvar_s *dvar, DvarSetSource setSource);
void __cdecl Dvar_SetCheatState();
void __cdecl Dvar_Init();
void __cdecl Dvar_ResetScriptInfo();
char __cdecl Dvar_AnyLatchedValues();
void __cdecl Dvar_RestoreDvars();
void __cdecl Dvar_ResetDvars(unsigned int filter, DvarSetSource setSource);
void __cdecl Dvar_ForEachConsoleAccessName(void (__cdecl *callback)(const char *));
void __cdecl Dvar_ResetConAccessDvars();
int __cdecl Com_SaveDvarsToBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, unsigned int bufsize);
int __cdecl Com_LoadDvarsFromBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, char *filename);
int __cdecl Com_LoadDvarsFromBufferOptional(
                const char **dvarnames,
                bool *isOptional,
                unsigned int numDvars,
                char *buffer,
                char *filename);
void __cdecl Dvar_SetBoolIfChanged(dvar_s *dvar, bool value);
void __cdecl Dvar_SetIntIfChanged(dvar_s *dvar, int value);
void __cdecl Dvar_SetFloatIfChanged(dvar_s *dvar, float value);


extern int dvar_modifiedFlags;
extern int dvarCount;