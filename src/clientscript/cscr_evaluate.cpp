#include "cscr_evaluate.h"
#include <cstdlib>
#include <universal/assertive.h>
#include <string.h>
#include "cscr_stringlist.h"
#include "cscr_compiler.h"
#include <qcommon/common.h>
#include "cscr_vm.h"
#include "cscr_parser.h"
#include <xanim/xanim.h>
#include "cscr_parsetree.h"
#include "cscr_tempmemory.h"
#include "cscr_yacc.h"
#include <setjmp.h>

scrEvaluateGlob_t gScrEvaluateGlob[2];

int g_script_error_level[2];
#ifdef KISAK_NX
jmp_buf g_script_error[2][33];
#else
int g_script_error[2][33][16];
#endif

int __cdecl Scr_CompareCanonicalStrings(unsigned int *arg1, unsigned int *arg2)
{
    return gScrEvaluateGlob[gDebuggerInstance].canonicalStringLookup[*arg1]
        - gScrEvaluateGlob[gDebuggerInstance].canonicalStringLookup[*arg2];
}

void __cdecl Scr_ArchiveCanonicalStrings(scriptInstance_t inst)
{
    char v1; // dl
    char *v2; // [esp+1Ch] [ebp-30h]
    char *v3; // [esp+20h] [ebp-2Ch]
    unsigned int stringValue; // [esp+34h] [ebp-18h]
    unsigned int stringValuea; // [esp+34h] [ebp-18h]
    int len; // [esp+38h] [ebp-14h]
    int lena; // [esp+38h] [ebp-14h]
    unsigned __int16 canonicalStr; // [esp+3Ch] [ebp-10h]
    char *s; // [esp+40h] [ebp-Ch]
    char *debugString; // [esp+44h] [ebp-8h]
    unsigned __int16 i; // [esp+48h] [ebp-4h]
    unsigned __int16 ia; // [esp+48h] [ebp-4h]

    len = 0;
    for (stringValue = 0; stringValue < 0x10000; ++stringValue)
    {
        if (gScrCompilePub[inst].canonicalStrings[stringValue])
            len += strlen(SL_ConvertToString(stringValue, inst)) + 1;
    }
    gScrEvaluateGlob[inst].archivedCanonicalStringsBuf = (char *)Hunk_UserAlloc(
        g_DebugHunkUser,
        len,
        4,
        "Scr_ArchiveCanonicalStrings1");
    gScrEvaluateGlob[inst].archivedCanonicalStrings = (ArchivedCanonicalStringInfo *)Hunk_UserAlloc(
        g_DebugHunkUser,
        8
        * gScrVarPub[inst].canonicalStrCount,
        4,
        "Scr_ArchiveCanonicalStrings2");
    gScrEvaluateGlob[inst].canonicalStringLookup = (int *)Hunk_UserAlloc(
        g_DebugHunkUser,
        4 * gScrVarPub[inst].canonicalStrCount + 4,
        4,
        "Scr_ArchiveCanonicalStrings3");
    i = 0;
    lena = 0;
    for (stringValuea = 0; stringValuea < 0x10000; ++stringValuea)
    {
        canonicalStr = gScrCompilePub[inst].canonicalStrings[stringValuea];
        if (canonicalStr)
        {
            if (canonicalStr > (int)gScrVarPub[inst].canonicalStrCount
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    156,
                    0,
                    "%s",
                    "canonicalStr <= gScrVarPub[inst].canonicalStrCount"))
            {
                __debugbreak();
            }
            s = SL_ConvertToString(stringValuea, inst);
            debugString = &gScrEvaluateGlob[inst].archivedCanonicalStringsBuf[lena];
            v3 = s;
            v2 = debugString;
            do
            {
                v1 = *v3;
                *v2++ = *v3++;
            } while (v1);
            gScrEvaluateGlob[inst].archivedCanonicalStrings[i].canonicalStr = canonicalStr;
            gScrEvaluateGlob[inst].archivedCanonicalStrings[i].value = debugString;
            lena += strlen(s) + 1;
            ++i;
        }
    }
    if (i != gScrVarPub[inst].canonicalStrCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            167,
            0,
            "%s",
            "i == gScrVarPub[inst].canonicalStrCount"))
    {
        __debugbreak();
    }
    qsort(
        gScrEvaluateGlob[inst].archivedCanonicalStrings,
        gScrVarPub[inst].canonicalStrCount,
        8u,
        (int(__cdecl *)(const void *, const void *))CompareCanonicalStrings);
    for (ia = 0; ia < (int)gScrVarPub[inst].canonicalStrCount; ++ia)
    {
        if (!gScrEvaluateGlob[inst].archivedCanonicalStrings[ia].canonicalStr
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                173,
                0,
                "%s",
                "gScrEvaluateGlob[inst].archivedCanonicalStrings[i].canonicalStr"))
        {
            __debugbreak();
        }
        if (gScrEvaluateGlob[inst].archivedCanonicalStrings[ia].canonicalStr > (int)gScrVarPub[inst].canonicalStrCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                174,
                0,
                "%s",
                "gScrEvaluateGlob[inst].archivedCanonicalStrings[i].canonicalStr <= gScrVarPub[inst].canonicalStrCount"))
        {
            __debugbreak();
        }
        gScrEvaluateGlob[inst].canonicalStringLookup[gScrEvaluateGlob[inst].archivedCanonicalStrings[ia].canonicalStr] = ia;
    }
    *gScrEvaluateGlob[inst].canonicalStringLookup = 0;
}

int __cdecl CompareCanonicalStrings(const char **arg1, const char **arg2)
{
    return strcmp(arg1[1], arg2[1]);
}

const char *__cdecl Scr_GetCanonicalString(scriptInstance_t inst, unsigned int fieldName)
{
    if (!fieldName
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            190,
            0,
            "%s",
            "fieldName"))
    {
        __debugbreak();
    }
    if (fieldName > gScrVarPub[inst].canonicalStrCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            191,
            0,
            "%s",
            "fieldName <= gScrVarPub[inst].canonicalStrCount"))
    {
        __debugbreak();
    }
    if (gScrEvaluateGlob[inst].canonicalStringLookup[fieldName] >= (unsigned int)gScrVarPub[inst].canonicalStrCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            192,
            0,
            "%s",
            "(unsigned)gScrEvaluateGlob[inst].canonicalStringLookup[fieldName] < gScrVarPub[inst].canonicalStrCount"))
    {
        __debugbreak();
    }
    return gScrEvaluateGlob[inst].archivedCanonicalStrings[gScrEvaluateGlob[inst].canonicalStringLookup[fieldName]].value;
}

void __cdecl Scr_InitEvaluate(scriptInstance_t inst)
{
    gScrEvaluateGlob[inst].archivedCanonicalStringsBuf = 0;
    gScrEvaluateGlob[inst].archivedCanonicalStrings = 0;
    gScrEvaluateGlob[inst].canonicalStringLookup = 0;
    //BG_EvalVehicleName();
}

void __cdecl Scr_EndLoadEvaluate(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer && com_sv_running->current.enabled)
        Scr_ArchiveCanonicalStrings(inst);
}

void __cdecl Scr_ShutdownEvaluate(scriptInstance_t inst)
{
    if (gScrEvaluateGlob[inst].archivedCanonicalStringsBuf)
    {
        Hunk_UserFree(g_DebugHunkUser, gScrEvaluateGlob[inst].archivedCanonicalStringsBuf);
        gScrEvaluateGlob[inst].archivedCanonicalStringsBuf = 0;
    }
    if (gScrEvaluateGlob[inst].archivedCanonicalStrings)
    {
        Hunk_UserFree(g_DebugHunkUser, gScrEvaluateGlob[inst].archivedCanonicalStrings);
        gScrEvaluateGlob[inst].archivedCanonicalStrings = 0;
    }
    if (gScrEvaluateGlob[inst].canonicalStringLookup)
    {
        Hunk_UserFree(g_DebugHunkUser, gScrEvaluateGlob[inst].canonicalStringLookup);
        gScrEvaluateGlob[inst].canonicalStringLookup = 0;
    }
    //BG_EvalVehicleName();
}

