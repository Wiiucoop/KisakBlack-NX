#include "cscr_compiler.h"
#include "cscr_codepos.h"   // nx-port: pointer-sized code positions
#include <cgame/cg_scr_main.h>
#include "cscr_tempmemory.h"
#include "cscr_parsetree.h"
#include <universal/com_memory.h>
#include "cscr_stringlist.h"
#include "cscr_parser.h"
#include "cscr_evaluate.h"
#include "cscr_vm.h"

#undef GetObject // what a shit horrible macro that ruins every project

scrCompilePub_t gScrCompilePub[2];
scrCompileGlob_t gScrCompileGlob[2];

void (__cdecl *__cdecl GetFunction(scriptInstance_t inst, const char **pName, int *type))()
{
    if ( inst )
        return CScr_GetFunction(pName, type);
    else
        return Scr_GetFunction(pName, type);
}

void (__cdecl *__cdecl GetMethod(scriptInstance_t inst, const char **pName, int *type))(scr_entref_t)
{
    void (__cdecl *method)(scr_entref_t);

    if ( inst )
        method = CScr_GetMethod(pName, type);
    else
        method = Scr_GetMethod(pName, type);

    return method;
}

int __cdecl GetExpressionCount(sval_u exprlist)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    int expr_count; // [esp+4h] [ebp-4h]

    expr_count = 0;
    for ( node = exprlist.node[0].node; node; node = node[1].node )
        ++expr_count;
    return expr_count;
}

unsigned int __cdecl Scr_GetBuiltin(scriptInstance_t inst, sval_u func_name)
{
    sval_u func_namea; // [esp+Ch] [ebp+Ch]
    sval_u func_nameb; // [esp+Ch] [ebp+Ch]

    if ( *(_BYTE *)func_name.node != 28 )
        return 0;
    func_namea = func_name.node[1];
    if ( *(_BYTE *)func_namea.node != 20 )
        return 0;
    func_nameb = func_namea.node[1];
    if ( *(_BYTE *)func_nameb.node != 22 )
        return 0;
    if ( gScrCompilePub[inst].developer_statement == 3
        || !FindVariable(inst, gScrCompileGlob[inst].filePosId, func_nameb.node[1].stringValue) )
    {
        return func_nameb.node[1].stringValue;
    }
    return 0;
}

int __cdecl CompareCaseInfo(unsigned int *elem1, unsigned int *elem2)
{
    // nx-port: switch table entries are packed and unaligned (cscr_codepos.h)
    unsigned int name1 = Scr_CaseEntryName((const char *)elem1);
    unsigned int name2 = Scr_CaseEntryName((const char *)elem2);
    if ( name1 <= name2 )
        return name1 < name2;
    else
        return -1;
}

void __cdecl ScriptCompile(
                scriptInstance_t inst,
                sval_u val,
                unsigned int filePosId,
                unsigned int fileCountId,
                unsigned int scriptId,
                PrecacheEntry *entries,
                int entriesCount)
{
    char *v7; // eax
    char *v8; // eax
    unsigned int Variable; // eax
    PrecacheEntry *v9; // [esp+0h] [ebp-98h]
    int j; // [esp+48h] [ebp-50h]
    VariableValue pos; // [esp+4Ch] [ebp-4Ch] BYREF
    unsigned __int16 filename; // [esp+54h] [ebp-44h]
    PrecacheEntry *precachescript; // [esp+58h] [ebp-40h]
    int far_function_count; // [esp+5Ch] [ebp-3Ch]
    PrecacheEntry *precachescript2; // [esp+60h] [ebp-38h]
    unsigned int includeFilePosId; // [esp+64h] [ebp-34h]
    unsigned int toPosId; // [esp+68h] [ebp-30h]
    unsigned int posId; // [esp+6Ch] [ebp-2Ch]
    VariableValue includePos; // [esp+70h] [ebp-28h] BYREF
    unsigned __int16 name; // [esp+78h] [ebp-20h]
    unsigned int includePosId; // [esp+7Ch] [ebp-1Ch]
    PrecacheEntry *precachescriptList; // [esp+80h] [ebp-18h]
    int i; // [esp+84h] [ebp-14h]
    unsigned int toThreadCountId; // [esp+88h] [ebp-10h]
    unsigned int duplicateFilePosId; // [esp+8Ch] [ebp-Ch]
    VariableValue value; // [esp+90h] [ebp-8h] BYREF
    int entriesCount_; // [esp+B8h] [ebp+20h]

    gScrCompileGlob[inst].filePosId = filePosId;
    gScrCompileGlob[inst].fileCountId = fileCountId;
    gScrCompileGlob[inst].bConstRefCount = 0;
    gScrAnimPub[inst].animTreeIndex = 0;
    gScrCompilePub[inst].developer_statement = 0;
    if ( gScrCompilePub[inst].far_function_count )
        v9 = &entries[entriesCount];
    else
        v9 = 0;
    precachescriptList = v9;
    entriesCount_ = gScrCompilePub[inst].far_function_count + entriesCount;
    if ( entriesCount_ > MAX_PRECACHE_ENTRIES)
        Com_Error(
            ERR_DROP,
            "MAX_PRECACHE_ENTRIES exceeded.\nThis means that the script recursion is too deep.\nPlease see a coder.");
    gScrCompileGlob[inst].precachescriptList = precachescriptList;
    EmitIncludeList(inst, *val.node);
    EmitThreadList(inst, val.node[1]);
    gScrCompilePub[inst].programLen = TempMallocAlignStrict(0) - gScrVarPub[inst].programBuffer;
    Scr_ShutdownAllocNode(inst);
    Hunk_ClearTempMemoryHigh();
    far_function_count = gScrCompileGlob[inst].precachescriptList - precachescriptList;
    duplicateFilePosId = AllocObject(inst);

    for ( i = 0; i < far_function_count; ++i )
    {
        precachescript = &precachescriptList[i];
        filename = precachescript->filename;
        v7 = SL_ConvertToString(filename, inst);
        includeFilePosId = Scr_LoadScriptInternal(inst, v7, entries, entriesCount_);
        if ( !includeFilePosId )
        {
            v8 = SL_ConvertToString(filename, inst);
            CompileError(inst, precachescript->sourcePos, "Could not find script '%s'", v8);
            return;
        }
        SL_RemoveRefToString(inst, filename);
        if ( precachescript->include )
        {
            for ( j = i + 1; j < far_function_count; ++j )
            {
                precachescript2 = &precachescriptList[j];
                if ( !precachescript2->include )
                    break;
                if ( precachescript2->filename == filename )
                {
                    CompileError(inst, precachescript2->sourcePos, "Duplicate #include");
                    return;
                }
            }

            precachescript->include = 0;
            pos.type = VAR_CODEPOS;

            for ( includePosId = FindFirstSibling(inst, includeFilePosId);
                        includePosId;
                        includePosId = FindNextSibling(inst, includePosId) )
            {
                includePos = Scr_EvalVariable(inst, includePosId);

                if ( includePos.type != 7
                    && includePos.type != 12
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                                4858,
                                0,
                                "%s",
                                "(includePos.type == VAR_CODEPOS) || (includePos.type == VAR_DEVELOPER_CODEPOS)") )
                {
                    __debugbreak();
                }

                name = GetVariableName(inst, includePosId);
                posId = FindVariable(inst, filePosId, name);
                if ( posId )
                    CheckThreadPosition(inst, posId, name, precachescript->sourcePos);
                toPosId = GetVariable(inst, duplicateFilePosId, name);
                CheckThreadPosition(inst, toPosId, name, precachescript->sourcePos);
                pos.u = GetVariableValueAddress(inst, includePosId)->u;   // nx-port: a code position, whole
                SetNewVariableValue(inst, toPosId, &pos);
                Variable = GetVariable(inst, fileCountId, name);
                toThreadCountId = GetObject(inst, Variable);
                LinkThread(inst, toThreadCountId, &includePos, 0);
                RemoveVariable(inst, fileCountId, name);
            }
        }
    }
    ClearObject(inst, duplicateFilePosId);
    RemoveRefToEmptyObject(inst, duplicateFilePosId);
    LinkFile(inst, filePosId, fileCountId);
    value.type = 6;
    SetVariableValue(inst, scriptId, &value);
}

void __cdecl LinkThread(scriptInstance_t inst, unsigned int threadCountId, VariableValue *pos, bool allowFarCall)
{
    VariableValue count; // [esp+18h] [ebp-24h]
    unsigned int valueId; // [esp+20h] [ebp-1Ch]
    unsigned int countId; // [esp+24h] [ebp-18h]
    unsigned int type; // [esp+28h] [ebp-14h]
    int i; // [esp+2Ch] [ebp-10h]
    VariableValueInternal_u *value; // [esp+38h] [ebp-4h]

    countId = FindVariable(inst, threadCountId, 0);
    if (countId)
    {
        count = Scr_EvalVariable(inst, countId);
        if (count.type != 6
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                2135,
                0,
                "%s",
                "count.type == VAR_INTEGER"))
        {
            __debugbreak();
        }
        for (i = 0; i < count.u.intValue; ++i)
        {
            valueId = FindVariable(inst, threadCountId, i + 1);
            if (!valueId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2141,
                    0,
                    "%s",
                    "valueId"))
            {
                __debugbreak();
            }
            value = GetVariableValueAddress(inst, valueId);
            type = GetValueType(inst, valueId);
            if (type != 7
                && type != 12
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2144,
                    0,
                    "%s",
                    "type == VAR_CODEPOS || type == VAR_DEVELOPER_CODEPOS"))
            {
                __debugbreak();
            }
            if (pos->type == 12)
            {
                if (!gScrVarPub[inst].developer_script
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        2148,
                        0,
                        "%s",
                        "gScrVarPub[inst].developer_script"))
                {
                    __debugbreak();
                }
                if (type == 7)
                {
                    CompileError2(inst, (char *)value->u.codePosValue, "normal script cannot reference a function in a /# ... #/ comment");
                    return;
                }
            }
            if (!pos->type || !allowFarCall && (uintptr_t)Scr_ReadCodePosAt(value->u.codePosValue) == 1)
            {
                CompileError2(inst, (char *)value->u.codePosValue, "unknown function");
                return;
            }
            Scr_WriteCodePosAt((char *)value->u.codePosValue, pos->u.codePosValue);   // nx-port: patch the call-target slot
            RemoveVariable(inst, threadCountId, i + 1);
        }
        RemoveVariable(inst, threadCountId, 0);
    }
}

void __cdecl LinkFile(scriptInstance_t inst, unsigned int filePosId, unsigned int fileCountId)
{
    unsigned int threadCountId; // [esp+38h] [ebp-20h]
    VariableValue pos; // [esp+3Ch] [ebp-1Ch] BYREF
    unsigned int threadCountPtr; // [esp+44h] [ebp-14h]
    unsigned int posId; // [esp+48h] [ebp-10h]
    VariableValue emptyValue; // [esp+4Ch] [ebp-Ch] BYREF
    unsigned int nameId; // [esp+54h] [ebp-4h]

    emptyValue.type = 0;
    emptyValue.u.intValue = 0;
    for ( threadCountPtr = FindFirstSibling(inst, fileCountId);
                threadCountPtr;
                threadCountPtr = FindNextSibling(inst, threadCountPtr) )
    {
        threadCountId = FindObject(inst, threadCountPtr);
        if ( !threadCountId
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        2193,
                        0,
                        "%s",
                        "threadCountId") )
        {
            __debugbreak();
        }
        nameId = GetVariableName(inst, threadCountPtr);
        posId = FindVariable(inst, filePosId, nameId);
        if ( posId )
        {
            pos = Scr_EvalVariable(inst, posId);
            if ( pos.type != 7
                && pos.type != 12
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            2202,
                            0,
                            "%s",
                            "pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS") )
            {
                __debugbreak();
            }
            if ( !pos.u.intValue
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            2203,
                            0,
                            "%s",
                            "pos.u.codePosValue") )
            {
                __debugbreak();
            }
            LinkThread(inst, threadCountId, &pos, 1);
        }
        else
        {
            LinkThread(inst, threadCountId, &emptyValue, 1);
        }
    }
    ClearObject(inst, fileCountId);
}

void __cdecl CheckThreadPosition(scriptInstance_t inst, unsigned int posId, unsigned int name, unsigned int sourcePos)
{
    char *v4; // eax
    char *v5; // eax
    char *buf; // [esp-4h] [ebp-30h]
    VariableValue pos; // [esp+20h] [ebp-Ch]

    if ( gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2220,
                    0,
                    "%s",
                    "!gScrVarPub[inst].evaluate") )
    {
        __debugbreak();
    }
    pos = Scr_EvalVariable(inst, posId);
    if ( pos.type )
    {
        if ( pos.u.codePosValue )   // nx-port: the whole pointer
        {
            buf = gScrParserPub[inst].sourceBufferLookup[Scr_GetSourceBuffer(inst, pos.u.codePosValue)].buf;
            v4 = SL_ConvertToString(name, inst);
            CompileError(inst, sourcePos, "function '%s' already defined in '%s'", v4, buf);
        }
        else
        {
            v5 = SL_ConvertToString(name, inst);
            CompileError(inst, sourcePos, "function '%s' already defined", v5);
        }
    }
}

void __cdecl EmitThreadList(scriptInstance_t inst, sval_u val)
{
    sval_u *node; // [esp+0h] [ebp-4h]
    sval_u *nodea; // [esp+0h] [ebp-4h]

    gScrCompileGlob[inst].in_developer_thread = 0;
    for ( node = val.node[0].node[1].node; node; node = node[1].node )
        SpecifyThread(inst, *node);
    if ( gScrCompileGlob[inst].in_developer_thread )
        CompileError(inst, gScrCompileGlob[inst].developer_thread_sourcePos, "/# has no matching #/");
    gScrCompileGlob[inst].firstThread[0] = 1;
    gScrCompileGlob[inst].firstThread[1] = 1;
    if ( gScrCompileGlob[inst].in_developer_thread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    4742,
                    0,
                    "%s",
                    "!gScrCompileGlob[inst].in_developer_thread") )
    {
        __debugbreak();
    }
    for ( nodea = val.node[0].node[1].node; nodea; nodea = nodea[1].node )
        EmitThread(inst, *nodea);
    if ( gScrCompileGlob[inst].in_developer_thread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    4747,
                    0,
                    "%s",
                    "!gScrCompileGlob[inst].in_developer_thread") )
    {
        __debugbreak();
    }
}

void __cdecl SpecifyThread(scriptInstance_t inst, sval_u val)
{
    unsigned int Variable; // eax
    unsigned int v3; // [esp-Ch] [ebp-10h]
    unsigned int v4; // [esp-8h] [ebp-Ch]
    int v5; // [esp-4h] [ebp-8h]
    char v6; // [esp+0h] [ebp-4h]

    v6 = *(_BYTE *)val.node;
    if ( *(_BYTE *)val.node == ENUM_thread )
    {
        if ( !gScrCompileGlob[inst].in_developer_thread || gScrVarPub[inst].developer_script )
        {
            v5 = gScrCompileGlob[inst].in_developer_thread ? 12 : 7;
            v4 = val.node[4].stringValue;
            v3 = val.node[1].stringValue;
            Variable = GetVariable(inst, gScrCompileGlob[inst].filePosId, v3);
            SpecifyThreadPosition(inst, Variable, v3, v4, v5);
        }
    }
    else if ( v6 == ENUM_begin_developer_thread )
    {
        if ( gScrCompileGlob[inst].in_developer_thread )
        {
            CompileError(inst, val.node[1].stringValue, "cannot recurse /#");
        }
        else
        {
            gScrCompileGlob[inst].in_developer_thread = 1;
            gScrCompileGlob[inst].developer_thread_sourcePos = val.node[1].stringValue;
        }
    }
    else if ( v6 == ENUM_end_developer_thread )
    {
        if ( gScrCompileGlob[inst].in_developer_thread )
            gScrCompileGlob[inst].in_developer_thread = 0;
        else
            CompileError(inst, val.node[1].stringValue, "#/ has no matching /#");
    }
}

unsigned int __cdecl SpecifyThreadPosition(
                scriptInstance_t inst,
                unsigned int posId,
                unsigned int name,
                unsigned int sourcePos,
                int type)
{
    VariableValue pos; // [esp+0h] [ebp-8h] BYREF

    CheckThreadPosition(inst, posId, name, sourcePos);
    pos.type = type;
    pos.u.intValue = 0;
    SetNewVariableValue(inst, posId, &pos);
    return posId;
}

void __cdecl EmitThread(scriptInstance_t inst, sval_u val)
{
    char *v2; // eax
    unsigned int v3; // [esp-4h] [ebp-8h]

    switch ( *(_BYTE *)val.node )
    {
        case ENUM_thread:
            Scr_CalcLocalVarsThread(
                inst,
                val.node[2],
                val.node[3],
                &val.node[6]);
            if ( gScrCompileGlob[inst].in_developer_thread )
                EmitDeveloperThread(inst, val, &val.node[6]);
            else
                EmitNormalThread(inst, val, &val.node[6]);
            break;
        case ENUM_begin_developer_thread:
            if ( gScrCompileGlob[inst].in_developer_thread
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            4698,
                            0,
                            "%s",
                            "!gScrCompileGlob[inst].in_developer_thread") )
            {
                __debugbreak();
            }
            gScrCompileGlob[inst].in_developer_thread = 1;
            break;
        case ENUM_end_developer_thread:
            if ( !gScrCompileGlob[inst].in_developer_thread
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            4703,
                            0,
                            "%s",
                            "gScrCompileGlob[inst].in_developer_thread") )
            {
                __debugbreak();
            }
            gScrCompileGlob[inst].in_developer_thread = 0;
            break;
        case ENUM_usingtree:
            if ( gScrCompileGlob[inst].in_developer_thread )
            {
                CompileError(inst, val.node[2].stringValue, "cannot put #using_animtree inside /# ... #/ comment");
            }
            else
            {
                v3 = val.node[3].stringValue;
                v2 = SL_ConvertToString(val.node[1].stringValue, inst);
                Scr_UsingTree(inst, v2, v3);
                Scr_CompileRemoveRefToString(inst, val.node[1].stringValue);
            }
            break;
        default:
            return;
    }
}

void __cdecl Scr_CompileRemoveRefToString(scriptInstance_t inst, unsigned int stringValue)
{
    if (!stringValue
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            104,
            0,
            "%s",
            "stringValue"))
    {
        __debugbreak();
    }
    if (!gScrCompileGlob[inst].bConstRefCount && gScrCompilePub[inst].developer_statement != 3)
        SL_RemoveRefToString(inst, stringValue);
}

void __cdecl Scr_CalcLocalVarsThread(scriptInstance_t inst, sval_u exprlist, sval_u stmtlist, sval_u *stmttblock)
{
    unsigned int *v4; // eax

    gScrCompileGlob[inst].forceNotCreate = 0;
    // nx-port: a scr_block_s, not parse-tree nodes; its fields by name (the
    // x86 form wrote words 0..5 of it through stringValue)
    stmttblock->block = (scr_block_s *)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s), "Scr_CalcLocalVarsThread");
    stmttblock->block->abortLevel = 0;
    stmttblock->block->localVarsCreateCount = 0;
    stmttblock->block->localVarsCount = 0;
    stmttblock->block->localVarsPublicCount = 0;
    memset(stmttblock->block->localVarsInitBits, 0, sizeof(stmttblock->block->localVarsInitBits));
    (void)v4;
    Scr_CalcLocalVarsFormalParameterList(exprlist, stmttblock->block);
    Scr_CalcLocalVarsStatementList(inst, stmtlist, stmttblock->block);
}

void __cdecl Scr_CalcLocalVarsStatementList(scriptInstance_t inst, sval_u val, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    for ( node = val.node[0].node[1].node; node; node = node[1].node )
        Scr_CalcLocalVarsStatement(inst, *node, block);
}

