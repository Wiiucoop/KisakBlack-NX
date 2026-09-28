#include "cscr_parsetree.h"
#include <universal/assertive.h>
#include <universal/mem_userhunk.h>
#include "cscr_evaluate.h"
#include <universal/com_memory.h>
#include "cscr_vm.h"
#include "cscr_compiler.h"

HunkUser *g_allocNodeUser[2];

debugger_sval_s *g_debugExprHead = NULL;

sval_u node_pos(unsigned int pool)
{
    return (sval_u)pool;
}

sval_u __cdecl node1_(scriptInstance_t inst, int val1)
{
    return (sval_u)val1;
}

void __cdecl Scr_InitAllocNode(scriptInstance_t inst)
{
    if ( g_allocNodeUser[inst]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_parsetree.cpp",
                    79,
                    0,
                    "%s",
                    "!g_allocNodeUser[inst]") )
    {
        __debugbreak();
    }
    g_allocNodeUser[inst] = Hunk_UserCreate(0x10000, HU_SCHEME_DEFAULT, 0, 0, "Scr_InitAllocNode", 7);
}

void __cdecl Scr_ShutdownAllocNode(scriptInstance_t inst)
{
    if ( g_allocNodeUser[inst] )
    {
        Hunk_UserDestroy(g_allocNodeUser[inst]);
        g_allocNodeUser[inst] = 0;
    }
}

sval_u __cdecl node0(scriptInstance_t inst, ParseToken_t type)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 1);
    result.node[0].type = type;
    return result;
}

sval_u *__cdecl Scr_AllocNode(scriptInstance_t inst, int size)
{
    if ( !g_allocNodeUser[inst]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_parsetree.cpp",
                    95,
                    0,
                    "%s",
                    "g_allocNodeUser[inst]") )
    {
        __debugbreak();
    }
    return (sval_u *)Hunk_UserAlloc(g_allocNodeUser[inst], sizeof(sval_u) * size, alignof(sval_u), 0);   // nx-port: was 4 * size, align 4
}

sval_u __cdecl node1(scriptInstance_t inst, ParseToken_t type, sval_u val1)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 2);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    return result;
}

sval_u __cdecl node2(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 3);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    return result;
}

sval_u __cdecl node2_(scriptInstance_t inst, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 2);
    result.node[0] = val1;   // nx-port: whole values, pointer-sized
    result.node[1] = val2;
    return result;
}

sval_u __cdecl node3(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 4);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    return result;
}

sval_u __cdecl node4(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 5);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    return result;
}

sval_u __cdecl node5(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 6);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    return result;
}

sval_u __cdecl node6(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 7);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    return result;
}

sval_u __cdecl node7(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6,
                sval_u val7)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 8);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    result.node[7].node = val7.node;
    return result;
}

sval_u __cdecl node8(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6,
                sval_u val7,
                sval_u val8)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(inst, 9);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    result.node[7].node = val7.node;
    result.node[8].node = val8.node;
    return result;
}

sval_u __cdecl linked_list_end(scriptInstance_t inst, sval_u val)
{
    sval_u *node;
    sval_u result;

    node = Scr_AllocNode(inst, 2);
    node[0].node = val.node;
    node[1].stringValue = 0;
    result.node = Scr_AllocNode(inst, 2);
    result.node[0].node = node;
    result.node[1].node = node;
    return result;
}

sval_u __cdecl prepend_node(scriptInstance_t inst, sval_u val1, sval_u val2)
{
    sval_u *node;

    node = Scr_AllocNode(inst, 2);
    node[0] = val1;
    node[1] = *val2.node;
    val2.node->node = node;
    return val2;
}

sval_u __cdecl append_node(scriptInstance_t inst, sval_u val1, sval_u val2)
{
    sval_u *node;

    node = Scr_AllocNode(inst, 2);
    node[0] = val2;
    node[1].stringValue = 0;
    val1.node[1].node[1].node = node;
    val1.node[1].node = node;
    return val1;
}

void __cdecl Scr_ClearDebugExpr(scriptInstance_t inst, debugger_sval_s *debugExprHead)
{
    while ( debugExprHead )
    {
        //Scr_ClearDebugExprValue(inst, (sval_u)&debugExprHead[1]);
        sval_u *pval = (sval_u *)((char *)debugExprHead + sizeof(debugger_sval_s));
        Scr_ClearDebugExprValue(inst, *(sval_u *)&pval);
        debugExprHead = debugExprHead->next;
    }
}