unsigned __int16 __cdecl Scr_CompileCanonicalString(scriptInstance_t inst, unsigned int stringValue)
{
    int v3; // [esp+4h] [ebp-24h]
    int low; // [esp+18h] [ebp-10h]
    int middle; // [esp+1Ch] [ebp-Ch]
    char *s; // [esp+20h] [ebp-8h]
    int high; // [esp+24h] [ebp-4h]

    s = SL_ConvertToString(stringValue, inst);
    low = 0;
    high = gScrVarPub[inst].canonicalStrCount;
    while (low < high)
    {
        middle = (high + low) / 2;
        v3 = strcmp(s, gScrEvaluateGlob[inst].archivedCanonicalStrings[middle].value);
        if (v3 >= 0)
        {
            if (v3 <= 0)
                return gScrEvaluateGlob[inst].archivedCanonicalStrings[middle].canonicalStr;
            low = middle + 1;
        }
        else
        {
            high = (high + low) / 2;
        }
    }
    return 0;
}

void __cdecl Scr_GetFieldValue(
    scriptInstance_t inst,
    unsigned int objectId,
    const char *fieldName,
    int len,
    char *text)
{
    unsigned __int16 v5; // ax
    unsigned int stringValue; // [esp+8h] [ebp-Ch]
    VariableValue tempValue; // [esp+Ch] [ebp-8h] BYREF

    stringValue = SL_FindString(fieldName, inst);
    if (stringValue)
    {
        v5 = Scr_CompileCanonicalString(inst, stringValue);
        if (v5)
        {
            Scr_EvalFieldVariableInternal(inst, objectId, v5, &tempValue);
            if (gScrVarPub[inst].error_message)
            {
                Scr_ClearErrorMessage(inst);
                *text = 0;
            }
            else
            {
                Scr_GetValueString(inst, 0, &tempValue, len, text);
                RemoveRefToValue(inst, tempValue.type, tempValue.u);
            }
        }
        else
        {
            *text = 0;
        }
    }
    else
    {
        *text = 0;
    }
}

void __cdecl Scr_GetValueString(scriptInstance_t inst, unsigned int localId, VariableValue *value, int len, char *s)
{
    char *v5; // eax
    char *v6; // eax
    const XAnim_s *Anims; // eax
    char *AnimDebugName; // eax
    char EntClassId; // al
    unsigned int ArraySize; // eax
    unsigned int intValue; // [esp+14h] [ebp-20h]
    int EntNum; // [esp+14h] [ebp-20h]
    unsigned int type; // [esp+2Ch] [ebp-8h]
    VariableUnion id; // [esp+30h] [ebp-4h]

    if (value->type >= 0x16u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            354,
            0,
            "%s",
            "(unsigned)value->type < VAR_COUNT"))
    {
        __debugbreak();
    }
    switch (value->type)
    {
    case 0:
        I_strncpyz(s, "undefined", len);
        break;
    case 1:
        id.intValue = value->u.intValue;
        type = GetObjectType(inst, value->u.intValue);
        if (type == 19)
        {
            EntNum = Scr_GetEntNum(inst, id.stringValue);
            EntClassId = Scr_GetEntClassId(inst, id.stringValue);
            Com_sprintf(s, len, "$%c%i", EntClassId, EntNum);
        }
        else if (id.intValue == gScrVarPub[inst].levelId)
        {
            I_strncpyz(s, "level", len);
        }
        else if (id.intValue == gScrVarPub[inst].animId)
        {
            I_strncpyz(s, "anim", len);
        }
        else
        {
            if (type >= 0x16
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    409,
                    0,
                    "%s",
                    "(unsigned)type < VAR_COUNT"))
            {
                __debugbreak();
            }
            switch (type)
            {
            case 0xDu:
            case 0xEu:
            case 0xFu:
            case 0x10u:
                Com_sprintf(s, len, "$t%i", id.intValue);
                break;
            case 0x11u:
                if (!localId || id.intValue != Scr_GetSelf(inst, localId))
                    goto LABEL_28;
                I_strncpyz(s, "self", len);
                break;
            case 0x14u:
                ArraySize = GetArraySize(inst, id.stringValue);
                Com_sprintf(s, len, "<array of size %i>", ArraySize);
                break;
            default:
            LABEL_28:
                Com_sprintf(s, len, "<%s>", var_typename[type]);
                break;
            }
        }
        break;
    case 2:
        v5 = SL_ConvertToString(value->u.intValue, inst);
        Com_sprintf(s, len, "\"%s\"", v5);
        break;
    case 3:
        v6 = SL_ConvertToString(value->u.intValue, inst);
        Com_sprintf(s, len, "&\"%s\"", v6);
        break;
    case 4:
        sprintf(
            s,
            "(%g, %g, %g)",
            *(float *)value->u.intValue,
            *(float *)(value->u.intValue + 4),
            *(float *)(value->u.intValue + 8));
        break;
    case 5:
        Com_sprintf(s, len, "%g", value->u.floatValue);
        break;
    case 6:
        Com_sprintf(s, len, "%i", value->u.intValue);
        break;
    case 9:
        Scr_GetCodePos(inst, (const char *)(value->u.intValue - 1), 1u, s, len);
        break;
    case 0xB:
        intValue = (unsigned __int16)value->u.intValue;
        Anims = Scr_GetAnims(HIWORD(value->u.intValue), SCRIPTINSTANCE_SERVER);
        AnimDebugName = XAnimGetAnimDebugName(Anims, intValue);
        Com_sprintf(s, len, "%%%s", AnimDebugName);
        break;
    default:
        Com_sprintf(s, len, "<%s>", var_typename[value->type]);
        break;
    }
}

void __cdecl Scr_EvalArrayVariable(scriptInstance_t inst, unsigned int arrayId, VariableValue *value)
{
    VariableValue parentValue; // [esp+4h] [ebp-8h] BYREF

    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            493,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    parentValue.type = 1;
    parentValue.u.intValue = arrayId;
    AddRefToObject(inst, arrayId);
    Scr_EvalArrayVariableInternal(inst, &parentValue, value);
}

void __cdecl Scr_EvalArrayVariableInternal(scriptInstance_t inst, VariableValue *parentValue, VariableValue *value)
{
    if (gScrVarPub[inst].error_message || (Scr_EvalArray(inst, parentValue, value), gScrVarPub[inst].error_message))
    {
        Scr_ClearValue(inst, value);
        Scr_ClearValue(inst, parentValue);
    }
}

void __cdecl Scr_ClearValue(scriptInstance_t inst, VariableValue *value)
{
    RemoveRefToValue(inst, value->type, value->u);
    value->type = 0;
}

void __cdecl Scr_EvalFieldVariableInternal(
    scriptInstance_t inst,
    unsigned int objectId,
    unsigned int fieldName,
    VariableValue *value)
{
    unsigned int outparamcount; // [esp+8h] [ebp-8h]
    VariableValue *savedTop; // [esp+Ch] [ebp-4h]

    if (!fieldName
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1099,
            0,
            "%s",
            "fieldName"))
    {
        __debugbreak();
    }
    if (IsFieldObject(inst, objectId))
    {
        outparamcount = gScrVmPub[inst].outparamcount;
        gScrVmPub[inst].outparamcount = 0;
        savedTop = gScrVmPub[inst].top;
        *value = Scr_FindVariableField(inst, objectId, fieldName);
        gScrVmPub[inst].outparamcount = outparamcount;
        gScrVmPub[inst].top = savedTop;
        if (!value->type && !FindVariable(inst, objectId, fieldName))
            Scr_Error(inst, "unknown field", 0);
    }
    else
    {
        value->type = 0;
        Scr_Error(inst, "not a field object", 0);
    }
}

void __cdecl Scr_EvalFieldVariable(
    scriptInstance_t inst,
    unsigned int fieldName,
    VariableValue *value,
    unsigned int objectId)
{
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1134,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    Scr_EvalFieldVariableInternal(inst, objectId, fieldName, value);
}
void __cdecl Scr_CompileExpression(scriptInstance_t inst, sval_u *expr)
{
    switch (expr->node[0].type)
    {
        case ENUM_primitive_expression:
            Scr_CompilePrimitiveExpression(inst, &expr->node[1]);
            *expr = debugger_node1(inst, ENUM_primitive_expression, expr->node[1]);
            break;
        case ENUM_bool_or:
        case ENUM_bool_and:
            Scr_CompileExpression(inst, &expr->node[1]);
            Scr_CompileExpression(inst, &expr->node[2]);
            *expr = debugger_node2(
                                inst,
                expr->node[0].type,
                expr->node[1],
                expr->node[2]);
            break;
        case ENUM_binary:
            Scr_CompileExpression(inst, &expr->node[1]);
            Scr_CompileExpression(inst, &expr->node[2]);
            *expr = debugger_node3(
                                inst,
                                expr->node[0].type,
                                expr->node[1],
                                expr->node[2],
                                expr->node[3]);
            break;
        case ENUM_bool_not:
        case ENUM_bool_complement:
            Scr_CompileExpression(inst, &expr->node[1]);
            *expr = debugger_node1(inst, expr->node[0].type, expr->node[1]);
            break;
        default:
            return;
    }
}