void __cdecl Scr_CalcLocalVarsStatement(scriptInstance_t inst, sval_u val, scr_block_s *block)
{
    switch ( *(_BYTE *)val.node )
    {
        case 2:
            Scr_CalcLocalVarsAssignmentStatement(val.node[1], val.node[2], block);
            break;
        case 0x1D:
        case 0x1E:
            if ( !block->abortLevel )
                block->abortLevel = 3;
            break;
        case 0x27:
            Scr_CalcLocalVarsIfStatement(inst, val.node[2], block, &val.node[4]);
            break;
        case 0x28:
            Scr_CalcLocalVarsIfElseStatement(
                inst,
                val.node[2],
                val.node[3],
                block,
                &val.node[6],
                &val.node[7]);
            break;
        case 0x29:
            Scr_CalcLocalVarsWhileStatement(
                inst,
                val.node[1],
                val.node[2],
                block,
                &val.node[5]);
            break;
        case 0x2A:
            Scr_CalcLocalVarsForStatement(
                inst,
                val.node[1],
                val.node[2],
                val.node[3],
                val.node[4],
                block,
                &val.node[7],
                &val.node[8]);
            break;
        case 0x2B:
        case 0x2C:
        case 0x2D:
            Scr_CalcLocalVarsIncStatement(val.node[1], block);
            break;
        case 0x2E:
            Scr_CalcLocalVarsStatementList(inst, val.node[1], block);
            break;
        case 0x2F:
            Scr_CalcLocalVarsDeveloperStatementList(
                inst,
                val.node[1],
                block,
                &val.node[3]);
            break;
        case 0x39:
            Scr_CalcLocalVarsWaittillStatement(val.node[2], block);
            break;
        case 0x3E:
            Scr_CalcLocalVarsSwitchStatement(inst, val.node[2], block);
            break;
        case 0x41:
            Scr_AddBreakBlock(inst, block);
            if ( !block->abortLevel )
                block->abortLevel = 2;
            break;
        case 0x42:
            Scr_AddContinueBlock(inst, block);
            if ( !block->abortLevel )
                block->abortLevel = 1;
            break;
        default:
            return;
    }
}

void __cdecl Scr_CalcLocalVarsIncStatement(sval_u expr, scr_block_s *block)
{
    Scr_CalcLocalVarsVariableExpressionRef(expr, block);
}

void __cdecl Scr_CalcLocalVarsVariableExpressionRef(sval_u expr, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 5 )
    {
        Scr_CalcLocalVarsSafeSetVariableField(expr.node[1], expr.node[2], block);
    }
    else if ( *(_BYTE *)expr.node == 15 )
    {
        Scr_CalcLocalVarsArrayVariableRef(expr.node[1], block);
    }
}

void __cdecl Scr_RegisterLocalVar(unsigned int name, sval_u sourcePos, scr_block_s *block)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !block->abortLevel )
    {
        for ( i = 0; i < block->localVarsCount; ++i )
        {
            if ( block->localVars[i].name == name )
                return;
        }
        Scr_CheckLocalVarsCount(block->localVarsCount);
        block->localVars[block->localVarsCount].name = name;
        block->localVars[block->localVarsCount++].sourcePos = sourcePos.stringValue;
    }
}

void __cdecl Scr_CheckLocalVarsCount(int localVarsCount)
{
    if ( localVarsCount >= 64 )
        Com_Error(ERR_DROP, "LOCAL_VAR_STACK_SIZE exceeded");
}

void __cdecl Scr_CalcLocalVarsArrayVariableRef(sval_u expr, scr_block_s *block)
{
    Scr_CalcLocalVarsArrayPrimitiveExpressionRef(expr, block);
}

void __cdecl Scr_CalcLocalVarsArrayPrimitiveExpressionRef(sval_u expr, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 19 )
        Scr_CalcLocalVarsVariableExpressionRef(expr.node[1], block);
}

void __cdecl Scr_CalcLocalVarsAssignmentStatement(sval_u lhs, sval_u rhs, scr_block_s *block)
{
    Scr_CalcLocalVarsVariableExpressionRef(lhs, block);
}

void __cdecl Scr_CalcLocalVarsIfStatement(scriptInstance_t inst, sval_u stmt, scr_block_s *block, sval_u *ifStatBlock)
{
    Scr_CopyBlock(block, (scr_block_s **)ifStatBlock);
    Scr_CalcLocalVarsStatement(inst, stmt, ifStatBlock->block);
    Scr_MergeChildBlocks((scr_block_s **)ifStatBlock, 1, block);
}

void __cdecl Scr_CopyBlock(scr_block_s *from, scr_block_s **to)
{
    if ( !*to )
        *to = (scr_block_s *)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s), "Scr_CopyBlock");
    memcpy(*to, from, sizeof(scr_block_s));
    (*to)->localVarsPublicCount = 0;
}

void __cdecl Scr_MergeChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
    unsigned int v3; // eax
    int j; // [esp+4h] [ebp-18h]
    unsigned int localVar; // [esp+8h] [ebp-14h]
    unsigned int localVar_4; // [esp+Ch] [ebp-10h]
    scr_block_s *childBlock; // [esp+10h] [ebp-Ch]
    int childIndex; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    if ( childCount && !block->abortLevel )
    {
        for ( childIndex = 0; childIndex < childCount; ++childIndex )
        {
            childBlock = childBlocks[childIndex];
            if ( childBlock->localVarsPublicCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            896,
                            0,
                            "%s",
                            "!childBlock->localVarsPublicCount") )
            {
                __debugbreak();
            }
            childBlock->localVarsPublicCount = block->localVarsCount;
            for ( i = 0; i < block->localVarsCount; ++i )
            {
                localVar = block->localVars[i].name;
                localVar_4 = block->localVars[i].sourcePos;
                j = Scr_FindLocalVar(childBlock, i, localVar);
                if ( j < 0 )
                {
                    j = childBlock->localVarsCount;
                    Scr_CheckLocalVarsCount(j);
                    ++childBlock->localVarsCount;
                }
                while ( j > i )
                {
                    v3 = *(unsigned int *)&childBlock->localVarsInitBits[8 * j + 4];
                    childBlock->localVars[j].name = *(unsigned int *)&childBlock->localVarsInitBits[8 * j];
                    childBlock->localVars[j--].sourcePos = v3;
                }
                childBlock->localVars[i].name = localVar;
                childBlock->localVars[i].sourcePos = localVar_4;
            }
        }
    }
}

int __cdecl Scr_FindLocalVar(scr_block_s *block, int startIndex, unsigned int name)
{
    while ( startIndex < block->localVarsCount )
    {
        if ( block->localVars[startIndex].name == name )
            return startIndex;
        ++startIndex;
    }
    return -1;
}

void __cdecl Scr_CalcLocalVarsIfElseStatement(
                scriptInstance_t inst,
                sval_u stmt1,
                sval_u stmt2,
                scr_block_s *block,
                sval_u *ifStatBlock,
                sval_u *elseStatBlock)
{
    scr_block_s *childBlocks[2]; // [esp+0h] [ebp-10h] BYREF
    int childCount; // [esp+8h] [ebp-8h]
    int abortLevel; // [esp+Ch] [ebp-4h]

    childCount = 0;
    abortLevel = 3;
    Scr_CopyBlock(block, (scr_block_s **)ifStatBlock);
    Scr_CalcLocalVarsStatement(inst, stmt1, ifStatBlock->block);
    if ( ifStatBlock->block->abortLevel <= 3 )
    {
        abortLevel = ifStatBlock->block->abortLevel;
        if ( !abortLevel )
            childBlocks[childCount++] = ifStatBlock->block;
    }
    Scr_CopyBlock(block, (scr_block_s **)elseStatBlock);
    Scr_CalcLocalVarsStatement(inst, stmt2, elseStatBlock->block);
    if ( elseStatBlock->block->abortLevel <= abortLevel )
    {
        abortLevel = elseStatBlock->block->abortLevel;
        if ( !abortLevel )
            childBlocks[childCount++] = elseStatBlock->block;
    }
    if ( !block->abortLevel )
        block->abortLevel = abortLevel;
    Scr_AppendChildBlocks(childBlocks, childCount, block);
    Scr_MergeChildBlocks(childBlocks, childCount, block);
}

void __cdecl Scr_AppendChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
    int localVarsCount; // ecx
    unsigned int localVar; // [esp+0h] [ebp-14h]
    unsigned int localVar_4; // [esp+4h] [ebp-10h]
    int childIndex; // [esp+Ch] [ebp-8h]
    int childIndexa; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( childCount && !block->abortLevel )
    {
        for ( childIndex = 0; childIndex < childCount; ++childIndex )
            childBlocks[childIndex]->abortLevel = 0;
        for ( i = 0; i < (*childBlocks)->localVarsCount; ++i )
        {
            localVar = (*childBlocks)->localVars[i].name;
            localVar_4 = (*childBlocks)->localVars[i].sourcePos;
            if ( Scr_FindLocalVar(block, 0, localVar) < 0 )
            {
                for ( childIndexa = 1; childIndexa < childCount; ++childIndexa )
                {
                    if ( Scr_FindLocalVar(childBlocks[childIndexa], 0, localVar) < 0 )
                        goto LABEL_8;
                }
                localVarsCount = block->localVarsCount;
                block->localVars[localVarsCount].name = localVar;
                block->localVars[localVarsCount].sourcePos = localVar_4;
                ++block->localVarsCount;
            }
LABEL_8:
            ;
        }
    }
}

void __cdecl Scr_AddBreakBlock(scriptInstance_t inst, scr_block_s *block)
{
    if (!block->abortLevel && gScrCompileGlob[inst].breakChildBlocks && gScrCompilePub[inst].developer_statement != 2)
    {
        Scr_CheckMaxSwitchCases(*gScrCompileGlob[inst].breakChildCount);
        gScrCompileGlob[inst].breakChildBlocks[(*gScrCompileGlob[inst].breakChildCount)++] = block;
    }
}

void __cdecl Scr_CheckMaxSwitchCases(int count)
{
    if ( count >= 512 )
        Com_Error(ERR_DROP, "MAX_SWITCH_CASES exceeded");
}

void __cdecl Scr_AddContinueBlock(scriptInstance_t inst, scr_block_s *block)
{
    if (!block->abortLevel && gScrCompileGlob[inst].continueChildBlocks && gScrCompilePub[inst].developer_statement != 2)
    {
        Scr_CheckMaxSwitchCases(*gScrCompileGlob[inst].continueChildCount);
        gScrCompileGlob[inst].continueChildBlocks[(*gScrCompileGlob[inst].continueChildCount)++] = block;
    }
}

void __cdecl Scr_CalcLocalVarsWhileStatement(
    scriptInstance_t inst,
    sval_u expr,
    sval_u stmt,
    scr_block_s *block,
    sval_u *whileStatBlock)
{
    int breakChildCount; // [esp+8h] [ebp-38h] BYREF
    int *oldBreakChildCount; // [esp+Ch] [ebp-34h]
    scr_block_s **breakChildBlocks; // [esp+10h] [ebp-30h]
    bool constConditional; // [esp+17h] [ebp-29h]
    int continueChildCount; // [esp+18h] [ebp-28h] BYREF
    int *oldContinueChildCount; // [esp+1Ch] [ebp-24h]
    VariableCompileValue constValue; // [esp+20h] [ebp-20h] BYREF
    int i; // [esp+2Ch] [ebp-14h]
    scr_block_s **continueChildBlocks; // [esp+30h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+34h] [ebp-Ch]
    int abortLevel; // [esp+38h] [ebp-8h]
    scr_block_s **oldContinueChildBlocks; // [esp+3Ch] [ebp-4h]

    constConditional = 0;
    if (EvalExpression(inst, expr, &constValue))
    {
        if (constValue.value.type == 6 || constValue.value.type == 5)
        {
            Scr_CastBool(inst, &constValue.value);
            if (constValue.value.u.intValue)
                constConditional = 1;
        }
        RemoveRefToValue(inst, constValue.value.type, constValue.value.u);
    }
    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    oldContinueChildBlocks = gScrCompileGlob[inst].continueChildBlocks;
    oldContinueChildCount = gScrCompileGlob[inst].continueChildCount;
    breakChildCount = 0;
    continueChildCount = 0;
    continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsWhileStatement");
    gScrCompileGlob[inst].continueChildBlocks = continueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = &continueChildCount;
    abortLevel = block->abortLevel;
    if (constConditional)
    {
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsWhileStatement");
        gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    }
    else
    {
        breakChildBlocks = 0;
    }
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    Scr_CopyBlock(block, (scr_block_s **)whileStatBlock);
    Scr_CalcLocalVarsStatement(inst, stmt, whileStatBlock->block);
    Scr_AddContinueBlock(inst, whileStatBlock->block);
    for (i = 0; i < continueChildCount; ++i)
        Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
    if (constConditional)
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
    Scr_MergeChildBlocks((scr_block_s **)whileStatBlock, 1, block);
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
    gScrCompileGlob[inst].continueChildBlocks = oldContinueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = oldContinueChildCount;
}

char __cdecl EvalExpression(scriptInstance_t inst, sval_u expr, VariableCompileValue *constValue)
{
    if ( *(_BYTE *)expr.node == 8 )
        return EvalPrimitiveExpression(inst, expr.node[1], constValue);
    if ( *(_BYTE *)expr.node == 51 )
        return EvalBinaryOperatorExpression(
                         inst,
                         expr.node[1],
                         expr.node[2],
                         expr.node[3],
                         expr.node[4],
                         constValue);
    return 0;
}

char __cdecl EvalPrimitiveExpression(scriptInstance_t inst, sval_u expr, VariableCompileValue *constValue)
{
    char result; // al

    switch ( *(_BYTE *)expr.node )
    {
        case ENUM_integer:
            EvalInteger(expr.node[1].stringValue, expr.node[2], constValue);
            result = 1;
            break;
        case ENUM_float:
            EvalFloat(expr.node[1].floatValue, expr.node[2], constValue);
            result = 1;
            break;
        case ENUM_minus_integer:
            //EvalInteger(-expr.node[1].stringValue, expr.node[2], constValue);
            EvalInteger(-expr.node[1].intValue, expr.node[2], constValue);
            result = 1;
            break;
        case 0xC:
            //EvalFloat(COERCE_FLOAT(expr.node[1].stringValue ^ _mask__NegFloat_), expr.node[2],constValue);
            EvalFloat(-expr.node[1].floatValue, expr.node[2],constValue);
            result = 1;
            break;
        case 0xD:
            EvalString(expr.node[1].stringValue, expr.node[2], constValue);
            result = 1;
            break;
        case 0xE:
            EvalIString(expr.node[1].stringValue, expr.node[2], constValue);
            result = 1;
            break;
        case 0x21:
            EvalUndefined(expr.node[1], constValue);
            result = 1;
            break;
        case 0x30:
            result = EvalPrimitiveExpressionList(
                                 inst,
                                 expr.node[1],
                                 expr.node[2],
                                 constValue);
            break;
        case 0x4A:
            EvalInteger(0, expr.node[1], constValue);
            result = 1;
            break;
        case 0x4B:
            EvalInteger(1, expr.node[1], constValue);
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

void __cdecl EvalUndefined(sval_u sourcePos, VariableCompileValue *constValue)
{
    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    520,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    constValue->value.type = 0;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalInteger(int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    536,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    constValue->value.type = 6;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalFloat(float value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    598,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    constValue->value.type = 5;
    constValue->value.u.floatValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    990,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    constValue->value.type = 2;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalIString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1009,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    constValue->value.type = 3;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

char __cdecl EvalPrimitiveExpressionList(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                VariableCompileValue *constValue)
{
    sval_u *node; // [esp+0h] [ebp-30h]
    VariableCompileValue constValue2[3]; // [esp+4h] [ebp-2Ch] BYREF
    int expr_count; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]

    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2333,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    expr_count = GetExpressionCount(exprlist);
    if ( expr_count == 1 )
        return EvalExpression(inst, exprlist.node[0].node[0].node[0], constValue);
    if ( expr_count != 3 )
        return 0;
    i = 0;
    for ( node = exprlist.node[0].node; node; node = node[1].node )
    {
        if ( !EvalExpression(inst, *node->node, &constValue2[i]) )
            return 0;
        ++i;
    }
    Scr_CreateVector(inst, constValue2, &constValue->value);
    constValue->sourcePos = sourcePos;
    return 1;
}

void __cdecl Scr_CreateVector(scriptInstance_t inst, VariableCompileValue *constValue, VariableValue *value)
{
    int type; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    float vec[3]; // [esp+8h] [ebp-Ch] BYREF

    for ( i = 0; i < 3; ++i )
    {
        type = constValue[i].value.type;
        if ( type == 5 )
        {
            vec[2 - i] = constValue[i].value.u.floatValue;
        }
        else
        {
            if ( type != 6 )
            {
                CompileError(inst, constValue[i].sourcePos.stringValue, "type %s is not a float", var_typename[type]);
                return;
            }
            vec[2 - i] = (float)constValue[i].value.u.intValue;
        }
    }
    value->type = 4;
    value->u.vectorValue = Scr_AllocVector(inst, vec);   // nx-port: a pointer
}

char __cdecl EvalBinaryOperatorExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u opcode,
                sval_u sourcePos,
                VariableCompileValue *constValue)
{
    VariableCompileValue constValue2; // [esp+14h] [ebp-18h] BYREF
    VariableCompileValue constValue1; // [esp+20h] [ebp-Ch] BYREF

    if ( !EvalExpression(inst, expr1, &constValue1) )
        return 0;
    if ( !EvalExpression(inst, expr2, &constValue2) )
        return 0;
    AddRefToValue(inst, constValue1.value.type, constValue1.value.u);
    AddRefToValue(inst, constValue2.value.type, constValue2.value.u);
    Scr_EvalBinaryOperator(inst, opcode.intValue, &constValue1.value, &constValue2.value);
    if (gScrVarPub[inst].error_message)
    {
        CompileError(inst, sourcePos.stringValue, "%s", gScrVarPub[inst].error_message);
        return 0;
    }
    else
    {
        constValue->value.u = constValue1.value.u;  // nx-port: whole value
        constValue->value.type = constValue1.value.type;
        constValue->sourcePos = sourcePos;
        return 1;
    }
}

void __cdecl Scr_CalcLocalVarsForStatement(
    scriptInstance_t inst,
    sval_u stmt1,
    sval_u expr,
    sval_u stmt2,
    sval_u stmt,
    scr_block_s *block,
    sval_u *forStatBlock,
    sval_u *forStatPostBlock)
{
    int breakChildCount; // [esp+8h] [ebp-38h] BYREF
    int *oldBreakChildCount; // [esp+Ch] [ebp-34h]
    scr_block_s **breakChildBlocks; // [esp+10h] [ebp-30h]
    bool constConditional; // [esp+17h] [ebp-29h]
    int continueChildCount; // [esp+18h] [ebp-28h] BYREF
    int *oldContinueChildCount; // [esp+1Ch] [ebp-24h]
    VariableCompileValue constValue; // [esp+20h] [ebp-20h] BYREF
    int i; // [esp+2Ch] [ebp-14h]
    scr_block_s **continueChildBlocks; // [esp+30h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+34h] [ebp-Ch]
    int abortLevel; // [esp+38h] [ebp-8h]
    scr_block_s **oldContinueChildBlocks; // [esp+3Ch] [ebp-4h]

    Scr_CalcLocalVarsStatement(inst, stmt1, block);
    if (*(_BYTE *)expr.node == 67)
    {
        constConditional = 0;
        if (EvalExpression(inst, expr.node[1], &constValue))
        {
            if (constValue.value.type == 6 || constValue.value.type == 5)
            {
                Scr_CastBool(inst, &constValue.value);
                if (constValue.value.u.intValue)
                    constConditional = 1;
            }
            RemoveRefToValue(inst, constValue.value.type, constValue.value.u);
        }
    }
    else
    {
        constConditional = 1;
    }
    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    oldContinueChildBlocks = gScrCompileGlob[inst].continueChildBlocks;
    oldContinueChildCount = gScrCompileGlob[inst].continueChildCount;
    breakChildCount = 0;
    continueChildCount = 0;
    continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsForStatement");
    gScrCompileGlob[inst].continueChildBlocks = continueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = &continueChildCount;
    abortLevel = block->abortLevel;
    if (constConditional)
    {
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsForStatement");
        gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    }
    else
    {
        breakChildBlocks = 0;
    }
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    Scr_CopyBlock(block, (scr_block_s **)forStatBlock);
    Scr_CopyBlock(block, (scr_block_s **)forStatPostBlock);
    Scr_CalcLocalVarsStatement(inst, stmt, forStatBlock->block);
    Scr_AddContinueBlock(inst, forStatBlock->block);
    for (i = 0; i < continueChildCount; ++i)
        Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
    Scr_CalcLocalVarsStatement(inst, stmt2, forStatPostBlock->block);
    Scr_AppendChildBlocks((scr_block_s **)forStatPostBlock, 1, block);
    Scr_MergeChildBlocks((scr_block_s **)forStatPostBlock, 1, block);
    if (constConditional)
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
    Scr_MergeChildBlocks((scr_block_s **)forStatBlock, 1, block);
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
    gScrCompileGlob[inst].continueChildBlocks = oldContinueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = oldContinueChildCount;
}
void __cdecl Scr_CalcLocalVarsWaittillStatement(sval_u exprlist, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    node = exprlist.node[0].node[1].node;
    if ( !node
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp", 3721, 0, "%s", "node") )
    {
        __debugbreak();
    }
    Scr_CalcLocalVarsFormalParameterListInternal(node, block);
}

void __cdecl Scr_CalcLocalVarsFormalParameterListInternal(sval_u *node, scr_block_s *block)
{
    while ( 1 )
    {
        node = node[1].node;
        if ( !node )
            break;
        Scr_CalcLocalVarsSafeSetVariableField(*node->node, node->node[1], block);
    }
}

void __cdecl Scr_CalcLocalVarsSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    Scr_RegisterLocalVar(expr.stringValue, sourcePos, block);
}

void __cdecl Scr_CalcLocalVarsSwitchStatement(scriptInstance_t inst, sval_u stmtlist, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-28h]
    int breakChildCount; // [esp+4h] [ebp-24h] BYREF
    scr_block_s **breakChildBlocks; // [esp+8h] [ebp-20h]
    int *oldBreakChildCount; // [esp+Ch] [ebp-1Ch]
    bool hasDefault; // [esp+13h] [ebp-15h]
    scr_block_s *currentBlock; // [esp+14h] [ebp-14h] BYREF
    scr_block_s **childBlocks; // [esp+18h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+1Ch] [ebp-Ch]
    int childCount; // [esp+20h] [ebp-8h]
    int abortLevel; // [esp+24h] [ebp-4h]

    abortLevel = 3;
    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    breakChildCount = 0;
    breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsSwitchStatement");
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    childCount = 0;
    currentBlock = 0;
    hasDefault = 0;
    childBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "Scr_CalcLocalVarsSwitchStatement");
    for (node = stmtlist.node[0].node[1].node; node; node = node[1].node)
    {
        if (*(_BYTE *)node->node == 63 || *(_BYTE *)node->node == 64)
        {
            currentBlock = 0;
            Scr_CopyBlock(block, &currentBlock);
            if (*(_BYTE *)node->node == 63)
            {
                node->node[3].block = currentBlock;
            }
            else
            {
                node->node[2].block = currentBlock;
                hasDefault = 1;
            }
        }
        else if (currentBlock)
        {
            Scr_CalcLocalVarsStatement(inst, *node, currentBlock);
            if (currentBlock->abortLevel)
            {
                if (currentBlock->abortLevel == 2)
                {
                    currentBlock->abortLevel = 0;
                    abortLevel = 0;
                    Scr_CheckMaxSwitchCases(childCount);
                    childBlocks[childCount++] = currentBlock;
                }
                else if (currentBlock->abortLevel <= abortLevel)
                {
                    abortLevel = currentBlock->abortLevel;
                }
                currentBlock = 0;
            }
        }
    }
    if (hasDefault)
    {
        if (currentBlock)
        {
            Scr_AddBreakBlock(inst, currentBlock);
            Scr_CheckMaxSwitchCases(childCount);
            childBlocks[childCount++] = currentBlock;
        }
        if (!block->abortLevel)
            block->abortLevel = abortLevel;
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
        Scr_MergeChildBlocks(childBlocks, childCount, block);
    }
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
}

void __cdecl Scr_CalcLocalVarsDeveloperStatementList(
                scriptInstance_t inst,
                sval_u val,
                scr_block_s *block,
                sval_u *devStatBlock)
{
    Scr_CopyBlock(block, (scr_block_s **)devStatBlock);
    Scr_CalcLocalVarsStatementList(inst, val, devStatBlock->block);
    Scr_MergeChildBlocks((scr_block_s **)devStatBlock, 1, block);
}

void __cdecl Scr_CalcLocalVarsFormalParameterList(sval_u exprlist, scr_block_s *block)
{
    Scr_CalcLocalVarsFormalParameterListInternal(exprlist.node[0].node, block);
}

void __cdecl EmitNormalThread(scriptInstance_t inst, sval_u val, sval_u *stmttblock)
{
    unsigned int posId; // [esp+0h] [ebp-4h]

    InitThread(inst, 0);
    posId = FindVariable(inst, gScrCompileGlob[inst].filePosId, val.node[1].stringValue);
    SetThreadPosition(inst, posId);
    EmitThreadInternal(inst, val, val.node[4], val.node[5], stmttblock->block);
}

void __cdecl SetThreadPosition(scriptInstance_t inst, unsigned int posId)
{
    char *v2; // esi

    v2 = TempMallocAlignStrict(0);
    GetVariableValueAddress(inst, posId)->u.codePosValue = v2;   // nx-port: the thread's entry, pointer-sized
}

void __cdecl EmitThreadInternal(
                scriptInstance_t inst,
                sval_u val,
                sval_u sourcePos,
                sval_u endSourcePos,
                scr_block_s *block)
{
    AddThreadStartOpcodePos(inst, sourcePos.stringValue);
    gScrCompileGlob[inst].cumulOffset = 0;
    gScrCompileGlob[inst].maxOffset = 0;
    gScrCompileGlob[inst].maxCallOffset = 0;
    CompileTransferRefToString(inst, val.node[1].stringValue, 2u);
    EmitFormalParameterList(inst, val.node[2], sourcePos, block);
    EmitStatementList(inst, val.node[3], 1, endSourcePos.stringValue, block);
    EmitEnd(inst);
    AddOpcodePos(inst, endSourcePos.stringValue, 1);
    AddOpcodePos(inst, 0xFFFFFFFE, 0);
    if ( gScrCompileGlob[inst].cumulOffset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    4601,
                    0,
                    "%s",
                    "!gScrCompileGlob[inst].cumulOffset") )
    {
        __debugbreak();
    }
    if ( gScrCompileGlob[inst].maxOffset + 32 * gScrCompileGlob[inst].maxCallOffset >= 2048 )
        CompileError(inst, sourcePos.stringValue, "function exceeds operand stack size");
}