sval_u *__cdecl Scr_AllocDebugExpr(scriptInstance_t inst, ParseToken_t type, int size, const char *name)
{
    sval_u *val; // eax
    debugger_sval_s *debugval;

    // prefix the malloc with a `debugger_sval_s`
    unsigned char *data = (unsigned char*)Z_Malloc(sizeof(debugger_sval_s) + size, name, 0);

    debugval = (debugger_sval_s *)data;
    val = (sval_u *)(data + sizeof(debugger_sval_s));

    // prepend the global list
    debugval->next = g_debugExprHead;
    g_debugExprHead = debugval;

    // set val type (convenience vs. the non-debug way) and return it
    val->type = type;
    return val;
}

void __cdecl Scr_FreeDebugExpr(scriptInstance_t inst, ScriptExpression_t *expr)
{
    debugger_sval_s *debugExprHead; // [esp+0h] [ebp-Ch]
    debugger_sval_s *nextDebugExprHead; // [esp+8h] [ebp-4h]

    if (expr->breakonExpr)
        --gScrVmDebugPub[inst].checkBreakon;

    debugExprHead = expr->exprHead;

    iassert(debugExprHead);

    while (debugExprHead)
    {
        // See Prefixed data in Scr_AllocDebugExpr()
        sval_u *pval = (sval_u *)((char *)debugExprHead + sizeof(debugger_sval_s));
        Scr_FreeDebugExprValue(inst, *(sval_u *)&pval);

        nextDebugExprHead = debugExprHead->next;
        Z_Free((char*)debugExprHead, 0);
        debugExprHead = nextDebugExprHead;
    }
}

sval_u __cdecl debugger_node0(scriptInstance_t inst, ParseToken_t type)
{
    sval_u result;
    result.node = Scr_AllocDebugExpr(inst, type, 4, "debugger_node0");
    return result;
}

sval_u __cdecl debugger_node1(scriptInstance_t inst, ParseToken_t type, sval_u val1)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(inst, type, 8, "debugger_node1");
    result.node[1].node = val1.node;

    return result;
}

sval_u __cdecl debugger_node2(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(inst, type, 12, "debugger_node2");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    return result;
}

sval_u __cdecl debugger_node3(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(inst, type, 16, "debugger_node3");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;

    return result;
}

sval_u __cdecl debugger_node4(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(inst, type, 20, "debugger_node4");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    return result;
}

sval_u __cdecl debugger_prepend_node(scriptInstance_t inst, sval_u val1, sval_u val2)
{
    //*(unsigned int *)val2.stringValue = *(unsigned int *)&debugger_node2(inst, 0, val1, *val2.node) + 4;
    *(_DWORD *)val2.type = debugger_node2(inst, ENUM_NOP, val1, (sval_u)val2.node->type).type + 4;
    return val2;
}

sval_u __cdecl debugger_buffer(
                scriptInstance_t inst,
                ParseToken_t type,
                char *buf,
                unsigned int size,
                int alignment)
{
    //sval_u result; // [esp+4h] [ebp-8h]
    //unsigned __int8 *bufCopy; // [esp+8h] [ebp-4h]
    //int alignmenta; // [esp+24h] [ebp+18h]
    //
    //if ( (alignment & (alignment - 1)) != 0
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_parsetree.cpp",
    //                611,
    //                0,
    //                "%s",
    //                "IsPowerOf2( alignment )") )
    //{
    //    __debugbreak();
    //}
    //alignmenta = alignment - 1;
    //result = Scr_AllocDebugExpr(inst, type, size + alignmenta + 2 * sizeof(sval_u), "debugger_buffer");
    //bufCopy = (unsigned __int8 *)(~alignmenta & (result.stringValue + alignmenta + 8));
    //memcpy(bufCopy, (unsigned __int8 *)buf, size);
    //*(unsigned int *)(result.stringValue + 4) = bufCopy;
    //return result;

    sval_u *result; // [esp+4h] [ebp-8h]
    unsigned __int8 *bufCopy; // [esp+8h] [ebp-4h]
    int alignmenta; // [esp+20h] [ebp+14h]

    //if ((alignment & (alignment - 1)) != 0)
    //    MyAssertHandler((char *)".\\script\\scr_parsetree.cpp", 594, 0, "%s", "IsPowerOf2( alignment )");
    alignmenta = alignment - 1;
    result = Scr_AllocDebugExpr(inst, type, size + alignmenta + 2 * sizeof(sval_u), "debugger_buffer");
    bufCopy = (unsigned __int8 *)(~(uintptr_t)alignmenta & ((uintptr_t)&result[2] + alignmenta));
    memcpy(bufCopy, (unsigned __int8 *)buf, size);
    result[1].codePosValue = (const char *)bufCopy;   // nx-port: a pointer
    return *result; // sus deref
}

sval_u __cdecl debugger_string(scriptInstance_t inst, ParseToken_t type, char *s)
{
    return debugger_buffer(inst, type, s, strlen(s) + 1, 1);
}