void __cdecl Scr_CompilePrimitiveExpression(scriptInstance_t inst, sval_u *expr)
{
    sval_u tempVariableId; // [esp+2Ch] [ebp-8h]
    sval_u tempVariableIda; // [esp+2Ch] [ebp-8h]

    switch (expr->node[0].type)
    {
    case ENUM_integer:
    case ENUM_float:
    case ENUM_minus_integer:
    case ENUM_minus_float:
        *expr = debugger_node1(inst, expr->node[0].type, expr->node[1]);
        break;
    case ENUM_string:
    case ENUM_istring:
        *expr = debugger_string(inst, expr->node[0].type, SL_ConvertToString(*(_DWORD *)(expr->stringValue + 4), inst));
        break;
    case ENUM_variable:
        Scr_CompileVariableExpression(inst, &expr->node[1]);
        tempVariableId.stringValue = AllocValue(inst);
        *expr = debugger_node2(inst, ENUM_variable, expr->node[1], tempVariableId);
        break;
    case ENUM_call_expression:
        if (!Scr_CompileCallExpression(inst, &expr->node[1]))
            goto LABEL_13;
        tempVariableIda.stringValue = AllocValue(inst);
        *expr = debugger_node2(inst, ENUM_call_expression, expr->node[1], tempVariableIda);
        break;
    case ENUM_undefined:
    case ENUM_level:
    case ENUM_game:
    case ENUM_anim:
    case ENUM_empty_array:
    case ENUM_false:
    case ENUM_true:
        *expr = debugger_node0(inst, expr->node[0].type);
        break;
    case ENUM_self:
        *expr = debugger_node2(inst, expr->node[0].type, 0, 0);
        break;
    case ENUM_expression_list:
        Scr_CompilePrimitiveExpressionList(inst, &expr->node[1]);
        *expr = debugger_node1(inst, ENUM_expression_list, expr->node[1]);
        break;
    case ENUM_size_field:
        Scr_CompilePrimitiveExpression(inst, &expr->node[1]);
        *expr = debugger_node1(inst, ENUM_size_field, expr->node[1]);
        break;
    case ENUM_breakon:
        ++gScrVmDebugPub[inst].checkBreakon;
        g_breakonExpr = 1;
        Scr_CompilePrimitiveExpression(inst, &expr->node[1]);
        Scr_CompileExpression(inst, &expr->node[2]);
        *expr = debugger_node3(
            inst,
            expr->node[0].type,
            expr->node[1],
            expr->node[2],
            0);
        break;
    default:
    LABEL_13:
        *expr = debugger_node0(inst, ENUM_bad_expression);
        break;
    }
}

void __cdecl Scr_CompileVariableExpression(scriptInstance_t inst, sval_u *expr)
{
    signed int ObjectType; // [esp+14h] [ebp-58h]
    int argumentIndex; // [esp+4Ch] [ebp-20h]
    sval_u classnum; // [esp+50h] [ebp-1Ch]
    sval_u tempVariableId; // [esp+54h] [ebp-18h]
    char *s; // [esp+5Ch] [ebp-10h]
    sval_u entnum; // [esp+60h] [ebp-Ch]
    sval_u idValue; // [esp+68h] [ebp-4h]

    switch ( expr->node[0].type )
    {
        case 5:
            *(unsigned int *)(expr->stringValue + 4) = Scr_CompileCanonicalString(inst, *(unsigned int *)(expr->stringValue + 4));
            if ( *(unsigned int *)(expr->stringValue + 4) )
            {
                tempVariableId.stringValue = AllocValue(inst);
                *expr = debugger_node4(inst, ENUM_local_variable, expr->node[1], 0, 0, tempVariableId);
            }
            else
            {
                *expr = debugger_node0(inst, ENUM_unknown_variable);
            }
            break;
        case 0xF:
            Scr_CompileExpression(inst, &expr->node[2]);
            Scr_CompilePrimitiveExpression(inst, &expr->node[1]);
            *expr = debugger_node2(inst, ENUM_array_variable, expr->node[1], expr->node[2]);
            break;
        case 0x11:
            Scr_CompilePrimitiveExpressionFieldObject(inst, &expr->node[1]);
            *(unsigned int *)(expr->stringValue + 8) = Scr_CompileCanonicalString(inst, *(unsigned int *)(expr->stringValue + 8));
            if ( *(unsigned int *)(expr->stringValue + 8) )
                *expr = debugger_node3(inst, ENUM_field_variable, expr->node[1], expr->node[2], 0);
            else
                *expr = debugger_node0(inst, ENUM_unknown_field);
            break;
        case 0x37:
            Scr_CompilePrimitiveExpression(inst, &expr->node[1]);
            *expr = debugger_node1(inst, ENUM_self_field, expr->node[1]);
            break;
        case 0x52:
            s = SL_ConvertToString(*(unsigned int *)(expr->stringValue + 4), inst);
            if ( *s == 116 )
            {
                idValue.stringValue = atoi(s + 1);
                if ( !idValue.stringValue )
                    goto LABEL_28;
                if ( idValue.stringValue >= 0x7FFE )
                    goto LABEL_28;
                if ( IsObjectFree(inst, idValue.stringValue) )
                    goto LABEL_28;
                ObjectType = GetObjectType(inst, idValue.stringValue);
                if ( ObjectType < 13 || ObjectType > 16 && ObjectType != 21 )
                    goto LABEL_28;
                *expr = debugger_node1(inst, ENUM_thread_object, idValue);
                AddRefToObject(inst, idValue.stringValue);
            }
            else if ( *s == 97 )
            {
                argumentIndex = atoi(s + 1);
                if ( argumentIndex <= 0 && strcmp(s + 1, "0") )
                    goto LABEL_28;
                *expr = debugger_node1(inst, ENUM_argument, (sval_u)argumentIndex);
            }
            else
            {
                classnum.stringValue = Scr_GetClassnumForCharId(inst, *s);
                if ( classnum.intValue < 0 )
                    goto LABEL_28;
                entnum.stringValue = atoi(s + 1);
                if ( !entnum.stringValue && s[1] != 48 )
                    goto LABEL_28;
                *expr = debugger_node3(inst, ENUM_object, classnum, entnum, 0);
            }
            break;
        default:
LABEL_28:
            *expr = debugger_node0(inst, ENUM_bad_expression);
            break;
    }
}

void __cdecl Scr_CompilePrimitiveExpressionFieldObject(scriptInstance_t inst, sval_u *expr)
{
    sval_u tempVariableId; // [esp+18h] [ebp-8h]
    sval_u tempVariableIda; // [esp+18h] [ebp-8h]

    switch ( expr->node[0].type )
    {
        case 0x13:
            Scr_CompileVariableExpression(inst, &expr->node[1]);
            tempVariableId.stringValue = AllocValue(inst);
            *expr = debugger_node2(inst, ENUM_variable, expr->node[1], tempVariableId);
            break;
        case 0x15:
            Scr_CompileCallExpression(inst, &expr->node[1]);
            tempVariableIda.stringValue = AllocValue(inst);
            *expr = debugger_node2(inst, ENUM_call_expression, expr->node[1], tempVariableIda);
            break;
        case 0x22:
            *expr = debugger_node2(inst, expr->node[0].type, 0, 0);
            break;
        case 0x24:
        case 0x26:
            *expr = debugger_node0(inst, expr->node[0].type);
            break;
        default:
            *expr = debugger_node0(inst, ENUM_bad_expression);
            break;
    }
}