void __cdecl CompileTransferRefToString(scriptInstance_t inst, unsigned int stringValue, unsigned int user)
{
    if (!stringValue
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            159,
            0,
            "%s",
            "stringValue"))
    {
        __debugbreak();
    }
    if (gScrCompilePub[inst].developer_statement == 2)
    {
        Scr_CompileRemoveRefToString(inst, stringValue);
    }
    else if (gScrCompilePub[inst].developer_statement != 3)
    {
        if (gScrCompileGlob[inst].bConstRefCount)
            SL_AddRefToString(stringValue, inst);
        SL_TransferRefToUser(inst, stringValue, user);
    }
}

void __cdecl EmitEnd(scriptInstance_t inst)
{
    EmitOpcode(inst, 0, 0, 0);
    EmitPreAssignmentPos(inst);
}

void __cdecl EmitPreAssignmentPos(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer && gScrCompilePub[inst].developer_statement != 3)
        Scr_AddAssignmentPos(inst, (char *)gScrCompilePub[inst].opcodePos);
}

void __cdecl EmitOpcode(scriptInstance_t inst, unsigned int op, int offset, int callType)
{
    bool v4; // [esp+8h] [ebp-10h]
    int valueIndex; // [esp+Ch] [ebp-Ch]
    int value_count; // [esp+10h] [ebp-8h]
    unsigned int index; // [esp+14h] [ebp-4h]
    unsigned int indexa; // [esp+14h] [ebp-4h]

    if (gScrCompilePub[inst].developer_statement == 3)
    {
        gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(1);
        *gScrCompileGlob[inst].codePos = op;
    }
    else
    {
        if (gScrCompilePub[inst].value_count)
        {
            value_count = gScrCompilePub[inst].value_count;
            gScrCompilePub[inst].value_count = 0;
            for (valueIndex = 0; valueIndex < value_count; ++valueIndex)
                EmitValue(inst, &gScrCompileGlob[inst].value_start[valueIndex]);
        }
        v4 = !gScrCompileGlob[inst].cumulOffset || callType == 2 || callType == 3;
        gScrCompilePub[inst].allowedBreakpoint = v4;
        gScrCompileGlob[inst].cumulOffset += offset;
        if (gScrCompileGlob[inst].maxOffset < gScrCompileGlob[inst].cumulOffset)
            gScrCompileGlob[inst].maxOffset = gScrCompileGlob[inst].cumulOffset;
        if (callType && gScrCompileGlob[inst].maxCallOffset < gScrCompileGlob[inst].cumulOffset)
            gScrCompileGlob[inst].maxCallOffset = gScrCompileGlob[inst].cumulOffset;
        gScrVarPub[inst].checksum *= 31;
        gScrVarPub[inst].checksum += op;
        if (gScrCompilePub[inst].opcodePos)
        {
            gScrCompileGlob[inst].codePos = gScrCompilePub[inst].opcodePos;
            switch (op)
            {
            case ' ':
                if (*gScrCompilePub[inst].opcodePos == 30)
                {
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 31;
                    return;
                }
                index = *gScrCompilePub[inst].opcodePos - 24;
                if (index > 5)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 31;
                EmitByte(inst, index);
                return;
            case '#':
                if (*gScrCompilePub[inst].opcodePos == 55)
                {
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 34;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos != 54)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 33;
                EmitPreAssignmentPos(inst);
                return;
            case '*':
                if (*gScrCompilePub[inst].opcodePos == 38)
                {
                    *gScrCompilePub[inst].opcodePos = 41;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos == 13)
                {
                    *gScrCompilePub[inst].opcodePos = 39;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos != 14)
                    goto LABEL_81;
                *gScrCompilePub[inst].opcodePos = 40;
                EmitPreAssignmentPos(inst);
                return;
            case '.':
                if (*gScrCompilePub[inst].opcodePos == 38)
                {
                    *gScrCompilePub[inst].opcodePos = 45;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos == 13)
                {
                    *gScrCompilePub[inst].opcodePos = 43;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos != 14)
                    goto LABEL_81;
                *gScrCompilePub[inst].opcodePos = 44;
                EmitPreAssignmentPos(inst);
                return;
            case '1':
                if (*gScrCompilePub[inst].opcodePos != 22)
                    goto LABEL_81;
                *gScrCompilePub[inst].opcodePos = 48;
                return;
            case '9':
                switch (*gScrCompilePub[inst].opcodePos)
                {
                case '7':
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 61;
                    EmitPreAssignmentPos(inst);
                    return;
                case '6':
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 60;
                    EmitPreAssignmentPos(inst);
                    return;
                case '-':
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 59;
                    EmitPreAssignmentPos(inst);
                    return;
                case '+':
                    RemoveOpcodePos(inst);
                    *gScrCompilePub[inst].opcodePos = 56;
                    EmitPreAssignmentPos(inst);
                    return;
                }
                if (*gScrCompilePub[inst].opcodePos != 44)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 58;
                EmitPreAssignmentPos(inst);
                return;
            case 'P':
                if (*gScrCompilePub[inst].opcodePos != 78)
                    goto LABEL_81;
                *gScrCompilePub[inst].opcodePos = 79;
                return;
            case 'R':
                if (*gScrCompilePub[inst].opcodePos != 15)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 80;
                if (!gScrCompileGlob[inst].prevOpcodePos
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        400,
                        0,
                        "%s",
                        "gScrCompileGlob[inst].prevOpcodePos"))
                {
                    __debugbreak();
                }
                if (*gScrCompileGlob[inst].prevOpcodePos == 78)
                {
                    if ((char *)gScrCompilePub[inst].opcodePos != TempMallocAlignStrict(0) - 1
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            403,
                            0,
                            "%s",
                            "gScrCompilePub[inst].opcodePos == (byte *)TempMalloc( 0 ) - 1"))
                    {
                        __debugbreak();
                    }
                    TempMemorySetPos((char *)gScrCompilePub[inst].opcodePos);
                    --gScrCompilePub[inst].opcodePos;
                    gScrCompileGlob[inst].prevOpcodePos = 0;
                    gScrCompileGlob[inst].codePos = gScrCompilePub[inst].opcodePos;
                    *gScrCompilePub[inst].opcodePos = 79;
                }
                return;
            case 'V':
                if (*gScrCompilePub[inst].opcodePos != 15)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 84;
                return;
            case 'Y':
                if (*gScrCompilePub[inst].opcodePos == 30)
                {
                    *gScrCompilePub[inst].opcodePos = 90;
                    return;
                }
                indexa = *gScrCompilePub[inst].opcodePos - 24;
                if (indexa > 5)
                    goto LABEL_81;
                *gScrCompilePub[inst].opcodePos = 90;
                EmitByte(inst, indexa);
                break;
            case '^':
                if (*gScrCompilePub[inst].opcodePos != 92)
                    goto LABEL_81;
                RemoveOpcodePos(inst);
                *gScrCompilePub[inst].opcodePos = 95;
                return;
            default:
                goto LABEL_81;
            }
        }
        else
        {
        LABEL_81:
            gScrCompileGlob[inst].prevOpcodePos = gScrCompilePub[inst].opcodePos;
            gScrCompilePub[inst].opcodePos = (unsigned __int8 *)TempMallocAlignStrict(1);
            gScrCompileGlob[inst].codePos = gScrCompilePub[inst].opcodePos;
            *gScrCompilePub[inst].opcodePos = op;
        }
    }
}

void __cdecl EmitByte(scriptInstance_t inst, unsigned __int8 value)
{
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(1);
    *gScrCompileGlob[inst].codePos = value;
}

void __cdecl EmitValue(scriptInstance_t inst, VariableCompileValue *constValue)
{
    switch ( constValue->value.type )
    {
        case 0:
            EmitGetUndefined(inst, constValue->sourcePos);
            break;
        case 2:
            EmitGetString(inst, constValue->value.u.intValue, constValue->sourcePos);
            break;
        case 3:
            EmitGetIString(inst, constValue->value.u.intValue, constValue->sourcePos);
            break;
        case 4:
            EmitGetVector(inst, constValue->value.u.vectorValue, constValue->sourcePos);
            break;
        case 5:
            EmitGetFloat(inst, constValue->value.u.floatValue, constValue->sourcePos);
            break;
        case 6:
            EmitGetInteger(inst, constValue->value.u.intValue, constValue->sourcePos);
            break;
        default:
            return;
    }
}

void __cdecl EmitGetUndefined(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 2u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitGetInteger(scriptInstance_t inst, int value, sval_u sourcePos)
{
    if ( value < 0 )
    {
        if ( value > -256 )
        {
            EmitOpcode(inst, 5u, 1, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            EmitByte(inst, -(char)value);
            return;
        }
        if ( value > -65536 )
        {
            EmitOpcode(inst, 7u, 1, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            EmitShort(inst, -(__int16)value);
            return;
        }
    }
    else
    {
        if ( !value )
        {
            EmitOpcode(inst, 3u, 1, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            return;
        }
        if ( value < 256 )
        {
            EmitOpcode(inst, 4u, 1, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            EmitByte(inst, value);
            return;
        }
        if ( value < 0x10000 )
        {
            EmitOpcode(inst, 6u, 1, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            EmitShort(inst, value);
            return;
        }
    }
    EmitOpcode(inst, 8u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitCodeInt(inst, value);
}

// nx-port: a code position is pointer-sized in the byte stream (cscr_codepos.h).
void __cdecl EmitCodepos(scriptInstance_t inst, const char *pos)
{
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(SCR_CODEPOS_SIZE);
    Scr_WriteCodePosAt((char *)gScrCompileGlob[inst].codePos, pos);
}

// ...and the integer operands that used to travel through EmitCodepos stay 4
// bytes: integer constants, jump offsets, parameter counts, case names.
void __cdecl EmitCodeInt(scriptInstance_t inst, int value)
{
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(4);
    memcpy(gScrCompileGlob[inst].codePos, &value, sizeof(value));
}

void __cdecl EmitGetFloat(scriptInstance_t inst, float value, sval_u sourcePos)
{
    EmitOpcode(inst, 9u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitFloat(inst, value);
}

void __cdecl EmitFloat(scriptInstance_t inst, float value)
{
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(4);
    *(float *)gScrCompileGlob[inst].codePos = value;
}

void __cdecl EmitGetString(scriptInstance_t inst, unsigned int value, sval_u sourcePos)
{
    EmitOpcode(inst, 0xAu, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitShort(inst, value);
    CompileTransferRefToString(inst, value, 1u);
}

void __cdecl EmitShort(scriptInstance_t inst, __int16 value)
{
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(2);
    *(_WORD *)gScrCompileGlob[inst].codePos = value;
}

void __cdecl EmitGetIString(scriptInstance_t inst, unsigned int value, sval_u sourcePos)
{
    EmitOpcode(inst, 0xBu, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitShort(inst, value);
    CompileTransferRefToString(inst, value, 1u);
}

void __cdecl EmitGetVector(scriptInstance_t inst, const float *value, sval_u sourcePos)
{
    int i; // [esp+8h] [ebp-4h]

    EmitOpcode(inst, 0xCu, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    for ( i = 0; i < 3; ++i )
        EmitFloat(inst, value[i]);
    RemoveRefToVector(inst, value);
}

void __cdecl EmitStatementList(
                scriptInstance_t inst,
                sval_u val,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block)
{
    sval_u *node; // [esp+4h] [ebp-8h]
    sval_u *nextNode; // [esp+8h] [ebp-4h]

    for ( node = val.node[0].node[1].node; node; node = nextNode )
    {
        nextNode = node[1].node;
        if ( lastStatement && Scr_IsLastStatement(inst, nextNode) )
            EmitStatement(inst, *node, 1, endSourcePos, block);
        else
            EmitStatement(inst, *node, 0, endSourcePos, block);
    }
}

char __cdecl Scr_IsLastStatement(scriptInstance_t inst, sval_u *node)
{
    if ( !node )
        return 1;

    if (gScrVarPub[inst].developer_script)
        return 0;

    while ( node )
    {
        if ( *(_BYTE *)node->node != 47 )
            return 0;
        node = node[1].node;
    }
    return 1;
}

void __cdecl EmitStatement(
                scriptInstance_t inst,
                sval_u val,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block)
{
    if ( gScrCompilePub[inst].developer_statement == 3 )
    {
        switch ( *(_BYTE *)val.node )
        {
            case 2:
            case 0x1B:
            case 0x2B:
            case 0x2C:
            case 0x2D:
            case 0x2E:
            case 0x3C:
                goto LABEL_3;
            default:
                CompileError(inst, 0, "illegal statement in debugger");
                break;
        }
    }
    else
    {
LABEL_3:
        switch ( *(_BYTE *)val.node )
        {
            case 2:
                EmitAssignmentStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x1B:
                EmitCallExpressionStatement(inst, val.node[1], block);
                break;
            case 0x1D:
                EmitReturnStatement(inst, val.node[1], val.node[2], block);
                break;
            case 0x1E:
                EmitEndStatement(inst, val.node[1], block);
                break;
            case 0x1F:
                EmitWaitStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    block);
                break;
            case 0x27:
                EmitIfStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    lastStatement,
                    endSourcePos,
                    block,
                    &val.node[4]);
                break;
            case 0x28:
                EmitIfElseStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    val.node[5],
                    lastStatement,
                    endSourcePos,
                    block,
                    &val.node[6],
                    &val.node[7]);
                break;
            case 0x29:
                EmitWhileStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block,
                    &val.node[5]);
                break;
            case 0x2A:
                EmitForStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    val.node[5],
                    val.node[6],
                    block,
                    &val.node[7],
                    &val.node[8]);
                break;
            case 0x2B:
                EmitIncStatement(inst, val.node[1], val.node[2], block);
                break;
            case 0x2C:
                EmitDecStatement(inst, val.node[1], val.node[2], block);
                break;
            case 0x2D:
                EmitBinaryEqualsOperatorExpression(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x2E:
                EmitStatementList(inst, val.node[1], lastStatement, endSourcePos, block);
                break;
            case 0x2F:
                EmitDeveloperStatementList(
                    inst,
                    val.node[1],
                    val.node[2],
                    block,
                    &val.node[3]);
                break;
            case 0x39:
                EmitWaittillStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x3A:
                EmitWaittillmatchStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x3B:
                EmitWaittillFrameEnd(inst, val.node[1]);
                break;
            case 0x3C:
                EmitNotifyStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x3D:
                EmitEndOnStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    val.node[4],
                    block);
                break;
            case 0x3E:
                EmitSwitchStatement(
                    inst,
                    val.node[1],
                    val.node[2],
                    val.node[3],
                    lastStatement,
                    endSourcePos,
                    block);
                break;
            case 0x3F:
                CompileError(inst, val.node[2].stringValue, "illegal case statement");
                break;
            case 0x40:
                CompileError(inst, val.node[1].stringValue, "illegal default statement");
                break;
            case 0x41:
                EmitBreakStatement(inst, val.node[1], block);
                break;
            case 0x42:
                EmitContinueStatement(inst, val.node[1], block);
                break;
            case 0x4E:
                EmitBreakpointStatement(inst, val.node[1]);
                break;
            case 0x4F:
                EmitProfBeginStatement(inst, val.node[1], val.node[2]);
                break;
            case 0x50:
                EmitProfEndStatement(inst, val.node[1], val.node[2]);
                break;
            default:
                return;
        }
    }
}

void __cdecl EmitBinaryEqualsOperatorExpression(
    scriptInstance_t inst,
    sval_u lhs,
    sval_u rhs,
    sval_u opcode,
    sval_u sourcePos,
    scr_block_s *block)
{
    if (gScrCompileGlob[inst].bConstRefCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            2653,
            0,
            "%s",
            "!gScrCompileGlob[inst].bConstRefCount"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].bConstRefCount = 1;
    EmitVariableExpression(inst, lhs, block);
    if (!gScrCompileGlob[inst].bConstRefCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            2656,
            0,
            "%s",
            "gScrCompileGlob[inst].bConstRefCount"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].bConstRefCount = 0;
    EmitExpression(inst, rhs, block);
    EmitOpcode(inst, (char)opcode.type, -1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitVariableExpressionRef(inst, lhs, block);
    EmitSetVariableField(inst, sourcePos);
}

void __cdecl EmitSetVariableField(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x39u, -1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitAssignmentPos(inst);
}

void __cdecl EmitAssignmentPos(scriptInstance_t inst)
{
    char *v1; // eax

    if (gScrVarPub[inst].developer && gScrCompilePub[inst].developer_statement != 3)
    {
        v1 = TempMallocAlignStrict(0);
        Scr_AddAssignmentPos(inst, v1);
    }
}

void __cdecl EmitVariableExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    switch ( *(_BYTE *)expr.node )
    {
        case 5:
            EmitLocalVariable(inst, expr.node[1], expr.node[2], block);
            break;
        case 0xF:
            EmitArrayVariable(
                inst,
                expr.node[1],
                expr.node[2],
                expr.node[3],
                expr.node[4],
                block);
            break;
        case 0x11:
            EmitFieldVariable(
                inst,
                expr.node[1],
                expr.node[2],
                expr.node[3],
                block);
            break;
        case 0x37:
            if ( gScrCompilePub[inst].script_loading )
                CompileError(inst, expr.node[2].stringValue, "self field can only be used in the script debugger");
            else
                CompileError(
                    inst,
                    expr.node[2].stringValue,
                    "self field in assignment expression not currently supported");
            break;
        case 0x52:
            EmitObject(inst, expr.node[1], expr.node[2]);
            break;
        default:
            return;
    }
}

void __cdecl EmitLocalVariable(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index; // [esp+0h] [ebp-8h]
    unsigned int opcode; // [esp+4h] [ebp-4h]

    if ( gScrCompilePub[inst].developer_statement == 3 )
    {
        EmitOpcode(inst, 0x83u, 1, 0);
        EmitCanonicalString(inst, expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(inst, expr.stringValue, sourcePos, 0, block);
        if ( index > 5 )
        {
            opcode = 30;
            EmitOpcode(inst, 0x1Eu, 1, 0);
        }
        else
        {
            opcode = index + 24;
            EmitOpcode(inst, index + 24, 1, 0);
        }
        if ( opcode == 30 )
            EmitByte(inst, index);
        AddOpcodePos(inst, sourcePos.stringValue, 1);
    }
}

void __cdecl EmitCanonicalString(scriptInstance_t inst, unsigned int stringValue)
{
    if (!stringValue
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            118,
            0,
            "%s",
            "stringValue"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].codePos = (unsigned __int8 *)TempMallocAlignStrict(2);
    if (gScrCompilePub[inst].developer_statement == 2)
    {
        if (gScrVarPub[inst].developer_script
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                124,
                0,
                "%s",
                "!gScrVarPub[inst].developer_script"))
        {
            __debugbreak();
        }
        Scr_CompileRemoveRefToString(inst, stringValue);
    }
    else if (gScrCompilePub[inst].developer_statement == 3)
    {
        *(_WORD *)gScrCompileGlob[inst].codePos = Scr_CompileCanonicalString(inst, stringValue);
        if (!*(_WORD *)gScrCompileGlob[inst].codePos)
            CompileError(inst, 0, "unknown field");
    }
    else
    {
        if (gScrCompileGlob[inst].bConstRefCount)
            SL_AddRefToString(stringValue, inst);
        *(_WORD *)gScrCompileGlob[inst].codePos = SL_TransferToCanonicalString(inst, stringValue);
    }
}

int __cdecl Scr_FindLocalVarIndex(
    scriptInstance_t inst,
    unsigned int name,
    sval_u sourcePos,
    bool create,
    scr_block_s *block)
{
    char *v6; // eax
    int i; // [esp+4h] [ebp-4h]

    if (gScrCompilePub[inst].developer_statement == 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            649,
            0,
            "%s",
            "gScrCompilePub[inst].developer_statement != SCR_DEV_EVALUATE"))
    {
        __debugbreak();
    }
    if (block)
    {
        for (i = 0; ; ++i)
        {
            if (i >= block->localVarsCount)
                goto LABEL_20;
            if (i == block->localVarsCreateCount)
            {
                ++block->localVarsCreateCount;
                EmitOpcode(inst, 0x16u, 0, 0);
                EmitCanonicalStringConst(inst, block->localVars[i].name);
                AddOpcodePos(inst, block->localVars[i].sourcePos, 0);
            }
            if (block->localVars[i].name == name)
                break;
        }
        Scr_CompileRemoveRefToString(inst, name);
        if (((unsigned __int8)(1 << (i & 7)) & block->localVarsInitBits[i >> 3]) == 0)
        {
            if (!create || gScrCompileGlob[inst].forceNotCreate)
            {
            LABEL_20:
                if (!create || gScrCompileGlob[inst].forceNotCreate)
                {
                    v6 = SL_ConvertToString(name, inst);
                    CompileError(inst, sourcePos.stringValue, "uninitialised variable '%s'", v6);
                    return 0;
                }
                goto unreachable;
            }
            block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
        }
        if (block->localVarsCreateCount - 1 < i
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                679,
                0,
                "%s",
                "(block->localVarsCreateCount - 1) >= i"))
        {
            __debugbreak();
        }
        return block->localVarsCreateCount - 1 - i;
    }
unreachable:
    CompileError(inst, sourcePos.stringValue, "unreachable code");
    return 0;
}

void __cdecl EmitCanonicalStringConst(scriptInstance_t inst, unsigned int stringValue)
{
    bool bConstRefCount; // [esp+3h] [ebp-1h]

    bConstRefCount = gScrCompileGlob[inst].bConstRefCount;
    gScrCompileGlob[inst].bConstRefCount = 1;
    EmitCanonicalString(inst, stringValue);
    gScrCompileGlob[inst].bConstRefCount = bConstRefCount;
}

void __cdecl EmitFieldVariable(scriptInstance_t inst, sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(inst, expr, sourcePos, block);
    EmitOpcode(inst, 0x2Au, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitCanonicalString(inst, field.stringValue);
}

void __cdecl EmitObject(scriptInstance_t inst, sval_u expr, sval_u sourcePos)
{
    signed int ObjectType; // [esp+0h] [ebp-18h]
    int classnum; // [esp+4h] [ebp-14h]  nx-port: an int, typed as a pointer by the decompiler
    char *s; // [esp+Ch] [ebp-Ch]
    int entnum; // [esp+10h] [ebp-8h]
    unsigned int idValue; // [esp+14h] [ebp-4h]

    if ( gScrCompilePub[inst].script_loading )
    {
        CompileError(inst, sourcePos.stringValue, "$ can only be used in the script debugger");
        return;
    }
    s = SL_ConvertToString(expr.stringValue, inst);
    if ( *s == 116 )
    {
        idValue = atoi(s + 1);
        if ( idValue )
        {
            if ( idValue < 0x7FFE && !IsObjectFree(inst, (unsigned __int16)idValue) )
            {
                ObjectType = GetObjectType(inst, (unsigned __int16)idValue);
                if ( ObjectType >= 13 && (ObjectType <= 16 || ObjectType == 21) )
                {
                    EmitOpcode(inst, 0x82u, 1, 0);
                    EmitShort(inst, idValue);
                    return;
                }
            }
        }
LABEL_17:
        CompileError(inst, sourcePos.stringValue, "bad expression");
        return;
    }
    if ( *s == 97 )
    {
        CompileError(inst, sourcePos.stringValue, "argument expressions not supported in statements");
        return;
    }
    classnum = Scr_GetClassnumForCharId(inst, *s);
    if ( classnum < 0 )
        goto LABEL_17;
    entnum = atoi(s + 1);
    if ( !entnum && s[1] != 48 )
        goto LABEL_17;
    EmitOpcode(inst, 0x81u, 1, 0);
    EmitCodeInt(inst, classnum);
    EmitCodeInt(inst, entnum);
}

void __cdecl EmitArrayVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, index, block);
    EmitPrimitiveExpression(inst, expr, block);
    EmitEvalArray(inst, sourcePos, indexSourcePos);
}

void __cdecl EmitEvalArray(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(inst, 0x20u, -1, 0);
    AddOpcodePos(inst, indexSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitPrimitiveExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    VariableCompileValue constValue; // [esp+0h] [ebp-Ch] BYREF

    if ( EmitOrEvalPrimitiveExpression(inst, expr, &constValue, block) )
        EmitValue(inst, &constValue);
}

char __cdecl EmitOrEvalPrimitiveExpression(
                scriptInstance_t inst,
                sval_u expr,
                VariableCompileValue *constValue,
                scr_block_s *block)
{
    char result; // al

    switch ( *(_BYTE *)expr.node )
    {
        case 0x13:
            EmitVariableExpression(inst, expr.node[1], block);
            result = 0;
            break;
        case 0x14:
            EmitGetFunction(inst, expr.node[1], expr.node[2]);
            result = 0;
            break;
        case 0x15:
            EmitCallExpression(inst, expr.node[1], 0, block);
            result = 0;
            break;
        case 0x22:
            EmitSelf(inst, expr.node[1]);
            result = 0;
            break;
        case 0x24:
            EmitLevel(inst, expr.node[1]);
            result = 0;
            break;
        case 0x25:
            EmitGame(inst, expr.node[1]);
            result = 0;
            break;
        case 0x26:
            EmitAnim(inst, expr.node[1]);
            result = 0;
            break;
        case 0x30:
            result = EmitOrEvalPrimitiveExpressionList(
                                 inst,
                                 expr.node[1],
                                 expr.node[2],
                                 constValue,
                                 block);
            break;
        case 0x36:
            EmitSize(inst, expr.node[1], expr.node[2], block);
            result = 0;
            break;
        case 0x44:
            EmitEmptyArray(inst, expr.node[1]);
            result = 0;
            break;
        case 0x45:
            EmitAnimation(inst, expr.node[1], expr.node[2]);
            result = 0;
            break;
        case 0x4C:
            EmitAnimTree(inst, expr.node[1]);
            result = 0;
            break;
        case 0x4D:
            EmitBreakOn(
                inst,
                expr.node[1],
                expr.node[2],
                expr.node[3]);
            result = 0;
            break;
        default:
            result = EvalPrimitiveExpression(inst, expr, constValue);
            break;
    }
    return result;
}

void __cdecl EmitAnimTree(scriptInstance_t inst, sval_u sourcePos)
{
    if (gScrAnimPub[inst].animTreeIndex)
        EmitGetInteger(inst, gScrAnimPub[inst].animTreeIndex, sourcePos);
    else
        CompileError(inst, sourcePos.stringValue, "#using_animtree was not specified");
}

void __cdecl EmitSize(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpression(inst, expr, block);
    EmitOpcode(inst, 0x76u, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitSelf(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0xFu, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitLevel(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x10u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitGame(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x11u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitAnim(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x12u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitEmptyArray(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x25u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitAnimation(scriptInstance_t inst, sval_u anim, sval_u sourcePos)
{
    EmitOpcode(inst, 0x13u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitCodepos(inst, (const char *)0xFFFFFFFF);
    if ( gScrCompilePub[inst].developer_statement != 2 )
        Scr_EmitAnimation(inst, (char *)gScrCompileGlob[inst].codePos, anim.stringValue, sourcePos.stringValue);
    Scr_CompileRemoveRefToString(inst, anim.stringValue);
}

void __cdecl EmitGetFunction(scriptInstance_t inst, sval_u func, sval_u sourcePos)
{
    EmitOpcode(inst, 0x15u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 3);
    EmitFunction(inst, func, sourcePos);
}

void __cdecl EmitFunction(scriptInstance_t inst, sval_u func, sval_u sourcePos)
{
    char *v3; // eax
    unsigned int v4; // eax
    unsigned int Variable; // eax
    VariableValue pos; // [esp+58h] [ebp-50h]
    unsigned int threadCountId; // [esp+68h] [ebp-40h]
    unsigned int valueId; // [esp+6Ch] [ebp-3Ch]
    unsigned int fileCountId; // [esp+78h] [ebp-30h] BYREF
    unsigned int filename; // [esp+7Ch] [ebp-2Ch]
    unsigned int posId; // [esp+80h] [ebp-28h]
    unsigned int threadName; // [esp+84h] [ebp-24h]
    bool bExists; // [esp+8Bh] [ebp-1Dh]
    int scope; // [esp+8Ch] [ebp-1Ch]
    unsigned int countId; // [esp+90h] [ebp-18h]
    unsigned int filePosId; // [esp+94h] [ebp-14h] BYREF
    VariableValue count; // [esp+98h] [ebp-10h] BYREF
    VariableValue value; // [esp+A0h] [ebp-8h] BYREF

    if (gScrCompilePub[inst].developer_statement == 3)
    {
        CompileError(inst, sourcePos.stringValue, "cannot evaluate in the debugger");
        return;
    }
    if (gScrCompilePub[inst].developer_statement == 2)
    {
        Scr_CompileRemoveRefToString(inst, func.node[1].stringValue);
        if (*(_BYTE *)func.node == ENUM_far_function)
        {
            Scr_CompileRemoveRefToString(inst, func.node[2].stringValue);
            --gScrCompilePub[inst].far_function_count;
        }
        return;
    }

    if (*(_BYTE *)func.node == ENUM_local_function)
    {
        scope = 0;
        fileCountId = gScrCompileGlob[inst].fileCountId;
        threadName = func.node[1].stringValue;
        CompileTransferRefToString(inst, threadName, 2u);
    EMIT:
        EmitCodepos(inst, (const char *)(uintptr_t)scope);   // nx-port: the call-target slot LinkThread patches
        Variable = GetVariable(inst, fileCountId, threadName);
        threadCountId = GetObject(inst, Variable);
        if (!threadCountId
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1612,
                0,
                "%s",
                "threadCountId"))
        {
            __debugbreak();
        }

        countId = GetVariable(inst, threadCountId, 0);
        count = Scr_EvalVariable(inst, countId);

        if (count.type
            && count.type != 6
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1616,
                0,
                "%s",
                "(count.type == VAR_UNDEFINED) || (count.type == VAR_INTEGER)"))
        {
            __debugbreak();
        }

        if (!count.type)
        {
            count.type = VAR_INTEGER;
            count.u.intValue = 0;
        }

        valueId = GetNewVariable(inst, threadCountId, count.u.intValue + 1);
        value.u.codePosValue = (const char *)gScrCompileGlob[inst].codePos;   // nx-port: the slot, pointer-sized
        if (gScrCompilePub[inst].developer_statement)
        {
            if (!gScrVarPub[inst].developer_script
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1629,
                    0,
                    "%s",
                    "gScrVarPub[inst].developer_script"))
            {
                __debugbreak();
            }
            value.type = VAR_DEVELOPER_CODEPOS;
        }
        else
        {
            value.type = VAR_CODEPOS;
        }
        SetNewVariableValue(inst, valueId, &value);
        ++count.u.intValue;
        SetVariableValue(inst, countId, &count);
        AddOpcodePos(inst, sourcePos.stringValue, 0);
        return;
    }
    if (*(_BYTE *)func.node != 23
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            1562,
            0,
            "%s",
            "func.node[0].type == ENUM_far_function"))
    {
        __debugbreak();
    }
    scope = FUNC_SCOPE_FAR;
    v3 = SL_ConvertToString(func.node[1].stringValue, inst);
    filename = Scr_CreateCanonicalFilename(inst, v3);
    Scr_CompileRemoveRefToString(inst, func.node[1].stringValue);
    v4 = FindVariable(inst, gScrCompilePub[inst].loadedscripts, filename);
    value = Scr_EvalVariable(inst, v4);
    bExists = value.type != 0;
    AddFilePrecache(inst, filename, sourcePos.stringValue, 0, &filePosId, &fileCountId);
    threadName = func.node[2].stringValue;
    CompileTransferRefToString(inst, threadName, 2u);
    if (!bExists)
        goto EMIT;
    posId = FindVariable(inst, filePosId, threadName);
    if (posId)
    {
        pos = Scr_EvalVariable(inst, posId);
        if (pos.type != 7
            && pos.type != 12
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1585,
                0,
                "%s\n\t(pos.type) = %i",
                "(pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS)",
                pos.type))
        {
            __debugbreak();
        }
        if (!pos.u.intValue
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1586,
                0,
                "%s",
                "pos.u.codePosValue"))
        {
            __debugbreak();
        }
        if (pos.type == VAR_CODEPOS)
            goto LABEL_23;
        if (pos.type != 12
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1595,
                0,
                "%s",
                "pos.type == VAR_DEVELOPER_CODEPOS"))
        {
            __debugbreak();
        }
        if (!gScrVarPub[inst].developer_script
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1596,
                0,
                "%s",
                "gScrVarPub[inst].developer_script"))
        {
            __debugbreak();
        }
        if (gScrCompilePub[inst].developer_statement == 2
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                1597,
                0,
                "%s",
                "gScrCompilePub[inst].developer_statement != SCR_DEV_IGNORE"))
        {
            __debugbreak();
        }
        if (gScrCompilePub[inst].developer_statement)
            LABEL_23:
        EmitCodepos(inst, pos.u.codePosValue);
        else
            CompileError(inst, sourcePos.stringValue, "normal script cannot reference a function in a /# ... #/ comment");
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "unknown function");
    }
}

void __cdecl AddFilePrecache(
                scriptInstance_t inst,
                unsigned int filename,
                unsigned int sourcePos,
                bool include,
                unsigned int *filePosId,
                unsigned int *fileCountId)
{
    unsigned int Variable; // eax
    unsigned int v7; // eax

    if ( !gScrCompileGlob[inst].precachescriptList
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1492,
                    0,
                    "%s",
                    "gScrCompileGlob[inst].precachescriptList") )
    {
        __debugbreak();
    }
    SL_AddRefToString(filename, inst);
    Scr_CompileRemoveRefToString(inst, filename);

    gScrCompileGlob[inst].precachescriptList->filename = filename;
    gScrCompileGlob[inst].precachescriptList->sourcePos = sourcePos;
    gScrCompileGlob[inst].precachescriptList->include = include;
    ++gScrCompileGlob[inst].precachescriptList;
    if ( !include )
    {
        Variable = GetVariable(inst, gScrCompilePub[inst].scriptsPos, filename);
        *filePosId = GetObject(inst, Variable);
        v7 = GetVariable(inst, gScrCompilePub[inst].scriptsCount, filename);
        *fileCountId = GetObject(inst, v7);
    }
}

void __cdecl EmitCallExpression(scriptInstance_t inst, sval_u expr, bool bStatement, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 25 )
    {
        EmitCall(inst, expr.node[1], expr.node[2], bStatement, block);
    }
    else if ( *(_BYTE *)expr.node == 26 )
    {
        EmitMethod(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            bStatement,
            block);
    }
}