void __cdecl Scr_CompilePrimitiveExpressionList(scriptInstance_t inst, sval_u *exprlist)
{
    sval_u *nodea; // [esp+8h] [ebp-18h]
    sval_u *node; // [esp+8h] [ebp-18h]
    int expr_count; // [esp+Ch] [ebp-14h]
    int i; // [esp+10h] [ebp-10h]
    sval_u expr[3]; // [esp+14h] [ebp-Ch]

    expr_count = GetExpressionCount(*exprlist);
    if ( expr_count == 1 )
    {
        nodea = *(sval_u **)exprlist->stringValue;
        Scr_CompileExpression(inst, nodea->node);
        *exprlist = *nodea->node;
    }
    else if ( expr_count == 3 )
    {
        i = 0;
        for ( node = *(sval_u **)exprlist->stringValue; node; node = node[1].node )
        {
            Scr_CompileExpression(inst, node->node);
            expr[i++] = *node->node;
        }
        *exprlist = debugger_node3(inst, ENUM_vector, expr[0], expr[1], expr[2]);
    }
    else
    {
        *exprlist = debugger_node0(inst, ENUM_bad_expression);
    }
}

char __cdecl Scr_CompileCallExpression(scriptInstance_t inst, sval_u *expr)
{
    if (expr->node[0].type == ENUM_call)
    {
        if ( Scr_CompileFunction(inst, &expr->node[1], &expr->node[2]) )
        {
            *expr = debugger_node2(inst, ENUM_call, expr->node[1], expr->node[2]);
            return 1;
        }
    }
    else if (expr->node[0].type == ENUM_method
                 && Scr_CompileMethod(
                            inst,
                            &expr->node[1],
                            &expr->node[2],
                            &expr->node[3]) )
    {
        *expr = debugger_node3(
                            inst,
                            ENUM_method,
                            expr->node[1],
                            expr->node[2],
                            expr->node[3]);
        return 1;
    }
    return 0;
}

char __cdecl Scr_CompileFunction(scriptInstance_t inst, sval_u *func_name, sval_u *params)
{
    void (__cdecl *func)(); // [esp+0h] [ebp-10h]
    unsigned int name; // [esp+4h] [ebp-Ch]
    const char *pName; // [esp+8h] [ebp-8h] BYREF
    int type; // [esp+Ch] [ebp-4h] BYREF

    name = Scr_GetBuiltin(inst, *func_name);
    if ( !name )
        return 0;
    pName = SL_ConvertToString(name, inst);
    type = 0;
    func = GetFunction(inst, &pName, &type);
    if ( !func )
        return 0;
    func_name->stringValue = (unsigned int)func;
    Scr_CompileCallExpressionList(inst, params);
    return 1;
}

void __cdecl Scr_CompileCallExpressionList(scriptInstance_t inst, sval_u *exprlist)
{
    sval_u *node; // [esp+8h] [ebp-8h]
    sval_u expr; // [esp+Ch] [ebp-4h]

    expr = debugger_node0(inst, ENUM_NOP);
    for ( node = *(sval_u **)exprlist->stringValue; node; node = node[1].node )
    {
        Scr_CompileExpression(inst, node->node);
        expr = debugger_prepend_node(inst, *node->node, expr);
    }
    *exprlist = expr;
}

char __cdecl Scr_CompileMethod(scriptInstance_t inst, sval_u *expr, sval_u *func_name, sval_u *params)
{
    void (__cdecl *meth)(scr_entref_t); // [esp+0h] [ebp-10h]
    unsigned int name; // [esp+4h] [ebp-Ch]
    const char *pName; // [esp+8h] [ebp-8h] BYREF
    int type; // [esp+Ch] [ebp-4h] BYREF

    name = Scr_GetBuiltin(inst, *func_name);
    if ( !name )
        return 0;
    pName = SL_ConvertToString(name, inst);
    type = 0;
    meth = GetMethod(inst, &pName, &type);
    if ( !meth )
        return 0;
    Scr_CompilePrimitiveExpression(inst, expr);
    func_name->stringValue = (unsigned int)meth;
    Scr_CompileCallExpressionList(inst, params);
    return 1;
}

void __cdecl Scr_CompileText(scriptInstance_t inst, const char *text, ScriptExpression_t *scriptExpr)
{
    g_debugExprHead = 0;
    g_breakonExpr = 0;
    Scr_CompileTextInternal(inst, text, scriptExpr);
    if (!g_debugExprHead
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            2203,
            0,
            "%s",
            "g_debugExprHead"))
    {
        __debugbreak();
    }
    scriptExpr->exprHead = g_debugExprHead;
    scriptExpr->breakonExpr = g_breakonExpr;
}

void __cdecl Scr_CompileTextInternal(scriptInstance_t inst, const char *text, ScriptExpression_t *scriptExpr)
{
    const char *varUsagePos; // [esp+24h] [ebp-18h]
    char *start; // [esp+28h] [ebp-14h]
    char *end; // [esp+2Ch] [ebp-10h]
    HunkUser *user; // [esp+30h] [ebp-Ch]
    _BYTE *expr; // [esp+38h] [ebp-4h]

    if (!strcmp(text, "<locals>"))
    {
        scriptExpr->parseData = debugger_node1(inst, ENUM_local, 0);
    }
    else
    {
        Scr_InitAllocNode(inst);
        gScrCompilePub[inst].in_ptr = "*";
        gScrCompilePub[inst].parseBuf = text;
        if (gScrVarPub[inst].error_message
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                2121,
                0,
                "%s",
                "!gScrVarPub[inst].error_message"))
        {
            __debugbreak();
        }
        if (!gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                2123,
                0,
                "%s",
                "gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        ScriptParse(inst, &scriptExpr->parseData, 2u);
        if (gScrVarPub[inst].error_message)
        {
            if (*text)
                Com_PrintError(24, "%s\n", gScrVarPub[inst].error_message);
            Scr_ClearErrorMessage(inst);
            scriptExpr->parseData = debugger_node0(inst, ENUM_bad_expression);
            SL_ShutdownSystem(inst, 2u);
        }
        else
        {
            gScrCompilePub[inst].developer_statement = 3;
            expr = (_BYTE *)scriptExpr->parseData.stringValue;
            scriptExpr->parseData.stringValue = *(_DWORD *)(scriptExpr->parseData.stringValue + 4);
            if (*expr == 67)
            {
                varUsagePos = gScrVarPub[inst].varUsagePos;
                if (!varUsagePos)
                    gScrVarPub[inst].varUsagePos = "<debug expression>";
                Scr_CompileExpression(inst, &scriptExpr->parseData);
                gScrVarPub[inst].varUsagePos = varUsagePos;
                if (gScrVarPub[inst].error_message)
                {
                    if (!Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                        2149,
                        0,
                        "%s",
                        "!gScrVarPub[inst].error_message"))
                        __debugbreak();
                }
                SL_ShutdownSystem(inst, 2u);
            }
            else
            {
                if (*expr != 85
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                        2154,
                        0,
                        "%s",
                        "expr.node[0].type == ENUM_statement"))
                {
                    __debugbreak();
                }
                user = Hunk_UserCreate(0x10000, HU_SCHEME_DEBUG, 0, 0, "Scr_CompileTextInternal", 0);
                TempMemoryReset(user);
                start = TempMallocAlignStrict(0);
                Scr_CompileStatement(inst, scriptExpr->parseData);
                if (gScrVarPub[inst].error_message)
                {
                    Com_PrintError(24, "%s\n", gScrVarPub[inst].error_message);
                    Scr_ClearErrorMessage(inst);
                    scriptExpr->parseData = debugger_node0(inst, ENUM_bad_statement);
                    SL_ShutdownSystem(inst, 2u);
                }
                else
                {
                    end = TempMallocAlignStrict(0);
                    scriptExpr->parseData = debugger_buffer(inst, ENUM_statement, start, end - start, 32);
                }
                Hunk_UserDestroy(user);
            }
        }
        Scr_ShutdownAllocNode(inst);
    }
}