void __cdecl EmitCall(scriptInstance_t inst, sval_u func_name, sval_u params, bool bStatement, scr_block_s *block)
{
    __int16 v5; // ax
    void (__cdecl *func)(); // [esp+28h] [ebp-28h]
    char *savedPos; // [esp+2Ch] [ebp-24h] BYREF
    unsigned int funcId; // [esp+30h] [ebp-20h]
    int param_count; // [esp+34h] [ebp-1Ch]
    unsigned int name; // [esp+38h] [ebp-18h]
    const char *pName; // [esp+3Ch] [ebp-14h] BYREF
    int type; // [esp+40h] [ebp-10h] BYREF
    sval_u sourcePos; // [esp+44h] [ebp-Ch]
    VariableValue value; // [esp+48h] [ebp-8h] BYREF

    savedPos = 0;
    name = Scr_GetBuiltin(inst, func_name);
    if ( !name )
        goto script_function;
    pName = SL_ConvertToString(name, inst);
    sourcePos = func_name.node[2];
    if ( gScrCompilePub[inst].developer_statement == 3 )
    {
        type = 0;
        func = GetFunction(inst, &pName, &type);
    }
    else
    {
        funcId = FindVariable(inst, gScrCompilePub[inst].builtinFunc, name);
        if ( funcId )
        {
            value = Scr_EvalVariable(inst, funcId);
            type = Scr_GetUncacheType(value.type);
            func = (void (__cdecl *)())value.u.codePosValue;   // nx-port: a function pointer, whole
        }
        else
        {
            type = 0;
            func = GetFunction(inst, &pName, &type);
            funcId = GetNewVariable(inst, gScrCompilePub[inst].builtinFunc, name);
            value.type = Scr_GetCacheType(type);
            value.u.codePosValue = (const char *)func;
            SetVariableValue(inst, funcId, &value);
        }
    }
    if ( func )
    {
        if ( type == 1 && (Scr_BeginDevScript(inst, &type, &savedPos), type == 1) && !bStatement )
        {
            CompileError(
                inst,
                sourcePos.stringValue,
                "return value of developer command can not be accessed if not in a /# ... #/ comment");
        }
        else
        {
            param_count = EmitExpressionList(inst, params, block);
            if ( param_count < 256 )
            {
                Scr_CompileRemoveRefToString(inst, name);
                EmitCallBuiltinOpcode(inst, param_count, sourcePos);
                v5 = AddFunction(inst, (uintptr_t)func);
                EmitShort(inst, v5);
                AddExpressionListOpcodePos(inst, params);
                if ( bStatement )
                    EmitDecTop(inst);
                if ( type == 1 )
                    Scr_EndDevScript(inst, 1, &savedPos);
            }
            else
            {
                CompileError(inst, sourcePos.stringValue, "parameter count exceeds 256");
            }
        }
    }
    else
    {
script_function:
        if ( gScrCompilePub[inst].developer_statement == 3 )
        {
            CompileError(inst, func_name.node[2].stringValue, "unknown builtin function");
        }
        else
        {
            EmitPreFunctionCall(inst, func_name);
            param_count = EmitExpressionList(inst, params, block);
            EmitPostFunctionCall(inst, func_name, param_count, 0, block);
            AddExpressionListOpcodePos(inst, params);
            if ( bStatement )
                EmitDecTop(inst);
        }
    }
}

void __cdecl EmitDecTop(scriptInstance_t inst)
{
    EmitOpcode(inst, 0x58u, -1, 0);
}

int __cdecl EmitExpressionList(scriptInstance_t inst, sval_u exprlist, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    int expr_count; // [esp+4h] [ebp-4h]

    expr_count = 0;
    for ( node = exprlist.node[0].node; node; node = node[1].node )
    {
        EmitExpression(inst, *node->node, block);
        ++expr_count;
    }
    return expr_count;
}

void __cdecl AddExpressionListOpcodePos(scriptInstance_t inst, sval_u exprlist)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    if (gScrVarPub[inst].developer)
    {
        for (node = exprlist.node[0].node; node; node = node[1].node)
            AddOpcodePos(inst, node->node[1].stringValue, 0);
    }
}
int __cdecl AddFunction(scriptInstance_t inst, uintptr_t func)   // nx-port: was int
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < gScrCompilePub[inst].func_table_size; ++i )
    {
        if ( gScrCompilePub[inst].func_table[i] == func )
            return i;
    }
    if ( i != gScrCompilePub[inst].func_table_size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1664,
                    0,
                    "%s",
                    "i == gScrCompilePub[inst].func_table_size") )
    {
        __debugbreak();
    }
    if ( gScrCompilePub[inst].func_table_size == 1024 )
        Com_Error(ERR_DROP, "SCR_FUNC_TABLE_SIZE exceeded");
    gScrCompilePub[inst].func_table[gScrCompilePub[inst].func_table_size] = func;
    gScrVmDebugPub[inst].func_table[gScrCompilePub[inst].func_table_size++].breakpointCount = 0;
    return i;
}

void __cdecl EmitPreFunctionCall(scriptInstance_t inst, sval_u func_name)
{
    if ( *(_BYTE *)func_name.node == 28 )
        EmitOpcode(inst, 0x4Eu, 1, 0);
}

void __cdecl EmitPostFunctionCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                scr_block_s *block)
{
    if ( *(_BYTE *)func_name.node == 28 )
    {
        EmitPostScriptFunctionCall(
            inst,
            func_name.node[1],
            param_count,
            bMethod,
            func_name.node[2],
            block);
    }
    else if ( *(_BYTE *)func_name.node == 32 )
    {
        EmitPostScriptThreadCall(
            inst,
            func_name.node[1],
            param_count,
            bMethod,
            func_name.node[2],
            func_name.node[3],
            block);
    }
}

void __cdecl EmitPostScriptFunctionCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos,
                scr_block_s *block)
{
    if ( *(_BYTE *)func_name.node == 20 )
    {
        EmitPostScriptFunction(inst, func_name.node[1], param_count, bMethod, nameSourcePos);
    }
    else if ( *(_BYTE *)func_name.node == 24 )
    {
        EmitPostScriptFunctionPointer(
            inst,
            func_name.node[1],
            param_count,
            bMethod,
            nameSourcePos,
            func_name.node[2],
            block);
    }
}

void __cdecl EmitPostScriptFunction(
                scriptInstance_t inst,
                sval_u func,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos)
{
    if ( bMethod )
        EmitOpcode(inst, 0x52u, -param_count - 1, 3);
    else
        EmitOpcode(inst, 0x50u, -param_count, 3);
    AddOpcodePos(inst, nameSourcePos.stringValue, 3);
    EmitFunction(inst, func, nameSourcePos);
}

void __cdecl EmitPostScriptFunctionPointer(
                scriptInstance_t inst,
                sval_u expr,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos,
                sval_u sourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, expr, block);
    if ( bMethod )
        EmitOpcode(inst, 0x53u, -param_count - 2, 3);
    else
        EmitOpcode(inst, 0x51u, -param_count - 1, 3);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    AddOpcodePos(inst, nameSourcePos.stringValue, 1);
}

void __cdecl EmitPostScriptThreadCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                sval_u sourcePos,
                sval_u nameSourcePos,
                scr_block_s *block)
{
    if ( *(_BYTE *)func_name.node == 20 )
    {
        EmitPostScriptThread(inst, func_name.node[1], param_count, bMethod, nameSourcePos);
    }
    else if ( *(_BYTE *)func_name.node == 24 )
    {
        EmitPostScriptThreadPointer(
            inst,
            func_name.node[1],
            param_count,
            bMethod,
            func_name.node[2],
            block);
    }
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitPostScriptThread(scriptInstance_t inst, sval_u func, int param_count, bool bMethod, sval_u sourcePos)
{
    if ( bMethod )
        EmitOpcode(inst, 0x56u, -param_count, 2);
    else
        EmitOpcode(inst, 0x54u, 1 - param_count, 2);
    AddOpcodePos(inst, sourcePos.stringValue, 3);
    EmitFunction(inst, func, sourcePos);
    EmitCodeInt(inst, param_count);
}

void __cdecl EmitPostScriptThreadPointer(
                scriptInstance_t inst,
                sval_u expr,
                int param_count,
                bool bMethod,
                sval_u sourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, expr, block);
    if ( bMethod )
        EmitOpcode(inst, 0x57u, -param_count - 1, 2);
    else
        EmitOpcode(inst, 0x55u, -param_count, 2);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    EmitCodeInt(inst, param_count);
}

void __cdecl Scr_BeginDevScript(scriptInstance_t inst, int *type, char **savedPos)
{
    if ( gScrCompilePub[inst].developer_statement )
    {
        *type = 0;
    }
    else
    {
        if (gScrVarPub[inst].developer_script)
        {
            gScrCompilePub[inst].developer_statement = 1;
        }
        else
        {
            *savedPos = TempMallocAlignStrict(0);
            gScrCompilePub[inst].developer_statement = 2;
        }
        *type = 1;
    }
}

void __cdecl Scr_EndDevScript(scriptInstance_t inst, int type, char **savedPos)
{
    if ( type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1833,
                    0,
                    "%s",
                    "type == BUILTIN_DEVELOPER_ONLY") )
    {
        __debugbreak();
    }

    gScrCompilePub[inst].developer_statement = 0;

    if (!gScrVarPub[inst].developer_script)
        TempMemorySetPos(*savedPos);
}

int __cdecl Scr_GetCacheType(int type)
{
    if ( !type )
        return 7;
    if ( type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1850,
                    0,
                    "%s",
                    "type == BUILTIN_DEVELOPER_ONLY") )
    {
        __debugbreak();
    }
    return 12;
}

int __cdecl Scr_GetUncacheType(int type)
{
    if ( type == 7 )
        return 0;
    if ( type != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1864,
                    0,
                    "%s",
                    "type == VAR_DEVELOPER_CODEPOS") )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl EmitCallBuiltinOpcode(scriptInstance_t inst, int param_count, sval_u sourcePos)
{
    unsigned int opcode; // [esp+0h] [ebp-4h]

    if ( param_count > 5 )
    {
        opcode = 68;
        EmitOpcode(inst, 0x44u, 1 - param_count, 1);
    }
    else
    {
        opcode = param_count + 62;
        EmitOpcode(inst, param_count + 62, 1 - param_count, 1);
    }
    AddOpcodePos(inst, sourcePos.stringValue, 9);
    if ( opcode == 68 )
        EmitByte(inst, param_count);
}

void __cdecl EmitMethod(
                scriptInstance_t inst,
                sval_u expr,
                sval_u func_name,
                sval_u params,
                sval_u methodSourcePos,
                bool bStatement,
                scr_block_s *block)
{
    __int16 v7; // ax
    char *savedPos; // [esp+28h] [ebp-28h] BYREF
    unsigned int methId; // [esp+2Ch] [ebp-24h]
    void (__cdecl *meth)(scr_entref_t); // [esp+30h] [ebp-20h]
    int param_count; // [esp+34h] [ebp-1Ch]
    unsigned int name; // [esp+38h] [ebp-18h]
    const char *pName; // [esp+3Ch] [ebp-14h] BYREF
    int type; // [esp+40h] [ebp-10h] BYREF
    sval_u sourcePos; // [esp+44h] [ebp-Ch]
    VariableValue value; // [esp+48h] [ebp-8h] BYREF

    savedPos = 0;
    name = Scr_GetBuiltin(inst, func_name);
    if ( !name )
        goto script_method;
    pName = SL_ConvertToString(name, inst);
    sourcePos = func_name.node[2];
    if ( gScrCompilePub[inst].developer_statement == 3 )
    {
        type = 0;
        meth = GetMethod(inst, &pName, &type);
    }
    else
    {
        methId = FindVariable(inst, gScrCompilePub[inst].builtinMeth, name);
        if ( methId )
        {
            value = Scr_EvalVariable(inst, methId);
            type = Scr_GetUncacheType(value.type);
            meth = (void (__cdecl *)(scr_entref_t))value.u.codePosValue;   // nx-port: a function pointer, whole
        }
        else
        {
            type = 0;
            meth = GetMethod(inst, &pName, &type);
            methId = GetNewVariable(inst, gScrCompilePub[inst].builtinMeth, name);
            value.type = Scr_GetCacheType(type);
            value.u.codePosValue = (const char *)meth;
            SetVariableValue(inst, methId, &value);
        }
    }
    if ( meth )
    {
        if ( type == 1 && (Scr_BeginDevScript(inst, &type, &savedPos), type == 1) && !bStatement )
        {
            CompileError(
                inst,
                sourcePos.stringValue,
                "return value of developer command can not be accessed if not in a /# ... #/ comment");
        }
        else
        {
            param_count = EmitExpressionList(inst, params, block);
            EmitPrimitiveExpression(inst, expr, block);
            if ( param_count < 256 )
            {
                Scr_CompileRemoveRefToString(inst, name);
                EmitCallBuiltinMethodOpcode(inst, param_count, sourcePos);
                v7 = AddFunction(inst, (uintptr_t)meth);
                EmitShort(inst, v7);
                AddOpcodePos(inst, methodSourcePos.stringValue, 0);
                AddExpressionListOpcodePos(inst, params);
                if ( bStatement )
                    EmitDecTop(inst);
                EmitAssignmentPos(inst);
                if ( type == 1 )
                    Scr_EndDevScript(inst, 1, &savedPos);
            }
            else
            {
                CompileError(inst, sourcePos.stringValue, "parameter count exceeds 256");
            }
        }
    }
    else
    {
script_method:
        if ( gScrCompilePub[inst].developer_statement == 3 )
        {
            CompileError(inst, func_name.node[2].stringValue, "unknown builtin method");
        }
        else
        {
            EmitPreFunctionCall(inst, func_name);
            param_count = EmitExpressionList(inst, params, block);
            EmitPrimitiveExpression(inst, expr, block);
            EmitPostFunctionCall(inst, func_name, param_count, 1, block);
            AddOpcodePos(inst, methodSourcePos.stringValue, 0);
            AddExpressionListOpcodePos(inst, params);
            if ( bStatement )
                EmitDecTop(inst);
        }
    }
}

void __cdecl EmitCallBuiltinMethodOpcode(scriptInstance_t inst, int param_count, sval_u sourcePos)
{
    unsigned int opcode; // [esp+0h] [ebp-4h]

    if ( param_count > 5 )
    {
        opcode = 75;
        EmitOpcode(inst, 0x4Bu, -param_count, 1);
    }
    else
    {
        opcode = param_count + 69;
        EmitOpcode(inst, param_count + 69, -param_count, 1);
    }
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 9);
    if ( opcode == 75 )
        EmitByte(inst, param_count);
}

void __cdecl EmitBreakOn(scriptInstance_t inst, sval_u expr, sval_u param, sval_u sourcePos)
{
    CompileError(inst, sourcePos.stringValue, "illegal function name");
}

bool __cdecl EmitOrEvalPrimitiveExpressionList(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                VariableCompileValue *constValue,
                scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-18h]
    VariableCompileValue constValue2; // [esp+4h] [ebp-14h] BYREF
    int expr_count; // [esp+10h] [ebp-8h]
    bool success; // [esp+17h] [ebp-1h]

    if ( !constValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2369,
                    0,
                    "%s",
                    "constValue") )
    {
        __debugbreak();
    }
    expr_count = GetExpressionCount(exprlist);
    if ( expr_count == 1 )
        return EmitOrEvalExpression(inst, exprlist.node[0].node[0].node[0], constValue, block);
    if ( expr_count == 3 )
    {
        success = 1;
        for ( node = exprlist.node[0].node; node; node = node[1].node )
        {
            if ( success )
            {
                success = EmitOrEvalExpression(inst, *node->node, &constValue2, block);
                if ( success )
                    Scr_PushValue(inst, &constValue2);
            }
            else
            {
                EmitExpression(inst, *node->node, block);
            }
        }
        if ( success )
        {
            if ( gScrCompilePub[inst].value_count < 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            2398,
                            0,
                            "%s",
                            "gScrCompilePub[inst].value_count >= 3") )
            {
                __debugbreak();
            }
            gScrCompilePub[inst].value_count -= 3;
            Scr_CreateVector(inst, &gScrCompileGlob[inst].value_start[gScrCompilePub[inst].value_count], &constValue->value);
            constValue->sourcePos = sourcePos;
            return 1;
        }
        else
        {
            EmitOpcode(inst, 0x7Eu, -2, 0);
            AddOpcodePos(inst, sourcePos.stringValue, 1);
            AddExpressionListOpcodePos(inst, exprlist);
            return 0;
        }
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "expression list must have 1 or 3 parameters");
        return 0;
    }
}

void __cdecl Scr_PushValue(scriptInstance_t inst, VariableCompileValue *constValue)
{
    if (gScrCompilePub[inst].value_count < 32)
        gScrCompileGlob[inst].value_start[gScrCompilePub[inst].value_count++] = *constValue;
    else
        CompileError(inst, constValue->sourcePos.stringValue, "VALUE_STACK_SIZE exceeded");
}

char __cdecl EmitOrEvalExpression(
    scriptInstance_t inst,
    sval_u expr,
    VariableCompileValue *constValue,
    scr_block_s *block)
{
    char v4; // al
    char result; // [esp+7h] [ebp-1h]

    switch (*(_BYTE *)expr.node)
    {
    case 7:
        if (gScrCompileGlob[inst].bConstRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                2694,
                0,
                "%s",
                "!gScrCompileGlob[inst].bConstRefCount"))
        {
            __debugbreak();
        }
        gScrCompileGlob[inst].bConstRefCount = 1;
        result = EmitOrEvalExpression(inst, expr.node[1], constValue, block);
        if (!gScrCompileGlob[inst].bConstRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                2697,
                0,
                "%s",
                "gScrCompileGlob[inst].bConstRefCount"))
        {
            __debugbreak();
        }
        gScrCompileGlob[inst].bConstRefCount = 0;
        v4 = result;
        break;
    case 8:
        v4 = EmitOrEvalPrimitiveExpression(inst, expr.node[1], constValue, block);
        break;
    case 0x31:
        EmitBoolOrExpression(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            block);
        v4 = 0;
        break;
    case 0x32:
        EmitBoolAndExpression(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            block);
        v4 = 0;
        break;
    case 0x33:
        v4 = EmitOrEvalBinaryOperatorExpression(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            constValue,
            block);
        break;
    case 0x34:
        EmitExpression(inst, expr.node[1], block);
        EmitBoolNot(inst, expr.node[2]);
        v4 = 0;
        break;
    case 0x35:
        EmitExpression(inst, expr.node[1], block);
        EmitBoolComplement(inst, expr.node[2]);
        v4 = 0;
        break;
    default:
        v4 = 0;
        break;
    }
    return v4;
}

void __cdecl EmitBoolNot(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x5Cu, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitBoolComplement(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x5Du, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitBoolOrExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr1sourcePos,
                sval_u expr2sourcePos,
                scr_block_s *block)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    intptr_t offset; // [esp+4h] [ebp-8h]  nx-port: a byte count, typed as a pointer
    const char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(inst, expr1, block);
    EmitOpcode(inst, 0x61u, -1, 0);
    AddOpcodePos(inst, expr1sourcePos.stringValue, 0);
    EmitShort(inst, 0);
    pos = gScrCompileGlob[inst].codePos;
    nextPos = TempMallocAlignStrict(0);
    EmitExpression(inst, expr2, block);
    EmitCastBool(inst, expr2sourcePos);
    offset = TempMallocAlignStrict(0) - nextPos;
    if ( (unsigned int)offset >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2564,
                    0,
                    "%s",
                    "offset < 65536") )
    {
        __debugbreak();
    }
    *(_WORD *)pos = (_WORD)offset;
}