bool __cdecl Scr_EvalScriptExpression(
    scriptInstance_t inst,
    ScriptExpression_t *expr,
    unsigned int localId,
    VariableValue *value,
    bool freezeScope,
    bool freezeObjects)
{
    if (!expr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp", 2217, 0, "%s", "expr"))
    {
        __debugbreak();
    }
    gScrEvaluateGlob[inst].freezeScope = freezeScope;
    gScrEvaluateGlob[inst].freezeObjects = freezeObjects;
    gScrEvaluateGlob[inst].objectChanged = 0;
    Scr_EvalExpression(inst, expr->parseData, localId, value);
    return gScrEvaluateGlob[inst].objectChanged;
}

void __cdecl Scr_EvalExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value)
{
    switch (*(_BYTE *)expr.stringValue)
    {
    case 8:
        Scr_EvalPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        break;
    case 0x31:
        Scr_EvalBoolOrExpression(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            localId,
            value);
        break;
    case 0x32:
        Scr_EvalBoolAndExpression(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            localId,
            value);
        break;
    case 0x33:
        Scr_EvalBinaryOperatorExpression(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            *(sval_u *)(expr.stringValue + 12),
            localId,
            value);
        break;
    case 0x34:
        Scr_EvalExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        Scr_EvalBoolNot(inst, value);
        break;
    case 0x35:
        Scr_EvalExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        Scr_EvalBoolComplement(inst, value);
        break;
    case 0x51:
        Scr_EvalVector(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            *(sval_u *)(expr.stringValue + 12),
            localId,
            value);
        break;
    case 0x54:
        if (gScrEvaluateGlob[inst].freezeScope)
            localId = *(_DWORD *)(expr.stringValue + 4);
        if (localId && Scr_IsThreadAlive(localId, inst))
        {
            value->type = 1;
            value->u.intValue = localId;
            AddRefToObject(inst, localId);
        }
        else
        {
            localId = 0;
            value->type = 0;
            Scr_Error(inst, "thread not active", 0);
        }
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 4) != localId)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 4) = localId;
        }
        break;
    default:
        value->type = 0;
        Scr_Error(inst, "bad expression", 0);
        break;
    }
}

void __cdecl Scr_EvalPrimitiveExpression(
    scriptInstance_t inst,
    sval_u expr,
    unsigned int localId,
    VariableValue *value)
{
    VariableValue stringValue; // [esp+34h] [ebp-14h] BYREF
    VariableValue objectValue; // [esp+3Ch] [ebp-Ch] BYREF
    unsigned int selfId; // [esp+44h] [ebp-4h]

    switch (*(_BYTE *)expr.stringValue)
    {
    case 9:
        value->type = VAR_INTEGER;
        value->u.intValue = *(_DWORD *)(expr.stringValue + 4);
        break;
    case 0xA:
        value->type = VAR_FLOAT;
        value->u.floatValue = *(float *)(expr.stringValue + 4);
        break;
    case 0xB:
        value->type = VAR_INTEGER;
        value->u.intValue = -*(int *)(expr.stringValue + 4);
        break;
    case 0xC:
        value->type = VAR_FLOAT;
        //value->u.intValue = *(_DWORD *)(expr.stringValue + 4) ^ _mask__NegFloat_;
        value->u.floatValue = -expr.node[1].floatValue;
        break;
    case 0xD:
        value->type = 2;
        value->u.intValue = SL_GetString_(inst, *(char **)(expr.stringValue + 4), 0, 20);
        break;
    case 0xE:
        value->type = 3;
        value->u.intValue = SL_GetString_(inst, *(char **)(expr.stringValue + 4), 0, 20);
        break;
    case 0x13:
        Scr_EvalVariableExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        break;
    case 0x15:
        Scr_EvalCallExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        break;
    case 0x21:
        value->type = 0;
        break;
    case 0x22:
        if (gScrEvaluateGlob[inst].freezeScope)
            localId = *(_DWORD *)(expr.stringValue + 4);
        if (localId && Scr_IsThreadAlive(localId, inst))
        {
            selfId = Scr_GetSelf(inst, localId);
            value->type = 1;
            value->u.intValue = selfId;
            AddRefToObject(inst, value->u.intValue);
        }
        else
        {
            localId = 0;
            selfId = 0;
            value->type = 0;
            Scr_Error(inst, "thread not active", 0);
        }
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 4) != localId || *(_DWORD *)(expr.stringValue + 8) != selfId)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 4) = localId;
            *(_DWORD *)(expr.stringValue + 8) = selfId;
        }
        break;
    case 0x23:
        if (!*(_DWORD *)(expr.stringValue + 8)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                1228,
                0,
                "%s",
                "expr.node[2].idValue"))
        {
            __debugbreak();
        }
        value->type = 1;
        value->u.intValue = *(_DWORD *)(expr.stringValue + 8);
        AddRefToObject(inst, value->u.intValue);
        break;
    case 0x24:
        value->type = 1;
        value->u.intValue = gScrVarPub[inst].levelId;
        AddRefToObject(inst, gScrVarPub[inst].levelId);
        break;
    case 0x25:
        *value = Scr_EvalVariable(inst, gScrVarPub[inst].gameId);
        break;
    case 0x26:
        value->type = 1;
        value->u.intValue = gScrVarPub[inst].animId;
        AddRefToObject(inst, gScrVarPub[inst].animId);
        break;
    case 0x30:
        Scr_EvalExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        break;
    case 0x36:
        Scr_EvalPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        Scr_EvalSizeValue(inst, value);
        break;
    case 0x44:
        value->type = 0;
        Scr_Error(inst, "cannot evaluate []", 0);
        break;
    case 0x4A:
        value->type = 6;
        value->u.intValue = 0;
        break;
    case 0x4B:
        value->type = 6;
        value->u.intValue = 1;
        break;
    case 0x4D:
        Scr_EvalPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, &objectValue);
        Scr_EvalExpression(inst, *(sval_u *)(expr.stringValue + 8), localId, &stringValue);
        if (objectValue.type == 1
            && stringValue.type == 2
            && g_breakonObject == objectValue.u.intValue
            && g_breakonString == stringValue.u.intValue)
        {
            g_breakonHit = 1;
            ++*(_DWORD *)(expr.stringValue + 12);
        }
        RemoveRefToValue(inst, objectValue.type, objectValue.u);
        RemoveRefToValue(inst, stringValue.type, stringValue.u);
        value->type = 6;
        value->u.intValue = *(_DWORD *)(expr.stringValue + 12);
        break;
    case 0x56:
        value->type = 0;
        Scr_Error(inst, "bad expression", 0);
        break;
    default:
        return;
    }
}

void __cdecl Scr_EvalVariableExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value)
{
    unsigned int objectId; // [esp+14h] [ebp-4h]
    int objectIda; // [esp+14h] [ebp-4h]
    unsigned int objectIdb; // [esp+14h] [ebp-4h]

    switch (*(_BYTE *)expr.stringValue)
    {
    case 3:
        value->type = 0;
        Scr_Error(inst, "unknown variable", 0);
        break;
    case 5:
        if (gScrEvaluateGlob[inst].freezeScope)
            localId = *(_DWORD *)(expr.stringValue + 8);
        objectId = 0;
        if (localId && Scr_IsThreadAlive(localId, inst))
        {
            Scr_EvalLocalVariable(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
            if (!gScrVarPub[inst].error_message)
            {
                AddRefToValue(inst, value->type, value->u);
                objectId = Scr_EvalFieldObject(inst, *(_DWORD *)(expr.stringValue + 16), value);
                Scr_ClearErrorMessage(inst);
            }
        }
        else
        {
            localId = 0;
            value->type = 0;
            Scr_Error(inst, "thread not active", 0);
        }
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 8) != localId)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 8) = localId;
        }
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 12) != objectId)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 12) = objectId;
        }
        break;
    case 6:
        if (!*(_DWORD *)(expr.stringValue + 12))
            goto LABEL_29;
        value->type = 1;
        value->u.intValue = *(_DWORD *)(expr.stringValue + 12);
        AddRefToObject(inst, value->u.intValue);
        break;
    case 0xF:
        Scr_EvalArrayVariableExpression(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            localId,
            value);
        break;
    case 0x10:
        value->type = 0;
        Scr_Error(inst, "unknown field", 0);
        break;
    case 0x11:
        objectIda = Scr_EvalPrimitiveExpressionFieldObject(inst, *(sval_u *)(expr.stringValue + 4), localId);
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 12) != objectIda)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 12) = objectIda;
        }
        if (!objectIda)
            goto LABEL_29;
        Scr_EvalFieldVariableInternal(inst, objectIda, *(_DWORD *)(expr.stringValue + 8), value);
        break;
    case 0x12:
        if (*(_DWORD *)(expr.stringValue + 12))
        {
            Scr_EvalFieldVariableInternal(
                inst,
                *(_DWORD *)(expr.stringValue + 12),
                *(_DWORD *)(expr.stringValue + 8),
                value);
        }
        else
        {
        LABEL_29:
            value->type = 0;
            Scr_Error(inst, "unknown object", 0);
        }
        break;
    case 0x37:
        Scr_EvalPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, value);
        Scr_EvalSelfValue(inst, value);
        break;
    case 0x52:
        objectIdb = Scr_EvalObject(inst, *(sval_u *)(expr.stringValue + 4), *(sval_u *)(expr.stringValue + 8), value);
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 12) != objectIdb)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 12) = objectIdb;
        }
        break;
    case 0x53:
        if (*(_DWORD *)(expr.stringValue + 4) && Scr_IsThreadAlive(*(_DWORD *)(expr.stringValue + 4), inst))
        {
            value->u.intValue = *(_DWORD *)(expr.stringValue + 4);
            value->type = 1;
            AddRefToObject(inst, value->u.intValue);
        }
        else
        {
            if (*(_DWORD *)(expr.stringValue + 4))
            {
                RemoveRefToObject(inst, *(_DWORD *)(expr.stringValue + 4));
                *(_DWORD *)(expr.stringValue + 4) = 0;
            }
            value->type = 0;
            Scr_Error(inst, "thread not active", 0);
        }
        break;
    case 0x56:
        value->type = 0;
        Scr_Error(inst, "bad expression", 0);
        break;
    case 0x59:
        Scr_GetValue(inst, *(_DWORD *)(expr.stringValue + 4), value);
        break;
    default:
        return;
    }
}

void __cdecl Scr_EvalArrayVariableExpression(
                scriptInstance_t inst,
                sval_u array,
                sval_u index,
                unsigned int localId,
                VariableValue *value)
{
    VariableValue exprValue; // [esp+0h] [ebp-8h] BYREF

    Scr_EvalExpression(inst, index, localId, value);
    Scr_EvalPrimitiveExpression(inst, array, localId, &exprValue);
    Scr_EvalArrayVariableInternal(inst, &exprValue, value);
}

void __cdecl Scr_EvalLocalVariable(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value)
{
    unsigned int varId; // [esp+20h] [ebp-4h]

    if ( !expr.stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    544,
                    0,
                    "%s",
                    "expr.stringValue") )
    {
        __debugbreak();
    }
    varId = FindVariable(inst, localId, expr.stringValue);
    if ( varId )
    {
        *value = Scr_EvalVariable(inst, varId);
    }
    else
    {
        value->type = 0;
        Scr_Error(inst, "unknown variable", 0);
    }
}

unsigned int __cdecl Scr_EvalObject(scriptInstance_t inst, sval_u classnum, sval_u entnum, VariableValue *value)
{
    unsigned int objectId; // [esp+4h] [ebp-4h]

    objectId = FindEntityId(inst, entnum.stringValue, classnum.stringValue, 0);
    if ( objectId )
    {
        value->type = 1;
        value->u.intValue = objectId;
        AddRefToObject(inst, objectId);
        return objectId;
    }
    else
    {
        value->type = 0;
        Scr_Error(inst, "unknown object", 0);
        return 0;
    }
}

void __cdecl Scr_EvalSelfValue(scriptInstance_t inst, VariableValue *value)
{
    const char *v2; // eax
    signed int ObjectType; // [esp+0h] [ebp-Ch]
    VariableUnion threadId; // [esp+8h] [ebp-4h]

    if ( value->type != 1 )
        goto LABEL_8;
    threadId.intValue = value->u.intValue;
    ObjectType = GetObjectType(inst, value->u.intValue);
    if ( ObjectType < 13 )
        goto LABEL_8;
    if ( ObjectType <= 16 )
    {
        value->type = 1;
        value->u.stringValue = Scr_GetSelf(inst, threadId.stringValue);
        AddRefToObject(inst, value->u.intValue);
        RemoveRefToObject(inst, threadId.stringValue);
        return;
    }
    if ( ObjectType == 21 )
    {
        Scr_Error(inst, "thread not active", 0);
        Scr_ClearValue(inst, value);
    }
    else
    {
LABEL_8:
        v2 = va("self cannot be applied to %s", var_typename[value->type]);
        Scr_Error(inst, v2, 0);
        Scr_ClearValue(inst, value);
    }
}

void __cdecl Scr_GetValue(scriptInstance_t inst, unsigned int index, VariableValue *value)
{
    VariableValue *v3; // ecx
    int type; // eax
    const char *v5; // eax

    if (index >= gScrVmPub[inst].breakpointOutparamcount)
    {
        value->type = 0;
        v5 = va("parameter %d does not exist", index);
        Scr_Error(inst, v5, 0);
    }
    else
    {
        v3 = &gScrVmPub[inst].top[-(int)index];
        type = v3->type;
        value->u.intValue = v3->u.intValue;
        value->type = type;
        AddRefToValue(inst, value->type, value->u);
    }
}

int __cdecl Scr_EvalPrimitiveExpressionFieldObject(scriptInstance_t inst, sval_u expr, unsigned int localId)
{
    int result; // eax
    VariableValue value; // [esp+4h] [ebp-Ch] BYREF
    unsigned int selfId; // [esp+Ch] [ebp-4h]

    switch (*(_BYTE *)expr.stringValue)
    {
    case 0x13:
        Scr_EvalVariableExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, &value);
        result = Scr_EvalFieldObject(inst, *(_DWORD *)(expr.stringValue + 8), &value);
        break;
    case 0x15:
        Scr_EvalCallExpression(inst, *(sval_u *)(expr.stringValue + 4), localId, &value);
        result = Scr_EvalFieldObject(inst, *(_DWORD *)(expr.stringValue + 8), &value);
        break;
    case 0x22:
        if (gScrEvaluateGlob[inst].freezeScope)
            localId = *(_DWORD *)(expr.stringValue + 4);
        if (localId && Scr_IsThreadAlive(localId, inst))
        {
            selfId = Scr_GetSelf(inst, localId);
        }
        else
        {
            localId = 0;
            selfId = 0;
            Scr_Error(inst, "thread not active", 0);
        }
        if (gScrEvaluateGlob[inst].freezeObjects)
        {
            if (*(_DWORD *)(expr.stringValue + 4) != localId || *(_DWORD *)(expr.stringValue + 8) != selfId)
                gScrEvaluateGlob[inst].objectChanged = 1;
        }
        else
        {
            *(_DWORD *)(expr.stringValue + 4) = localId;
            *(_DWORD *)(expr.stringValue + 8) = selfId;
        }
        result = selfId;
        break;
    case 0x23:
        if (!*(_DWORD *)(expr.stringValue + 8)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                1028,
                0,
                "%s",
                "expr.node[2].idValue"))
        {
            __debugbreak();
        }
        result = *(_DWORD *)(expr.stringValue + 8);
        break;
    case 0x24:
        result = gScrVarPub[inst].levelId;
        break;
    case 0x26:
        result = gScrVarPub[inst].animId;
        break;
    default:
        Scr_Error(inst, "bad expression", 0);
        result = 0;
        break;
    }
    return result;
}

void __cdecl Scr_EvalCallExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value)
{
    if ( *(_BYTE *)expr.stringValue == 25 )
    {
        Scr_EvalFunction(inst, *(sval_u *)(expr.stringValue + 4), *(sval_u *)(expr.stringValue + 8), localId, value);
    }
    else if ( *(_BYTE *)expr.stringValue == 26 )
    {
        Scr_EvalMethod(
            inst,
            *(sval_u *)(expr.stringValue + 4),
            *(sval_u *)(expr.stringValue + 8),
            *(sval_u *)(expr.stringValue + 12),
            localId,
            value);
    }
}

void __cdecl Scr_EvalFunction(
                scriptInstance_t inst,
                sval_u func_name,
                sval_u params,
                unsigned int localId,
                VariableValue *value)
{
    Scr_PreEvalBuiltin(inst, params, localId);
    if ( (unsigned int)++g_script_error_level[inst] >= 0x21
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    1653,
                    0,
                    "g_script_error_level[inst] doesn't index ARRAY_COUNT( g_script_error[inst] )\n\t%i not in [0, %i)",
                    g_script_error_level[inst],
                    33) )
    {
        __debugbreak();
    }
    //if ( !_setjmp3(g_script_error[inst][g_script_error_level[inst]], 0) )
    if ( !_setjmp(g_script_error[inst][g_script_error_level[inst]]) )
        ((void (*)(void))func_name.stringValue)();
    if ( g_script_error_level[inst] < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    1658,
                    0,
                    "%s\n\t(g_script_error_level[inst]) = %i",
                    "(g_script_error_level[inst] >= 0)",
                    g_script_error_level[inst]) )
    {
        __debugbreak();
    }
    --g_script_error_level[inst];
    Scr_PostEvalBuiltin(inst, value);
}