void __cdecl EmitCastBool(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x5Bu, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitBoolAndExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr1sourcePos,
                sval_u expr2sourcePos,
                scr_block_s *block)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    intptr_t offset; // [esp+4h] [ebp-8h]  nx-port: a byte count, typed as a pointer
    const char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(inst, expr1, block);
    EmitOpcode(inst, 0x60u, -1, 0);
    AddOpcodePos(inst, expr1sourcePos.stringValue, 0);
    EmitShort(inst, 0);
    pos = gScrCompileGlob[inst].codePos;
    nextPos = TempMallocAlignStrict(0);
    EmitExpression(inst, expr2, block);
    EmitCastBool(inst, expr2sourcePos);
    offset = TempMallocAlignStrict(0) - nextPos;
    if ( (unsigned int)offset >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    2584,
                    0,
                    "%s",
                    "offset < 65536") )
    {
        __debugbreak();
    }
    *(_WORD *)pos = (_WORD)offset;
}

char __cdecl EmitOrEvalBinaryOperatorExpression(
    scriptInstance_t inst,
    sval_u expr1,
    sval_u expr2,
    sval_u opcode,
    sval_u sourcePos,
    VariableCompileValue *constValue,
    scr_block_s *block)
{
    VariableCompileValue constValue2; // [esp+0h] [ebp-18h] BYREF
    VariableCompileValue constValue1; // [esp+Ch] [ebp-Ch] BYREF

    if (!EmitOrEvalExpression(inst, expr1, &constValue1, block))
    {
        EmitExpression(inst, expr2, block);
    emitOpcode:
        EmitOpcode(inst, (char)opcode.type, -1, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 0);
        return 0;
    }
    Scr_PushValue(inst, &constValue1);
    if (!EmitOrEvalExpression(inst, expr2, &constValue2, block))
        goto emitOpcode;
    Scr_PopValue(inst);
    Scr_EvalBinaryOperator(inst, opcode.intValue, &constValue1.value, &constValue2.value);
    if (gScrVarPub[inst].error_message)
    {
        CompileError(inst, sourcePos.stringValue, "%s", gScrVarPub[inst].error_message);
        return 0;
    }
    else
    {
        constValue->value.u = constValue1.value.u;  // nx-port: whole value
        constValue->value.type = constValue1.value.type;
        constValue->sourcePos = sourcePos;
        return 1;
    }
}

void __cdecl Scr_PopValue(scriptInstance_t inst)
{
    if ( !gScrCompilePub[inst].value_count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    1089,
                    0,
                    "%s",
                    "gScrCompilePub[inst].value_count") )
    {
        __debugbreak();
    }
    --gScrCompilePub[inst].value_count;
}

void __cdecl EmitExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    VariableCompileValue constValue; // [esp+0h] [ebp-Ch] BYREF

    if ( EmitOrEvalExpression(inst, expr, &constValue, block) )
        EmitValue(inst, &constValue);
}

void __cdecl EmitVariableExpressionRef(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    switch (*(_BYTE *)expr.node)
    {
    case 4:
        if (gScrCompileGlob[inst].bConstRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                2745,
                0,
                "%s",
                "!gScrCompileGlob[inst].bConstRefCount"))
        {
            __debugbreak();
        }
        gScrCompileGlob[inst].bConstRefCount = 1;
        EmitVariableExpressionRef(inst, expr.node[1], block);
        if (!gScrCompileGlob[inst].bConstRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                2748,
                0,
                "%s",
                "gScrCompileGlob[inst].bConstRefCount"))
        {
            __debugbreak();
        }
        gScrCompileGlob[inst].bConstRefCount = 0;
        break;
    case 5:
        EmitLocalVariableRef(inst, expr.node[1], expr.node[2], block);
        break;
    case 0xF:
        EmitArrayVariableRef(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            block);
        break;
    case 0x11:
        EmitFieldVariableRef(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            block);
        break;
    case 0x37:
    case 0x52:
        if (gScrCompilePub[inst].script_loading)
            CompileError(
                inst,
                expr.node[2].stringValue,
                "$ and self field can only be used in the script debugger");
        else
            CompileError(inst, expr.node[2].stringValue, "not an lvalue");
        break;
    default:
        return;
    }
}

void __cdecl EmitLocalVariableRef(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index; // [esp+0h] [ebp-4h]

    if ( gScrCompilePub[inst].developer_statement == 3 )
    {
        EmitOpcode(inst, 0x84u, 0, 0);
        EmitCanonicalString(inst, expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(inst, expr.stringValue, sourcePos, 1, block);
        EmitOpcode(inst, (index != 0) + 54, 0, 0);
        EmitPreAssignmentPos(inst);
        if ( index )
            EmitByte(inst, index);
        AddOpcodePos(inst, sourcePos.stringValue, 1);
    }
}

void __cdecl EmitFieldVariableRef(
                scriptInstance_t inst,
                sval_u expr,
                sval_u field,
                sval_u sourcePos,
                scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(inst, expr, sourcePos, block);
    EmitOpcode(inst, 0x2Eu, 0, 0);
    EmitCanonicalString(inst, field.stringValue);
}

void __cdecl EmitArrayVariableRef(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, index, block);
    EmitArrayPrimitiveExpressionRef(inst, expr, sourcePos, block);
    EmitEvalArrayRef(inst, sourcePos, indexSourcePos);
}

void __cdecl EmitEvalArrayRef(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(inst, 0x23u, -1, 0);
    AddOpcodePos(inst, indexSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitArrayPrimitiveExpressionRef(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 19 )
    {
        EmitVariableExpressionRef(inst, expr.node[1], block);
    }
    else if ( *(_BYTE *)expr.node == 37 )
    {
        EmitGameRef(inst, expr.node[1]);
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "not an lvalue");
    }
}

void __cdecl EmitGameRef(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x14u, 0, 0);
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitPrimitiveExpressionFieldObject(
                scriptInstance_t inst,
                sval_u expr,
                sval_u sourcePos,
                scr_block_s *block)
{
    switch ( *(_BYTE *)expr.node )
    {
        case 0x13:
            EmitVariableExpression(inst, expr.node[1], block);
            EmitCastFieldObject(inst, expr.node[2]);
            break;
        case 0x15:
            EmitCallExpressionFieldObject(inst, expr.node[1], block);
            break;
        case 0x22:
            EmitSelfObject(inst, expr.node[1]);
            break;
        case 0x24:
            EmitLevelObject(inst, expr.node[1]);
            break;
        case 0x26:
            EmitAnimObject(inst, expr.node[1]);
            break;
        case 0x30:
            EmitExpressionListFieldObject(inst, expr.node[1], sourcePos, block);
            break;
        default:
            CompileError(inst, sourcePos.stringValue, "not an object");
            break;
    }
}

void __cdecl EmitSelfObject(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x26u, 0, 0);
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitLevelObject(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0xDu, 0, 0);
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitAnimObject(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0xEu, 0, 0);
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitCastFieldObject(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x59u, -1, 0);
    EmitPreAssignmentPos(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitCallExpressionFieldObject(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 25 )
    {
        EmitCall(inst, expr.node[1], expr.node[2], 0, block);
        EmitCastFieldObject(inst, expr.node[3]);
    }
    else if ( *(_BYTE *)expr.node == 26 )
    {
        EmitMethod(
            inst,
            expr.node[1],
            expr.node[2],
            expr.node[3],
            expr.node[4],
            0,
            block);
        EmitCastFieldObject(inst, expr.node[5]);
    }
}

void __cdecl EmitExpressionListFieldObject(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    node = GetSingleParameter(exprlist);
    if ( node )
        EmitExpressionFieldObject(inst, *node->node, node->node[1], block);
    else
        CompileError(inst, sourcePos.stringValue, "not an object");
}

sval_u *__cdecl GetSingleParameter(sval_u exprlist)
{
    if ( !exprlist.node[0].stringValue )
        return 0;
    if ( exprlist.node[0].node[1].stringValue )
        return 0;
    return exprlist.node[0].node;
}

void __cdecl EmitExpressionFieldObject(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if ( *(_BYTE *)expr.node == 8 )
        EmitPrimitiveExpressionFieldObject(
            inst,
            expr.node[1],
            expr.node[2],
            block);
    else
        CompileError(inst, sourcePos.stringValue, "not an object");
}

void __cdecl EmitAssignmentStatement(
                scriptInstance_t inst,
                sval_u lhs,
                sval_u rhs,
                sval_u sourcePos,
                sval_u rhsSourcePos,
                scr_block_s *block)
{
    if ( !IsUndefinedExpression(rhs) || !EmitClearVariableExpression(inst, lhs, rhsSourcePos, block) )
    {
        EmitExpression(inst, rhs, block);
        EmitVariableExpressionRef(inst, lhs, block);
        EmitSetVariableField(inst, sourcePos);
    }
}

bool __cdecl IsUndefinedExpression(sval_u expr)
{
    return *(_BYTE *)expr.node == 8 && IsUndefinedPrimitiveExpression(expr.node[1]);
}

bool __cdecl IsUndefinedPrimitiveExpression(sval_u expr)
{
    return *(_BYTE *)expr.node == 33;
}

char __cdecl EmitClearVariableExpression(scriptInstance_t inst, sval_u expr, sval_u rhsSourcePos, scr_block_s *block)
{
    switch ( *(_BYTE *)expr.node )
    {
        case 5:
            return 0;
        case 0xF:
            EmitClearArrayVariable(
                inst,
                expr.node[1],
                expr.node[2],
                expr.node[3],
                expr.node[4],
                block);
            return 1;
        case 0x11:
            EmitClearFieldVariable(
                inst,
                expr.node[1],
                expr.node[2],
                expr.node[3],
                rhsSourcePos,
                block);
            return 1;
        case 0x37:
        case 0x52:
            if ( gScrCompilePub[inst].script_loading )
                CompileError(
                    inst,
                    expr.node[2].stringValue,
                    "$ and self field can only be used in the script debugger");
            else
                CompileError(inst, expr.node[2].stringValue, "not an lvalue");
            return 1;
        default:
            return 1;
    }
}

void __cdecl EmitClearFieldVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u field,
                sval_u sourcePos,
                sval_u rhsSourcePos,
                scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(inst, expr, sourcePos, block);
    EmitOpcode(inst, 0x2Fu, 0, 0);
    AddOpcodePos(inst, rhsSourcePos.stringValue, 0);
    EmitCanonicalString(inst, field.stringValue);
    EmitAssignmentPos(inst);
}

void __cdecl EmitClearArrayVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, index, block);
    EmitArrayPrimitiveExpressionRef(inst, expr, sourcePos, block);
    EmitClearArray(inst, sourcePos, indexSourcePos);
}

void __cdecl EmitClearArray(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(inst, 0x24u, -1, 0);
    AddOpcodePos(inst, indexSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitAssignmentPos(inst);
}

void __cdecl EmitCallExpressionStatement(scriptInstance_t inst, sval_u expr, scr_block_s *block)
{
    EmitCallExpression(inst, expr, 1, block);
}

void __cdecl EmitReturnStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if ( !block->abortLevel )
        block->abortLevel = 3;
    EmitExpression(inst, expr, block);
    EmitReturn(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitReturn(scriptInstance_t inst)
{
    EmitOpcode(inst, 1u, -1, 0);
    EmitPreAssignmentPos(inst);
}

void __cdecl EmitEndStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block)
{
    if ( !block->abortLevel )
        block->abortLevel = 3;
    EmitEnd(inst);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
}

void __cdecl EmitWaitStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, expr, block);
    EmitOpcode(inst, 0x4Cu, -1, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitWaittillFrameEnd(scriptInstance_t inst, sval_u sourcePos)
{
    EmitOpcode(inst, 0x4Du, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitIfStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmt,
                sval_u sourcePos,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block,
                sval_u *ifStatBlock)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    intptr_t offset; // [esp+4h] [ebp-8h]  nx-port: a byte count, typed as a pointer
    const char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(inst, expr, block);
    EmitOpcode(inst, 0x5Eu, -1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitShort(inst, 0);
    pos = gScrCompileGlob[inst].codePos;
    nextPos = TempMallocAlignStrict(0);
    Scr_TransferBlock(block, ifStatBlock->block);
    EmitStatement(inst, stmt, lastStatement, endSourcePos, ifStatBlock->block);
    if ( ifStatBlock->block->localVarsPublicCount != block->localVarsCreateCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    3017,
                    0,
                    "%s",
                    "ifStatBlock->block->localVarsPublicCount == block->localVarsCreateCount") )
    {
        __debugbreak();
    }
    EmitNOP2(inst, lastStatement, endSourcePos, ifStatBlock->block);
    offset = TempMallocAlignStrict(0) - nextPos;
    if ( (unsigned int)offset >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    3020,
                    0,
                    "%s",
                    "offset < 65536") )
    {
        __debugbreak();
    }
    *(_WORD *)pos = (_WORD)offset;
}

void __cdecl EmitNOP2(scriptInstance_t inst, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    unsigned int checksum; // [esp+0h] [ebp-4h]

    checksum = gScrVarPub[inst].checksum;
    if (lastStatement)
    {
        EmitEnd(inst);
        AddOpcodePos(inst, endSourcePos, 1);
    }
    else
    {
        EmitRemoveLocalVars(inst, block, block);
    }
    gScrVarPub[inst].checksum = checksum + 1;
}

void __cdecl EmitRemoveLocalVars(scriptInstance_t inst, scr_block_s *block, scr_block_s *outerBlock)
{
    int removeCount; // [esp+0h] [ebp-4h]

    if ( !block->abortLevel )
    {
        if ( block->localVarsCreateCount < block->localVarsPublicCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        722,
                        0,
                        "%s",
                        "block->localVarsCreateCount >= block->localVarsPublicCount") )
        {
            __debugbreak();
        }
        if ( block->localVarsPublicCount < outerBlock->localVarsPublicCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        723,
                        0,
                        "%s",
                        "block->localVarsPublicCount >= outerBlock->localVarsPublicCount") )
        {
            __debugbreak();
        }
        removeCount = block->localVarsCreateCount - outerBlock->localVarsPublicCount;
        if ( removeCount < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        725,
                        0,
                        "%s",
                        "removeCount >= 0") )
        {
            __debugbreak();
        }
        if ( removeCount )
        {
            EmitOpcode(inst, 0x17u, 0, 0);
            EmitByte(inst, removeCount);
            block->localVarsCreateCount = block->localVarsPublicCount;
        }
    }
}

void __cdecl Scr_TransferBlock(scr_block_s *from, scr_block_s *to)
{
    unsigned int v2; // edx
    int j; // [esp+4h] [ebp-10h]
    unsigned int localVar; // [esp+8h] [ebp-Ch]
    unsigned int localVar_4; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( to->localVarsPublicCount > from->localVarsCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    924,
                    0,
                    "%s",
                    "to->localVarsPublicCount <= from->localVarsCount") )
    {
        __debugbreak();
    }
    for ( i = 0; i < to->localVarsPublicCount || i < from->localVarsCreateCount; ++i )
    {
        localVar = from->localVars[i].name;
        localVar_4 = from->localVars[i].sourcePos;
        j = Scr_FindLocalVar(to, i, localVar);
        if ( j < 0 )
        {
            j = to->localVarsCount;
            Scr_CheckLocalVarsCount(j);
            ++to->localVarsCount;
        }
        if ( j >= to->localVarsPublicCount
            && ++to->localVarsPublicCount > from->localVarsCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        939,
                        0,
                        "%s",
                        "to->localVarsPublicCount <= from->localVarsCount") )
        {
            __debugbreak();
        }
        while ( j > i )
        {
            v2 = *(unsigned int *)&to->localVarsInitBits[8 * j + 4];
            to->localVars[j].name = *(unsigned int *)&to->localVarsInitBits[8 * j];
            to->localVars[j--].sourcePos = v2;
        }
        to->localVars[i].name = localVar;
        to->localVars[i].sourcePos = localVar_4;
        if ( ((1 << (i & 7)) & from->localVarsInitBits[i >> 3]) != 0 )
            to->localVarsInitBits[i >> 3] |= 1 << (i & 7);
    }
    if ( from->localVarsCreateCount > to->localVarsPublicCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    951,
                    0,
                    "%s",
                    "from->localVarsCreateCount <= to->localVarsPublicCount") )
    {
        __debugbreak();
    }
    to->localVarsCreateCount = from->localVarsCreateCount;
    to->abortLevel = 0;
}

void __cdecl EmitIfElseStatement(
    scriptInstance_t inst,
    sval_u expr,
    sval_u stmt1,
    sval_u stmt2,
    sval_u sourcePos,
    sval_u elseSourcePos,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block,
    sval_u *ifStatBlock,
    sval_u *elseStatBlock)
{
    unsigned int checksum; // [esp+0h] [ebp-24h]
    intptr_t offset; // [esp+4h] [ebp-20h]  nx-port: a byte count, typed as a pointer
    const char *nextPos1; // [esp+8h] [ebp-1Ch]
    unsigned __int8 *pos1; // [esp+Ch] [ebp-18h]
    scr_block_s *childBlocks[2]; // [esp+10h] [ebp-14h] BYREF
    const char *nextPos2; // [esp+18h] [ebp-Ch]
    int childCount; // [esp+1Ch] [ebp-8h]
    const char *pos2; // [esp+20h] [ebp-4h]

    childCount = 0;
    EmitExpression(inst, expr, block);
    EmitOpcode(inst, 0x5Eu, -1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitShort(inst, 0);
    pos1 = gScrCompileGlob[inst].codePos;
    nextPos1 = TempMallocAlignStrict(0);
    Scr_TransferBlock(block, ifStatBlock->block);
    EmitStatement(inst, stmt1, lastStatement, endSourcePos, ifStatBlock->block);
    EmitRemoveLocalVars(inst, ifStatBlock->block, ifStatBlock->block);
    if (!ifStatBlock->block->abortLevel)
        childBlocks[childCount++] = ifStatBlock->block;
    checksum = gScrVarPub[inst].checksum;
    if (lastStatement)
    {
        EmitEnd(inst);
        EmitCodeInt(inst, 0);
        AddOpcodePos(inst, endSourcePos, 1);
        pos2 = 0;
        nextPos2 = 0;
    }
    else
    {
        EmitOpcode(inst, 0x62u, 0, 0);
        AddOpcodePos(inst, elseSourcePos.stringValue, 1);
        EmitCodeInt(inst, 0);
        pos2 = (const char *)gScrCompileGlob[inst].codePos;
        nextPos2 = TempMallocAlignStrict(0);
    }
    gScrVarPub[inst].checksum = checksum + 1;
    offset = TempMallocAlignStrict(0) - nextPos1;
    if ((unsigned int)offset >= 0x10000
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3084,
            0,
            "%s",
            "offset < 65536"))
    {
        __debugbreak();
    }
    *(_WORD *)pos1 = (_WORD)offset;
    Scr_TransferBlock(block, elseStatBlock->block);
    EmitStatement(inst, stmt2, lastStatement, endSourcePos, elseStatBlock->block);
    EmitNOP2(inst, lastStatement, endSourcePos, elseStatBlock->block);
    if (!elseStatBlock->block->abortLevel)
        childBlocks[childCount++] = elseStatBlock->block;
    if (!lastStatement)
        *(_DWORD *)pos2 = TempMallocAlignStrict(0) - nextPos2;
    Scr_InitFromChildBlocks(childBlocks, childCount, block);
}

void __cdecl Scr_InitFromChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
    scr_block_s *childBlock; // [esp+0h] [ebp-14h]
    scr_block_s *childBlocka; // [esp+0h] [ebp-14h]
    int childIndex; // [esp+4h] [ebp-10h]
    int childIndexa; // [esp+4h] [ebp-10h]
    int localVarsCreateCount; // [esp+8h] [ebp-Ch]
    unsigned int name; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( childCount )
    {
        localVarsCreateCount = (*childBlocks)->localVarsPublicCount;
        for ( childIndex = 1; childIndex < childCount; ++childIndex )
        {
            childBlock = childBlocks[childIndex];
            if ( childBlock->localVarsPublicCount < localVarsCreateCount )
                localVarsCreateCount = childBlock->localVarsPublicCount;
        }
        if ( block->localVarsCreateCount > localVarsCreateCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        820,
                        0,
                        "%s",
                        "block->localVarsCreateCount <= localVarsCreateCount") )
        {
            __debugbreak();
        }
        if ( localVarsCreateCount > block->localVarsCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        821,
                        0,
                        "%s",
                        "localVarsCreateCount <= block->localVarsCount") )
        {
            __debugbreak();
        }
        block->localVarsCreateCount = localVarsCreateCount;
        for ( i = 0; i < localVarsCreateCount; ++i )
        {
            if ( i >= block->localVarsCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                            826,
                            0,
                            "%s",
                            "i < block->localVarsCount") )
            {
                __debugbreak();
            }
            if ( ((1 << (i & 7)) & block->localVarsInitBits[i >> 3]) == 0 )
            {
                name = block->localVars[i].name;
                for ( childIndexa = 0; childIndexa < childCount; ++childIndexa )
                {
                    childBlocka = childBlocks[childIndexa];
                    if ( localVarsCreateCount > childBlocka->localVarsPublicCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                                    833,
                                    0,
                                    "%s",
                                    "localVarsCreateCount <= childBlock->localVarsPublicCount") )
                    {
                        __debugbreak();
                    }
                    if ( i >= childBlocka->localVarsPublicCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                                    834,
                                    0,
                                    "%s",
                                    "i < childBlock->localVarsPublicCount") )
                    {
                        __debugbreak();
                    }
                    if ( childBlocka->localVars[i].name != name
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                                    835,
                                    0,
                                    "%s",
                                    "childBlock->localVars[i].name == name") )
                    {
                        __debugbreak();
                    }
                    if ( ((1 << (i & 7)) & childBlocka->localVarsInitBits[i >> 3]) == 0 )
                        goto LABEL_14;
                }
                block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
            }
LABEL_14:
            ;
        }
    }
}

void __cdecl EmitWhileStatement(
    scriptInstance_t inst,
    sval_u expr,
    sval_u stmt,
    sval_u sourcePos,
    sval_u whileSourcePos,
    scr_block_s *block,
    sval_u *whileStatBlock)
{
    ContinueStatementInfo *oldContinueStatement; // [esp+0h] [ebp-48h]
    int breakChildCount; // [esp+4h] [ebp-44h] BYREF
    int *oldBreakChildCount; // [esp+8h] [ebp-40h]
    scr_block_s **breakChildBlocks; // [esp+Ch] [ebp-3Ch]
    BreakStatementInfo *oldBreakStatement; // [esp+10h] [ebp-38h]
    bool constConditional; // [esp+17h] [ebp-31h]
    unsigned int offset; // [esp+18h] [ebp-30h]
    bool bOldCanBreak; // [esp+1Eh] [ebp-2Ah]
    bool bOldCanContinue; // [esp+1Fh] [ebp-29h]
    int *oldContinueChildCount; // [esp+20h] [ebp-28h]
    VariableCompileValue constValue; // [esp+24h] [ebp-24h] BYREF
    const char *pos1; // [esp+30h] [ebp-18h]
    const char *nextPos2; // [esp+34h] [ebp-14h]
    scr_block_s **oldBreakChildBlocks; // [esp+38h] [ebp-10h]
    const char *pos2; // [esp+3Ch] [ebp-Ch]
    scr_block_s **oldContinueChildBlocks; // [esp+40h] [ebp-8h]
    scr_block_s *oldBreakBlock; // [esp+44h] [ebp-4h]

    bOldCanBreak = gScrCompileGlob[inst].bCanBreak;
    oldBreakStatement = gScrCompileGlob[inst].currentBreakStatement;
    gScrCompileGlob[inst].bCanBreak = 0;
    bOldCanContinue = gScrCompileGlob[inst].bCanContinue;
    oldContinueStatement = gScrCompileGlob[inst].currentContinueStatement;
    gScrCompileGlob[inst].bCanContinue = 0;
    Scr_TransferBlock(block, whileStatBlock->block);
    EmitCreateLocalVars(inst, whileStatBlock->block);
    if (whileStatBlock->block->localVarsCreateCount > block->localVarsCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3201,
            0,
            "%s",
            "whileStatBlock->block->localVarsCreateCount <= block->localVarsCount"))
    {
        __debugbreak();
    }
    block->localVarsCreateCount = whileStatBlock->block->localVarsCreateCount;
    pos1 = TempMallocAlignStrict(0);
    constConditional = 0;
    if (EmitOrEvalExpression(inst, expr, &constValue, block))
    {
        if (constValue.value.type == 6 || constValue.value.type == 5)
        {
            Scr_CastBool(inst, &constValue.value);
            if (!constValue.value.u.intValue)
                CompileError(inst, sourcePos.stringValue, "conditional expression cannot be always false");
            constConditional = 1;
        }
        else
        {
            EmitValue(inst, &constValue);
        }
    }
    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    oldBreakBlock = gScrCompileGlob[inst].breakBlock;
    oldContinueChildBlocks = gScrCompileGlob[inst].continueChildBlocks;
    oldContinueChildCount = gScrCompileGlob[inst].continueChildCount;
    breakChildCount = 0;
    gScrCompileGlob[inst].continueChildBlocks = 0;
    gScrCompileGlob[inst].breakBlock = whileStatBlock->block;
    if (constConditional)
    {
        pos2 = 0;
        nextPos2 = 0;
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "EmitWhileStatement");
        gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    }
    else
    {
        EmitOpcode(inst, 0x5Eu, -1, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 0);
        EmitShort(inst, 0);
        pos2 = (const char *)gScrCompileGlob[inst].codePos;
        nextPos2 = TempMallocAlignStrict(0);
        breakChildBlocks = 0;
    }
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    gScrCompileGlob[inst].bCanBreak = 1;
    gScrCompileGlob[inst].currentBreakStatement = 0;
    gScrCompileGlob[inst].bCanContinue = 1;
    gScrCompileGlob[inst].currentContinueStatement = 0;
    EmitStatement(inst, stmt, 0, 0, whileStatBlock->block);
    if (whileStatBlock->block->abortLevel != 3)
        whileStatBlock->block->abortLevel = 0;
    gScrCompileGlob[inst].bCanBreak = 0;
    gScrCompileGlob[inst].bCanContinue = 0;
    ConnectContinueStatements(inst);
    EmitOpcode(inst, 0x63u, 0, 0);
    AddOpcodePos(inst, whileSourcePos.stringValue, 0);
    if (*(_BYTE *)stmt.node == 46)
        AddOpcodePos(inst, stmt.node[3].stringValue, 1);
    EmitShort(inst, 0);
    offset = TempMallocAlignStrict(0) - pos1;
    if (offset >= 0x10000
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3280,
            0,
            "%s",
            "offset < 65536"))
    {
        __debugbreak();
    }
    *(_WORD *)gScrCompileGlob[inst].codePos = offset;
    if (pos2)
    {
        offset = TempMallocAlignStrict(0) - nextPos2;
        if (offset >= 0x10000
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                3285,
                0,
                "%s",
                "offset < 65536"))
        {
            __debugbreak();
        }
        *(_WORD *)pos2 = offset;
    }
    ConnectBreakStatements(inst);
    gScrCompileGlob[inst].bCanBreak = bOldCanBreak;
    gScrCompileGlob[inst].currentBreakStatement = oldBreakStatement;
    gScrCompileGlob[inst].bCanContinue = bOldCanContinue;
    gScrCompileGlob[inst].currentContinueStatement = oldContinueStatement;
    if (constConditional)
        Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
    gScrCompileGlob[inst].breakBlock = oldBreakBlock;
    gScrCompileGlob[inst].continueChildBlocks = oldContinueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = oldContinueChildCount;
}

void __cdecl EmitCreateLocalVars(scriptInstance_t inst, scr_block_s *block)
{
    int i; // [esp+0h] [ebp-4h]

    if ( block->localVarsPublicCount < block->localVarsCreateCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    699,
                    0,
                    "%s",
                    "block->localVarsPublicCount >= block->localVarsCreateCount") )
    {
        __debugbreak();
    }
    if ( block->localVarsCreateCount != block->localVarsPublicCount )
    {
        for ( i = block->localVarsCreateCount; i < block->localVarsPublicCount; ++i )
        {
            EmitOpcode(inst, 0x16u, 0, 0);
            EmitCanonicalStringConst(inst, block->localVars[i].name);
            AddOpcodePos(inst, block->localVars[i].sourcePos, 0);
        }
        block->localVarsCreateCount = block->localVarsPublicCount;
    }
}

void __cdecl ConnectBreakStatements(scriptInstance_t inst)
{
    BreakStatementInfo *breakStatement; // [esp+0h] [ebp-8h]
    const char *codePos; // [esp+4h] [ebp-4h]

    if (gScrCompilePub[inst].value_count
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            2873,
            0,
            "%s",
            "!gScrCompilePub[inst].value_count"))
    {
        __debugbreak();
    }
    codePos = TempMallocAlignStrict(0);
    for (breakStatement = gScrCompileGlob[inst].currentBreakStatement; breakStatement; breakStatement = breakStatement->next)
        *(_DWORD *)breakStatement->codePos = codePos - breakStatement->nextCodePos;
}

void __cdecl ConnectContinueStatements(scriptInstance_t inst)
{
    ContinueStatementInfo *continueStatement; // [esp+0h] [ebp-8h]
    const char *codePos; // [esp+4h] [ebp-4h]

    codePos = TempMallocAlignStrict(0);
    for (continueStatement = gScrCompileGlob[inst].currentContinueStatement;
        continueStatement;
        continueStatement = continueStatement->next)
    {
        *(_DWORD *)continueStatement->codePos = codePos - continueStatement->nextCodePos;
    }
}

void __cdecl EmitForStatement(
    scriptInstance_t inst,
    sval_u stmt1,
    sval_u expr,
    sval_u stmt2,
    sval_u stmt,
    sval_u sourcePos,
    sval_u forSourcePos,
    scr_block_s *block,
    sval_u *forStatBlock,
    sval_u *forStatPostBlock)
{
    ContinueStatementInfo *oldContinueStatement; // [esp+0h] [ebp-50h]
    int breakChildCount; // [esp+4h] [ebp-4Ch] BYREF
    int *oldBreakChildCount; // [esp+8h] [ebp-48h]
    scr_block_s **breakChildBlocks; // [esp+Ch] [ebp-44h]
    BreakStatementInfo *oldBreakStatement; // [esp+10h] [ebp-40h]
    bool constConditional; // [esp+17h] [ebp-39h]
    unsigned int offset; // [esp+18h] [ebp-38h]
    bool bOldCanBreak; // [esp+1Eh] [ebp-32h]
    bool bOldCanContinue; // [esp+1Fh] [ebp-31h]
    int continueChildCount; // [esp+20h] [ebp-30h] BYREF
    int *oldContinueChildCount; // [esp+24h] [ebp-2Ch]
    VariableCompileValue constValue; // [esp+28h] [ebp-28h] BYREF
    const char *pos1; // [esp+34h] [ebp-1Ch]
    const char *nextPos2; // [esp+38h] [ebp-18h]
    scr_block_s **continueChildBlocks; // [esp+3Ch] [ebp-14h]
    scr_block_s **oldBreakChildBlocks; // [esp+40h] [ebp-10h]
    const char *pos2; // [esp+44h] [ebp-Ch]
    scr_block_s **oldContinueChildBlocks; // [esp+48h] [ebp-8h]
    scr_block_s *oldBreakBlock; // [esp+4Ch] [ebp-4h]

    bOldCanBreak = gScrCompileGlob[inst].bCanBreak;
    oldBreakStatement = gScrCompileGlob[inst].currentBreakStatement;
    gScrCompileGlob[inst].bCanBreak = 0;
    bOldCanContinue = gScrCompileGlob[inst].bCanContinue;
    oldContinueStatement = gScrCompileGlob[inst].currentContinueStatement;
    gScrCompileGlob[inst].bCanContinue = 0;
    EmitStatement(inst, stmt1, 0, 0, block);
    Scr_TransferBlock(block, forStatBlock->block);
    EmitCreateLocalVars(inst, forStatBlock->block);
    if (forStatBlock->block->localVarsCreateCount > block->localVarsCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3417,
            0,
            "%s",
            "forStatBlock->block->localVarsCreateCount <= block->localVarsCount"))
    {
        __debugbreak();
    }
    block->localVarsCreateCount = forStatBlock->block->localVarsCreateCount;
    Scr_TransferBlock(block, forStatPostBlock->block);
    pos1 = TempMallocAlignStrict(0);
    if (*(_BYTE *)expr.node == 67)
    {
        constConditional = 0;
        if (EmitOrEvalExpression(inst, expr.node[1], &constValue, block))
        {
            if (constValue.value.type == 6 || constValue.value.type == 5)
            {
                Scr_CastBool(inst, &constValue.value);
                if (!constValue.value.u.intValue)
                    CompileError(inst, sourcePos.stringValue, "conditional expression cannot be always false");
                constConditional = 1;
            }
            else
            {
                EmitValue(inst, &constValue);
            }
        }
    }
    else
    {
        constConditional = 1;
    }
    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    oldBreakBlock = gScrCompileGlob[inst].breakBlock;
    oldContinueChildBlocks = gScrCompileGlob[inst].continueChildBlocks;
    oldContinueChildCount = gScrCompileGlob[inst].continueChildCount;
    breakChildCount = 0;
    continueChildCount = 0;
    continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "EmitForStatement");
    gScrCompileGlob[inst].continueChildBlocks = continueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = &continueChildCount;
    gScrCompileGlob[inst].breakBlock = forStatBlock->block;
    if (constConditional)
    {
        pos2 = 0;
        nextPos2 = 0;
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "EmitForStatement");
        gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    }
    else
    {
        EmitOpcode(inst, 0x5Eu, -1, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 0);
        EmitShort(inst, 0);
        pos2 = (const char *)gScrCompileGlob[inst].codePos;
        nextPos2 = TempMallocAlignStrict(0);
        breakChildBlocks = 0;
    }
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    gScrCompileGlob[inst].bCanBreak = 1;
    gScrCompileGlob[inst].currentBreakStatement = 0;
    gScrCompileGlob[inst].bCanContinue = 1;
    gScrCompileGlob[inst].currentContinueStatement = 0;
    EmitStatement(inst, stmt, 0, 0, forStatBlock->block);
    Scr_AddContinueBlock(inst, forStatBlock->block);
    gScrCompileGlob[inst].bCanBreak = 0;
    gScrCompileGlob[inst].bCanContinue = 0;
    ConnectContinueStatements(inst);
    Scr_InitFromChildBlocks(continueChildBlocks, continueChildCount, forStatPostBlock->block);
    EmitStatement(inst, stmt2, 0, 0, forStatPostBlock->block);
    EmitOpcode(inst, 0x63u, 0, 0);
    AddOpcodePos(inst, forSourcePos.stringValue, 0);
    if (*(_BYTE *)stmt.node == 46)
        AddOpcodePos(inst, stmt.node[3].stringValue, 1);
    EmitShort(inst, 0);
    offset = TempMallocAlignStrict(0) - pos1;
    if (offset >= 0x10000
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3509,
            0,
            "%s",
            "offset < 65536"))
    {
        __debugbreak();
    }
    *(_WORD *)gScrCompileGlob[inst].codePos = offset;
    if (pos2)
    {
        offset = TempMallocAlignStrict(0) - nextPos2;
        if (offset >= 0x10000
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                3514,
                0,
                "%s",
                "offset < 65536"))
        {
            __debugbreak();
        }
        *(_WORD *)pos2 = offset;
    }
    ConnectBreakStatements(inst);
    gScrCompileGlob[inst].bCanBreak = bOldCanBreak;
    gScrCompileGlob[inst].currentBreakStatement = oldBreakStatement;
    gScrCompileGlob[inst].bCanContinue = bOldCanContinue;
    gScrCompileGlob[inst].currentContinueStatement = oldContinueStatement;
    if (constConditional)
        Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
    gScrCompileGlob[inst].breakBlock = oldBreakBlock;
    gScrCompileGlob[inst].continueChildBlocks = oldContinueChildBlocks;
    gScrCompileGlob[inst].continueChildCount = oldContinueChildCount;
}

void __cdecl EmitIncStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (gScrCompileGlob[inst].forceNotCreate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3629,
            0,
            "%s",
            "!gScrCompileGlob[inst].forceNotCreate"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].forceNotCreate = 1;
    EmitVariableExpressionRef(inst, expr, block);
    if (!gScrCompileGlob[inst].forceNotCreate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3632,
            0,
            "%s",
            "gScrCompileGlob[inst].forceNotCreate"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].forceNotCreate = 0;
    EmitOpcode(inst, 0x64u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitSetVariableField(inst, sourcePos);
}

void __cdecl EmitDecStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (gScrCompileGlob[inst].forceNotCreate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3648,
            0,
            "%s",
            "!gScrCompileGlob[inst].forceNotCreate"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].forceNotCreate = 1;
    EmitVariableExpressionRef(inst, expr, block);
    if (!gScrCompileGlob[inst].forceNotCreate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
            3651,
            0,
            "%s",
            "gScrCompileGlob[inst].forceNotCreate"))
    {
        __debugbreak();
    }
    gScrCompileGlob[inst].forceNotCreate = 0;
    EmitOpcode(inst, 0x65u, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitSetVariableField(inst, sourcePos);
}