void __cdecl Scr_PreEvalBuiltin(scriptInstance_t inst, sval_u params, unsigned int localId)
{
    sval_u *node; // [esp+0h] [ebp-Ch]
    unsigned int expr_count; // [esp+4h] [ebp-8h]
    int index; // [esp+8h] [ebp-4h]

    if (gScrVmPub[inst].outparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1586,
            0,
            "%s",
            "!gScrVmPub[inst].outparamcount"))
    {
        __debugbreak();
    }
    if (gScrVmPub[inst].inparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1587,
            0,
            "%s",
            "!gScrVmPub[inst].inparamcount"))
    {
        __debugbreak();
    }
    expr_count = GetExpressionCount(params);
    if (gScrVmPub[inst].top < gScrVmPub[inst].stack
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1590,
            0,
            "%s",
            "gScrVmPub[inst].top >= gScrVmPub[inst].stack"))
    {
        __debugbreak();
    }
    if (gScrVmPub[inst].top > gScrVmPub[inst].maxstack
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1591,
            0,
            "%s",
            "gScrVmPub[inst].top <= gScrVmPub[inst].maxstack"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].top += expr_count;
    if (gScrVmPub[inst].top > gScrVmPub[inst].maxstack
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1593,
            0,
            "%s",
            "gScrVmPub[inst].top <= gScrVmPub[inst].maxstack"))
    {
        __debugbreak();
    }
    index = 0;
    for (node = *(sval_u **)params.stringValue; node; node = node[1].node)
        Scr_EvalExpression(inst, *node, localId, &gScrVmPub[inst].top[-index++]);
    gScrVmPub[inst].outparamcount = expr_count;
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1605,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    if (gScrVmPub[inst].debugCode
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1608,
            0,
            "%s",
            "!gScrVmPub[inst].debugCode"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].debugCode = 1;
}

void __cdecl Scr_PostEvalBuiltin(scriptInstance_t inst, VariableValue *value)
{
    VariableValue *top; // edx
    int type; // ecx

    if (!gScrVmPub[inst].debugCode
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1621,
            0,
            "%s",
            "gScrVmPub[inst].debugCode"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].debugCode = 0;
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1624,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    Scr_ClearOutParams(inst);
    if (gScrVmPub[inst].inparamcount)
    {
        if (gScrVmPub[inst].inparamcount != 1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                1631,
                0,
                "%s",
                "gScrVmPub[inst].inparamcount == 1"))
        {
            __debugbreak();
        }
        gScrVmPub[inst].inparamcount = 0;
        top = gScrVmPub[inst].top;
        type = top->type;
        value->u.intValue = top->u.intValue;
        value->type = type;
        --gScrVmPub[inst].top;
    }
    else
    {
        value->type = 0;
    }
}

void __cdecl Scr_EvalMethod(
    scriptInstance_t inst,
    sval_u expr,
    sval_u func_name,
    sval_u params,
    unsigned int localId,
    VariableValue *value)
{
    const char *v6; // eax
    const char *v7; // eax
    int v8; // [esp-4h] [ebp-34h]
    scr_entref_t v9; // [esp+8h] [ebp-28h] BYREF
    scr_entref_t v10; // [esp+12h] [ebp-1Eh]
    scr_entref_t entref; // [esp+18h] [ebp-18h]
    unsigned int objectId; // [esp+20h] [ebp-10h]
    int type; // [esp+24h] [ebp-Ch]
    VariableValue objectValue; // [esp+28h] [ebp-8h] BYREF

    Scr_EvalPrimitiveExpression(inst, expr, localId, &objectValue);
    Scr_PreEvalBuiltin(inst, params, localId);
    if ((unsigned int)++g_script_error_level[inst] >= 0x21
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1683,
            0,
            "g_script_error_level[inst] doesn't index ARRAY_COUNT( g_script_error[inst] )\n\t%i not in [0, %i)",
            g_script_error_level[inst],
            33))
    {
        __debugbreak();
    }
    //if (!_setjmp3(g_script_error[inst][g_script_error_level[inst]], 0))
    if (!_setjmp(g_script_error[inst][g_script_error_level[inst]]))
    {
        if (objectValue.type != 1)
        {
            type = objectValue.type;
            RemoveRefToValue(inst, objectValue.type, objectValue.u);
            gScrVarPub[inst].error_index = -1;
            v6 = va("%s is not an entity", var_typename[type]);
            Scr_Error(inst, v6, 0);
        }
        objectId = objectValue.u.intValue;
        if (GetObjectType(inst, objectValue.u.stringValue) != 19)
        {
            type = GetObjectType(inst, objectId);
            RemoveRefToObject(inst, objectId);
            gScrVarPub[inst].error_index = -1;
            v7 = va("%s is not an entity", var_typename[type]);
            Scr_Error(inst, v7, 0);
        }
        entref = Scr_GetEntityIdRef(inst, objectId);
        RemoveRefToObject(inst, objectId);
        //LOWORD(v8) = entref.client;
        ((void(__cdecl *)(_DWORD, int))func_name.stringValue)(*(_DWORD *)&entref.entnum, entref.client);
    }
    if (g_script_error_level[inst] < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
            1711,
            0,
            "%s\n\t(g_script_error_level[inst]) = %i",
            "(g_script_error_level[inst] >= 0)",
            g_script_error_level[inst]))
    {
        __debugbreak();
    }
    --g_script_error_level[inst];
    Scr_PostEvalBuiltin(inst, value);
}

void __cdecl Scr_EvalBoolOrExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                unsigned int localId,
                VariableValue *value)
{
    bool v5; // [esp+0h] [ebp-1Ch]
    bool v6; // [esp+4h] [ebp-18h]

    Scr_EvalExpression(inst, expr1, localId, value);
    Scr_CastBool(inst, value);
    v6 = value->type == 6 && value->u.intValue;
    RemoveRefToValue(inst, value->type, value->u);
    Scr_EvalExpression(inst, expr2, localId, value);
    Scr_CastBool(inst, value);
    v5 = v6 || value->type == 6 && value->u.intValue;
    RemoveRefToValue(inst, value->type, value->u);
    value->type = 6;
    value->u.intValue = v5;
}

void __cdecl Scr_EvalBoolAndExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                unsigned int localId,
                VariableValue *value)
{
    bool v5; // [esp+0h] [ebp-1Ch]
    bool v6; // [esp+4h] [ebp-18h]

    Scr_EvalExpression(inst, expr1, localId, value);
    Scr_CastBool(inst, value);
    v6 = value->type == 6 && value->u.intValue;
    RemoveRefToValue(inst, value->type, value->u);
    Scr_EvalExpression(inst, expr2, localId, value);
    Scr_CastBool(inst, value);
    v5 = v6 && value->type == 6 && value->u.intValue;
    RemoveRefToValue(inst, value->type, value->u);
    value->type = 6;
    value->u.intValue = v5;
}

void __cdecl Scr_EvalBinaryOperatorExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u opcode,
                unsigned int localId,
                VariableValue *value)
{
    VariableValue value2; // [esp+0h] [ebp-8h] BYREF

    Scr_EvalExpression(inst, expr1, localId, value);
    Scr_EvalExpression(inst, expr2, localId, &value2);
    Scr_EvalBinaryOperator(inst, opcode.intValue, value, &value2);
}

void __cdecl Scr_EvalVector(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr3,
                unsigned int localId,
                VariableValue *value)
{
    VariableValue vectorValue[3]; // [esp+0h] [ebp-18h] BYREF

    Scr_EvalExpression(inst, expr1, localId, vectorValue);
    Scr_EvalExpression(inst, expr2, localId, &vectorValue[1]);
    Scr_EvalExpression(inst, expr3, localId, &vectorValue[2]);
    Scr_CastVector(inst, vectorValue);
    *value = vectorValue[0];
}