void __cdecl EmitWaittillStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    node = exprlist.node[0].node[1].node;
    if ( !node
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp", 3700, 0, "%s", "node") )
    {
        __debugbreak();
    }
    EmitExpression(inst, *node->node, block);
    EmitPrimitiveExpression(inst, obj, block);
    EmitOpcode(inst, 0x78u, -2, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    AddOpcodePos(inst, node->node[1].stringValue, 0);
    EmitFormalWaittillParameterListRefInternal(inst, node, block);
    EmitOpcode(inst, 0x34u, 0, 0);
}

void __cdecl EmitFormalWaittillParameterListRefInternal(scriptInstance_t inst, sval_u *node, scr_block_s *block)
{
    while ( 1 )
    {
        node = node[1].node;
        if ( !node )
            break;
        EmitSafeSetWaittillVariableField(inst, *node->node, node->node[1], block);
    }
}

void __cdecl EmitSafeSetWaittillVariableField(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    unsigned __int8 index; // [esp+0h] [ebp-4h]

    index = Scr_FindLocalVarIndex(inst, expr.stringValue, sourcePos, 1, block);
    EmitOpcode(inst, 0x33u, 0, 0);
    EmitPreAssignmentPos(inst);
    EmitByte(inst, index);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitAssignmentPos(inst);
}

void __cdecl EmitWaittillmatchStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    sval_u *nodea; // [esp+0h] [ebp-8h]
    int exprCount; // [esp+4h] [ebp-4h]

    node = exprlist.node[0].node[1].node;
    if ( !node
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp", 3732, 0, "%s", "node") )
    {
        __debugbreak();
    }
    for ( exprCount = 0; ; ++exprCount )
    {
        node = node[1].node;
        if ( !node )
            break;
        EmitExpression(inst, *node->node, block);
    }
    nodea = exprlist.node[0].node[1].node;
    if ( !nodea
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp", 3746, 0, "%s", "node") )
    {
        __debugbreak();
    }
    EmitExpression(inst, *nodea->node, block);
    EmitPrimitiveExpression(inst, obj, block);
    EmitOpcode(inst, 0x77u, -2 - exprCount, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, waitSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    AddOpcodePos(inst, nodea->node[1].stringValue, 0);
    while ( 1 )
    {
        nodea = nodea[1].node;
        if ( !nodea )
            break;
        AddOpcodePos(inst, nodea->node[1].stringValue, 0);
    }
    if ( exprCount >= 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    3765,
                    0,
                    "%s",
                    "exprCount < 256") )
    {
        __debugbreak();
    }
    EmitByte(inst, exprCount);
    EmitOpcode(inst, 0x34u, 0, 0);
}

void __cdecl EmitNotifyStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u notifySourcePos,
                scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-Ch]
    sval_u *start_node; // [esp+4h] [ebp-8h]
    int expr_count; // [esp+8h] [ebp-4h]

    EmitOpcode(inst, 0x7Bu, 1, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 1);
    expr_count = 0;
    start_node = 0;
    for ( node = exprlist.node[0].node; node; node = node[1].node )
    {
        start_node = node;
        EmitExpression(inst, *node->node, block);
        ++expr_count;
    }
    if ( !start_node
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    3791,
                    0,
                    "%s",
                    "start_node") )
    {
        __debugbreak();
    }
    EmitPrimitiveExpression(inst, obj, block);
    EmitOpcode(inst, 0x79u, -expr_count - 2, 0);
    AddOpcodePos(inst, notifySourcePos.stringValue, 16);
    AddOpcodePos(inst, start_node->node[1].stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitEndOnStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u expr,
                sval_u sourcePos,
                sval_u exprSourcePos,
                scr_block_s *block)
{
    EmitExpression(inst, expr, block);
    EmitPrimitiveExpression(inst, obj, block);
    EmitOpcode(inst, 0x7Au, -2, 0);
    AddOpcodePos(inst, exprSourcePos.stringValue, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitSwitchStatement(
    scriptInstance_t inst,
    sval_u expr,
    sval_u stmtlist,
    sval_u sourcePos,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block)
{
    CaseStatementInfo *oldCaseStatement; // [esp+0h] [ebp-24h]
    char *pos3; // [esp+4h] [ebp-20h]
    BreakStatementInfo *oldBreakStatement; // [esp+8h] [ebp-1Ch]
    bool bOldCanBreak; // [esp+Fh] [ebp-15h]
    char *nextPos1; // [esp+10h] [ebp-14h]
    CaseStatementInfo *caseStatement; // [esp+14h] [ebp-10h]
    CaseStatementInfo *caseStatementa; // [esp+14h] [ebp-10h]
    unsigned __int8 *pos1; // [esp+18h] [ebp-Ch]
    signed int num; // [esp+1Ch] [ebp-8h]
    unsigned __int8 *pos2; // [esp+20h] [ebp-4h]

    oldCaseStatement = gScrCompileGlob[inst].currentCaseStatement;
    bOldCanBreak = gScrCompileGlob[inst].bCanBreak;
    oldBreakStatement = gScrCompileGlob[inst].currentBreakStatement;
    gScrCompileGlob[inst].bCanBreak = 0;
    EmitExpression(inst, expr, block);
    EmitOpcode(inst, 0x7Cu, -1, 0);
    EmitCodeInt(inst, 0);
    pos1 = gScrCompileGlob[inst].codePos;
    nextPos1 = TempMallocAlignStrict(0);
    gScrCompileGlob[inst].currentCaseStatement = 0;
    gScrCompileGlob[inst].currentBreakStatement = 0;
    EmitSwitchStatementList(inst, stmtlist, lastStatement, endSourcePos, block);
    EmitOpcode(inst, 0x7Du, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitShort(inst, 0);
    pos2 = gScrCompileGlob[inst].codePos;
    *(_DWORD *)pos1 = pos2 - (unsigned __int8 *)nextPos1;
    pos3 = TempMallocAlignStrict(0);
    num = 0;
    caseStatement = gScrCompileGlob[inst].currentCaseStatement;
    while (caseStatement)
    {
        EmitCodeInt(inst, caseStatement->name);
        EmitCodepos(inst, caseStatement->codePos);
        caseStatement = caseStatement->next;
        ++num;
    }
    *(_WORD *)pos2 = num;
    // nx-port: entries are SCR_CASE_ENTRY_SIZE bytes (a 4-byte name, a code position)
    qsort(pos3, num, SCR_CASE_ENTRY_SIZE, (int(__cdecl *)(const void *, const void *))CompareCaseInfo);
    while (num > 1)
    {
        if (Scr_CaseEntryName((const char *)pos3) == Scr_CaseEntryName((const char *)pos3 + SCR_CASE_ENTRY_SIZE))
        {
            for (caseStatementa = gScrCompileGlob[inst].currentCaseStatement;
                caseStatementa;
                caseStatementa = caseStatementa->next)
            {
                if (caseStatementa->name == Scr_CaseEntryName((const char *)pos3))
                {
                    CompileError(inst, caseStatementa->sourcePos, "duplicate case expression");
                    return;
                }
            }
        }
        --num;
        pos3 += SCR_CASE_ENTRY_SIZE;
    }
    ConnectBreakStatements(inst);
    gScrCompileGlob[inst].currentCaseStatement = oldCaseStatement;
    gScrCompileGlob[inst].bCanBreak = bOldCanBreak;
    gScrCompileGlob[inst].currentBreakStatement = oldBreakStatement;
}

void __cdecl EmitSwitchStatementList(
    scriptInstance_t inst,
    sval_u val,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block)
{
    sval_u *node; // [esp+4h] [ebp-20h]
    sval_u *nextNode; // [esp+8h] [ebp-1Ch]
    int breakChildCount; // [esp+Ch] [ebp-18h] BYREF
    scr_block_s **breakChildBlocks; // [esp+10h] [ebp-14h]
    int *oldBreakChildCount; // [esp+14h] [ebp-10h]
    bool hasDefault; // [esp+1Bh] [ebp-9h]
    scr_block_s **oldBreakChildBlocks; // [esp+1Ch] [ebp-8h]
    scr_block_s *oldBreakBlock; // [esp+20h] [ebp-4h]

    oldBreakChildBlocks = gScrCompileGlob[inst].breakChildBlocks;
    oldBreakChildCount = gScrCompileGlob[inst].breakChildCount;
    oldBreakBlock = gScrCompileGlob[inst].breakBlock;
    breakChildCount = 0;
    breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(512 * sizeof(scr_block_s *), "EmitSwitchStatementList");
    gScrCompileGlob[inst].breakChildBlocks = breakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = &breakChildCount;
    gScrCompileGlob[inst].breakBlock = 0;
    hasDefault = 0;
    for (node = val.node[0].node[1].node; node; node = nextNode)
    {
        nextNode = node[1].node;
        if (*(_BYTE *)node->node == 63 || *(_BYTE *)node->node == 64)
        {
            if (gScrCompileGlob[inst].breakBlock)
            {
                if (!gScrCompileGlob[inst].bCanBreak
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        3904,
                        0,
                        "%s",
                        "gScrCompileGlob[inst].bCanBreak"))
                {
                    __debugbreak();
                }
                gScrCompileGlob[inst].bCanBreak = 0;
                EmitRemoveLocalVars(inst, gScrCompileGlob[inst].breakBlock, gScrCompileGlob[inst].breakBlock);
            }
            if (*(_BYTE *)node->node == 63)
            {
                gScrCompileGlob[inst].breakBlock = node->node[3].block;
                EmitCaseStatement(inst, node->node[1], node->node[2]);
            }
            else
            {
                gScrCompileGlob[inst].breakBlock = node->node[2].block;
                hasDefault = 1;
                EmitDefaultStatement(inst, node->node[1]);
            }
            Scr_TransferBlock(block, gScrCompileGlob[inst].breakBlock);
            if (gScrCompileGlob[inst].bCanBreak
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    3923,
                    0,
                    "%s",
                    "!gScrCompileGlob[inst].bCanBreak"))
            {
                __debugbreak();
            }
            gScrCompileGlob[inst].bCanBreak = 1;
        }
        else
        {
            if (!gScrCompileGlob[inst].breakBlock)
            {
                CompileError(inst, endSourcePos, "missing case statement");
                return;
            }
            if (lastStatement && Scr_IsLastStatement(inst, nextNode))
                EmitStatement(inst, *node, 1, endSourcePos, gScrCompileGlob[inst].breakBlock);
            else
                EmitStatement(inst, *node, 0, endSourcePos, gScrCompileGlob[inst].breakBlock);
            if (gScrCompileGlob[inst].breakBlock && gScrCompileGlob[inst].breakBlock->abortLevel)
            {
                gScrCompileGlob[inst].breakBlock = 0;
                if (!gScrCompileGlob[inst].bCanBreak
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                        3940,
                        0,
                        "%s",
                        "gScrCompileGlob[inst].bCanBreak"))
                {
                    __debugbreak();
                }
                gScrCompileGlob[inst].bCanBreak = 0;
            }
        }
    }
    if (gScrCompileGlob[inst].breakBlock)
    {
        if (!gScrCompileGlob[inst].bCanBreak
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                3946,
                0,
                "%s",
                "gScrCompileGlob[inst].bCanBreak"))
        {
            __debugbreak();
        }
        gScrCompileGlob[inst].bCanBreak = 0;
        EmitRemoveLocalVars(inst, gScrCompileGlob[inst].breakBlock, gScrCompileGlob[inst].breakBlock);
    }
    if (hasDefault)
    {
        if (gScrCompileGlob[inst].breakBlock)
            Scr_AddBreakBlock(inst, gScrCompileGlob[inst].breakBlock);
        Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
    }
    gScrCompileGlob[inst].breakChildBlocks = oldBreakChildBlocks;
    gScrCompileGlob[inst].breakChildCount = oldBreakChildCount;
    gScrCompileGlob[inst].breakBlock = oldBreakBlock;
}

void __cdecl EmitCaseStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos)
{
    const char *v3; // eax
    unsigned int name; // [esp+4h] [ebp-4h]
    unsigned int namea; // [esp+4h] [ebp-4h]

    if ( *(_BYTE *)expr.node == 9 )
    {
        if ( IsValidArrayIndex(inst, expr.node[1].stringValue) )
        {
            name = GetInternalVariableIndex(inst, expr.node[1].stringValue);
            EmitCaseStatementInfo(inst, name, sourcePos);
        }
        else
        {
            v3 = va("case index %d out of range", expr.node[1].stringValue);
            CompileError(inst, sourcePos.stringValue, v3);
        }
    }
    else if ( *(_BYTE *)expr.node == 13 )
    {
        namea = expr.node[1].stringValue;
        CompileTransferRefToString(inst, namea, 1u);
        EmitCaseStatementInfo(inst, namea, sourcePos);
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "case expression must be an int or string");
    }
}

void __cdecl EmitDefaultStatement(scriptInstance_t inst, sval_u sourcePos)
{
    EmitCaseStatementInfo(inst, 0, sourcePos);
}

void __cdecl EmitCaseStatementInfo(scriptInstance_t inst, unsigned int name, sval_u sourcePos)
{
    CaseStatementInfo *newCaseStatement; // [esp+0h] [ebp-4h]

    if (gScrCompilePub[inst].developer_statement == 2)
    {
        if (gScrVarPub[inst].developer_script
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                4151,
                0,
                "%s",
                "!gScrVarPub[inst].developer_script"))
        {
            __debugbreak();
        }
    }
    else
    {
        newCaseStatement = (CaseStatementInfo *)Hunk_AllocateTempMemoryHigh(sizeof(CaseStatementInfo), "EmitCaseStatementInfo");
        newCaseStatement->name = name;
        newCaseStatement->codePos = TempMallocAlignStrict(0);
        newCaseStatement->sourcePos = sourcePos.stringValue;
        newCaseStatement->next = gScrCompileGlob[inst].currentCaseStatement;
        gScrCompileGlob[inst].currentCaseStatement = newCaseStatement;
    }
}

void __cdecl EmitBreakStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block)
{
    BreakStatementInfo *newBreakStatement; // [esp+0h] [ebp-4h]

    if (gScrCompileGlob[inst].bCanBreak && !block->abortLevel)
    {
        Scr_AddBreakBlock(inst, block);
        if (!gScrCompileGlob[inst].breakBlock
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                4176,
                0,
                "%s",
                "gScrCompileGlob[inst].breakBlock"))
        {
            __debugbreak();
        }
        EmitRemoveLocalVars(inst, block, gScrCompileGlob[inst].breakBlock);
        block->abortLevel = 2;
        EmitOpcode(inst, 0x62u, 0, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 1);
        EmitCodeInt(inst, 0);
        if (gScrCompilePub[inst].developer_statement != 2)
        {
            newBreakStatement = (BreakStatementInfo *)Hunk_AllocateTempMemoryHigh(sizeof(BreakStatementInfo), "EmitBreakStatement");
            newBreakStatement->codePos = (const char *)gScrCompileGlob[inst].codePos;
            newBreakStatement->nextCodePos = TempMallocAlignStrict(0);
            newBreakStatement->next = gScrCompileGlob[inst].currentBreakStatement;
            gScrCompileGlob[inst].currentBreakStatement = newBreakStatement;
        }
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "illegal break statement");
    }
}

void __cdecl EmitContinueStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block)
{
    ContinueStatementInfo *newContinueStatement; // [esp+0h] [ebp-4h]

    if (gScrCompileGlob[inst].bCanContinue && !block->abortLevel)
    {
        Scr_AddContinueBlock(inst, block);
        EmitRemoveLocalVars(inst, block, block);
        block->abortLevel = 1;
        EmitOpcode(inst, 0x62u, 0, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 1);
        EmitCodeInt(inst, 0);
        if (gScrCompilePub[inst].developer_statement != 2)
        {
            newContinueStatement = (ContinueStatementInfo *)Hunk_AllocateTempMemoryHigh(sizeof(ContinueStatementInfo), "EmitContinueStatement");
            newContinueStatement->codePos = (const char *)gScrCompileGlob[inst].codePos;
            newContinueStatement->nextCodePos = TempMallocAlignStrict(0);
            newContinueStatement->next = gScrCompileGlob[inst].currentContinueStatement;
            gScrCompileGlob[inst].currentContinueStatement = newContinueStatement;
        }
    }
    else
    {
        CompileError(inst, sourcePos.stringValue, "illegal continue statement");
    }
}

void __cdecl EmitBreakpointStatement(scriptInstance_t inst, sval_u sourcePos)
{
    if (gScrVarPub[inst].developer_script)
    {
        EmitOpcode(inst, 0x87u, 0, 0);
        AddOpcodePos(inst, sourcePos.stringValue, 1);
    }
}

void __cdecl EmitProfBeginStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos)
{
    EmitProfStatement(inst, profileName, sourcePos, 0x85u);
}

void __cdecl EmitProfStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos, unsigned __int8 op)
{
    if (gScrVarPub[inst].developer_script)
    {
        Scr_CompileRemoveRefToString(inst, profileName.stringValue);
        EmitOpcode(inst, op, 0, 0);
        EmitByte(inst, 0);
    }
    else
    {
        Scr_CompileRemoveRefToString(inst, profileName.stringValue);
    }
}

void __cdecl EmitProfEndStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos)
{
    EmitProfStatement(inst, profileName, sourcePos, 0x86u);
}

void __cdecl EmitDeveloperStatementList(
    scriptInstance_t inst,
    sval_u val,
    sval_u sourcePos,
    scr_block_s *block,
    sval_u *devStatBlock)
{
    char *savedPos; // [esp+0h] [ebp-8h]
    unsigned int savedChecksum; // [esp+4h] [ebp-4h]

    if (gScrCompilePub[inst].developer_statement)
    {
        CompileError(inst, sourcePos.stringValue, "cannot recurse /#");
    }
    else
    {
        savedChecksum = gScrVarPub[inst].checksum;
        Scr_TransferBlock(block, devStatBlock->block);
        if (gScrVarPub[inst].developer_script)
        {
            gScrCompilePub[inst].developer_statement = 1;
            EmitStatementList(inst, val, 0, 0, devStatBlock->block);
            EmitRemoveLocalVars(inst, devStatBlock->block, devStatBlock->block);
        }
        else
        {
            savedPos = TempMallocAlignStrict(0);
            gScrCompilePub[inst].developer_statement = 2;
            EmitStatementList(inst, val, 0, 0, devStatBlock->block);
            TempMemorySetPos(savedPos);
        }
        gScrCompilePub[inst].developer_statement = 0;
        gScrVarPub[inst].checksum = savedChecksum;
    }
}

void __cdecl EmitFormalParameterList(scriptInstance_t inst, sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
    EmitFormalParameterListInternal(inst, exprlist.node[0].node, block);
    EmitOpcode(inst, 0x35u, 0, 0);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
}

void __cdecl EmitFormalParameterListInternal(scriptInstance_t inst, sval_u *node, scr_block_s *block)
{
    while ( 1 )
    {
        node = node[1].node;
        if ( !node )
            break;
        EmitSafeSetVariableField(inst, *node->node, node->node[1], block);
    }
}

void __cdecl EmitSafeSetVariableField(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index; // [esp+0h] [ebp-4h]

    index = Scr_FindLocalVarIndex(inst, expr.stringValue, sourcePos, 1, block);
    EmitOpcode(inst, (index != 0) + 49, 0, 0);
    EmitPreAssignmentPos(inst);
    if ( index )
        EmitByte(inst, index);
    AddOpcodePos(inst, sourcePos.stringValue, 0);
    EmitAssignmentPos(inst);
}

void __cdecl InitThread(scriptInstance_t inst, int type)
{
    gScrCompileGlob[inst].currentCaseStatement = 0;
    gScrCompileGlob[inst].bCanBreak = 0;
    gScrCompileGlob[inst].currentBreakStatement = 0;
    gScrCompileGlob[inst].bCanContinue = 0;
    gScrCompileGlob[inst].currentContinueStatement = 0;
    gScrCompileGlob[inst].breakChildBlocks = 0;
    gScrCompileGlob[inst].continueChildBlocks = 0;
    if (gScrCompileGlob[inst].firstThread[type])
    {
        gScrCompileGlob[inst].firstThread[type] = 0;
        EmitEnd(inst);
        AddOpcodePos(inst, 0, 0);
        AddOpcodePos(inst, 0xFFFFFFFE, 0);
    }
}

void __cdecl EmitDeveloperThread(scriptInstance_t inst, sval_u val, sval_u *stmttblock)
{
    unsigned int posId; // [esp+0h] [ebp-Ch]
    unsigned int savedChecksum; // [esp+4h] [ebp-8h]
    char *begin_pos; // [esp+8h] [ebp-4h]

    if ( gScrCompilePub[inst].developer_statement
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    4663,
                    0,
                    "%s",
                    "gScrCompilePub[inst].developer_statement == SCR_DEV_NO") )
    {
        __debugbreak();
    }
    if ( gScrVarPub[inst].developer_script )
    {
        gScrCompilePub[inst].developer_statement = 1;
        InitThread(inst, 1);
        posId = FindVariable(inst, gScrCompileGlob[inst].filePosId, val.node[1].stringValue);
        SetThreadPosition(inst, posId);
        EmitThreadInternal(
            inst,
            val,
            val.node[4],
            val.node[5],
            stmttblock->block);
    }
    else
    {
        begin_pos = TempMallocAlignStrict(0);
        savedChecksum = gScrVarPub[inst].checksum;
        gScrCompilePub[inst].developer_statement = 2;
        InitThread(inst, 1);
        EmitThreadInternal(
            inst,
            val,
            val.node[4],
            val.node[5],
            stmttblock->block);
        TempMemorySetPos(begin_pos);
        gScrVarPub[inst].checksum = savedChecksum;
    }
    gScrCompilePub[inst].developer_statement = 0;
}

void __cdecl EmitIncludeList(scriptInstance_t inst, sval_u val)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    for ( node = val.node[0].node[1].node; node; node = node[1].node )
        EmitInclude(inst, *node);
}

void __cdecl EmitInclude(scriptInstance_t inst, sval_u val)
{
    char *v2; // eax
    unsigned int filename; // [esp+0h] [ebp-4h]

    if ( *(_BYTE *)val.node != 88
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_compiler.cpp",
                    4755,
                    0,
                    "%s",
                    "val.node[0].type == ENUM_include") )
    {
        __debugbreak();
    }
    v2 = SL_ConvertToString(val.node[1].stringValue, inst);
    filename = Scr_CreateCanonicalFilename(inst, v2);
    Scr_CompileRemoveRefToString(inst, val.node[1].stringValue);
    AddFilePrecache(inst, filename, val.node[2].stringValue, 1, 0, 0);
}

void __cdecl Scr_CompileStatement(scriptInstance_t inst, sval_u parseData)
{
    EmitStatement(inst, parseData, 0, 0, 0);
    EmitOpcode(inst, 0x80u, 0, 0);
}