void __cdecl Scr_ClearDebugExprValue(scriptInstance_t inst, sval_u val)
{
    switch ( *(_BYTE *)val.stringValue )
    {
        case 5:
        case 6:
            if ( !*(unsigned int *)(val.stringValue + 16)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                            2239,
                            0,
                            "%s",
                            "val.node[4].idValue") )
            {
                __debugbreak();
            }
            ClearVariableValue(inst, *(unsigned int *)(val.stringValue + 16));
            break;
        case 0x13:
        case 0x15:
            if ( !*(unsigned int *)(val.stringValue + 8)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                            2245,
                            0,
                            "%s",
                            "val.node[2].idValue") )
            {
                __debugbreak();
            }
            ClearVariableValue(inst, *(unsigned int *)(val.stringValue + 8));
            break;
        default:
            return;
    }
}

bool __cdecl Scr_RefScriptExpression(scriptInstance_t inst, ScriptExpression_t *expr)
{
    bool result; // [esp+3h] [ebp-1h]

    if ( !expr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp", 2262, 0, "%s", "expr") )
    {
        __debugbreak();
    }
    result = Scr_RefExpression(inst, expr->parseData);
    if ( !expr->exprHead
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                    2265,
                    0,
                    "%s",
                    "expr->exprHead") )
    {
        __debugbreak();
    }
    Scr_ClearDebugExpr(inst, expr->exprHead);
    return result;
}

bool __cdecl Scr_RefExpression(scriptInstance_t inst, sval_u expr)
{
    bool result; // al

    switch ( *(_BYTE *)expr.stringValue )
    {
        case 8:
            result = Scr_RefPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x31:
        case 0x32:
        case 0x33:
            result = Scr_RefBinaryOperatorExpression(
                                 inst,
                                 *(sval_u *)(expr.stringValue + 4),
                                 *(sval_u *)(expr.stringValue + 8));
            break;
        case 0x34:
        case 0x35:
            result = Scr_RefExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x51:
            result = Scr_RefVector(
                                 inst,
                                 *(sval_u *)(expr.stringValue + 4),
                                 *(sval_u *)(expr.stringValue + 8),
                                 *(sval_u *)(expr.stringValue + 12));
            break;
        case 0x54:
            result = Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 4), 1);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl Scr_RefPrimitiveExpression(scriptInstance_t inst, sval_u expr)
{
    bool result; // al

    switch ( *(_BYTE *)expr.stringValue )
    {
        case 0x13:
            result = Scr_RefVariableExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x15:
            result = Scr_RefCallExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x22:
            if ( !Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 4), 1) )
                goto $LN4_194;
            if ( *(unsigned int *)(expr.stringValue + 8) )
            {
                *(_BYTE *)expr.stringValue = 35;
                goto $LN4_194;
            }
            *(unsigned int *)(expr.stringValue + 4) = 0;
            result = 1;
            break;
        case 0x23:
$LN4_194:
            result = Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 8), 1);
            break;
        case 0x25:
            result = Scr_RefToVariable(inst, gScrVarPub[inst].gameId, 0);
            break;
        case 0x30:
            result = Scr_RefExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x36:
            result = Scr_RefPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x4D:
            result = Scr_RefBreakonExpression(inst, *(sval_u *)(expr.stringValue + 4), *(sval_u *)(expr.stringValue + 8));
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl Scr_RefVariableExpression(scriptInstance_t inst, sval_u expr)
{
    bool result; // al

    switch ( *(_BYTE *)expr.stringValue )
    {
        case 5:
            if ( !Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 8), 1) )
                goto $LN11_83;
            if ( *(unsigned int *)(expr.stringValue + 12) )
            {
                *(_BYTE *)expr.stringValue = 6;
                goto $LN11_83;
            }
            *(unsigned int *)(expr.stringValue + 8) = 0;
            result = 1;
            break;
        case 6:
$LN11_83:
            if ( Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 12), 1) )
            {
                *(unsigned int *)(expr.stringValue + 12) = 0;
                result = 1;
            }
            else
            {
                result = 0;
            }
            break;
        case 0xF:
            result = Scr_RefArrayVariableExpression(
                                 inst,
                                 *(sval_u *)(expr.stringValue + 4),
                                 *(sval_u *)(expr.stringValue + 8));
            break;
        case 0x11:
            if ( !Scr_RefPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4)) )
                goto $LN5_156;
            if ( *(unsigned int *)(expr.stringValue + 12) )
            {
                *(_BYTE *)expr.stringValue = 18;
                goto $LN5_156;
            }
            result = 1;
            break;
        case 0x12:
$LN5_156:
            if ( Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 12), 1) )
            {
                *(unsigned int *)(expr.stringValue + 12) = 0;
                result = 1;
            }
            else
            {
                result = 0;
            }
            break;
        case 0x37:
            result = Scr_RefPrimitiveExpression(inst, *(sval_u *)(expr.stringValue + 4));
            break;
        case 0x52:
            result = Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 12), 1);
            break;
        case 0x53:
            result = Scr_RefToVariable(inst, *(unsigned int *)(expr.stringValue + 4), 1);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl Scr_RefArrayVariableExpression(scriptInstance_t inst, sval_u array, sval_u index)
{
    bool arrayRemoved; // [esp+6h] [ebp-2h]
    bool indexRemoved; // [esp+7h] [ebp-1h]

    indexRemoved = Scr_RefExpression(inst, index);
    arrayRemoved = Scr_RefPrimitiveExpression(inst, array);
    return indexRemoved || arrayRemoved;
}

bool __cdecl Scr_RefBreakonExpression(scriptInstance_t inst, sval_u expr, sval_u param)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefPrimitiveExpression(inst, expr);
    expr2Removed = Scr_RefExpression(inst, param);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefCallExpression(scriptInstance_t inst, sval_u expr)
{
    if ( *(_BYTE *)expr.stringValue == 25 )
        return Scr_RefCall(inst, *(sval_u *)(expr.stringValue + 8));
    if ( *(_BYTE *)expr.stringValue == 26 )
        return Scr_RefMethod(inst, *(sval_u *)(expr.stringValue + 4), *(sval_u *)(expr.stringValue + 12));
    return 0;
}

bool __cdecl Scr_RefCall(scriptInstance_t inst, sval_u params)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    bool exprRemoved; // [esp+7h] [ebp-1h]

    exprRemoved = 0;
    for ( node = *(sval_u **)params.stringValue; node; node = node[1].node )
    {
        if ( Scr_RefExpression(inst, *node) )
            exprRemoved = 1;
    }
    return exprRemoved;
}

bool __cdecl Scr_RefMethod(scriptInstance_t inst, sval_u expr, sval_u params)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefPrimitiveExpression(inst, expr);
    expr2Removed = Scr_RefCall(inst, params);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefBinaryOperatorExpression(scriptInstance_t inst, sval_u expr1, sval_u expr2)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefExpression(inst, expr1);
    expr2Removed = Scr_RefExpression(inst, expr2);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefVector(scriptInstance_t inst, sval_u expr1, sval_u expr2, sval_u expr3)
{
    bool expr1Removed; // [esp+5h] [ebp-3h]
    bool expr2Removed; // [esp+6h] [ebp-2h]
    bool expr3Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefExpression(inst, expr1);
    expr2Removed = Scr_RefExpression(inst, expr2);
    expr3Removed = Scr_RefExpression(inst, expr3);
    return expr1Removed || expr2Removed || expr3Removed;
}

void __cdecl Scr_FreeDebugExprValue(scriptInstance_t inst, sval_u val)
{
    switch ( *(_BYTE *)val.stringValue )
    {
        case 5:
        case 6:
            if ( !*(unsigned int *)(val.stringValue + 16)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                            2291,
                            0,
                            "%s",
                            "val.node[4].idValue") )
            {
                __debugbreak();
            }
            FreeValue(inst, *(unsigned int *)(val.stringValue + 16));
            break;
        case 0x13:
        case 0x15:
            if ( !*(unsigned int *)(val.stringValue + 8)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_evaluate.cpp",
                            2297,
                            0,
                            "%s",
                            "val.node[2].idValue") )
            {
                __debugbreak();
            }
            FreeValue(inst, *(unsigned int *)(val.stringValue + 8));
            break;
        case 0x53:
            if ( *(unsigned int *)(val.stringValue + 4) )
            {
                RemoveRefToObject(inst, *(unsigned int *)(val.stringValue + 4));
                *(unsigned int *)(val.stringValue + 4) = 0;
            }
            break;
        default:
            return;
    }
}

