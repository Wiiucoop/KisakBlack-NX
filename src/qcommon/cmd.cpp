#include "cmd.h"

#include <stdlib.h>
#include <win32/win_common.h>
#include <clientscript/cscr_debugger.h>
#include <clientscript/cscr_main.h>
#include "common.h"
#include <cstring>
#include <game_mp/g_cmds_mp.h>
#include <server_mp/sv_main_mp.h>

#include <Windows.h>
#include "threads.h"
#include <stringed/stringed_hooks.h>
#include <client/cl_keys.h>
#include <universal/com_expressions_eval.h>
#include <universal/com_files.h>
#include "dvar_cmds.h"
#include <universal/com_shared.h>
#include <client/cl_console.h>
#include <database/db_registry.h>
#include <database/db_file_load.h>
#include <client_mp/cl_main_mp.h>
#include <server/sv_game.h>

int cmd_wait;
CmdText cmd_textArray[1];
unsigned __int8 cmd_text_buf[1][65536];
bool cmd_insideCBufExecute[1];
CmdText sv_cmd_text;
unsigned __int8 sv_cmd_text_buf[65536];
cmd_function_s *sv_cmd_functions;
CmdArgs sv_cmd_args;
int com_inServerFrame;
cmd_function_s *cmd_functions;
CmdArgs g_cmd_args[2];

void __cdecl Cmd_Wait_f()
{
    const char *v0; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        cmd_wait = atoi(v0);
    }
    else
    {
        cmd_wait = 1;
    }
}

void __cdecl Cbuf_Init()
{
    int client; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    for ( client = 0; client < 1; ++client )
    {
        cmd_textArray[client].data = cmd_text_buf[client];
        cmd_textArray[client].maxsize = 0x10000;
        cmd_textArray[client].cmdsize = 0;
        cmd_insideCBufExecute[client] = 0;
    }
    sv_cmd_text.data = sv_cmd_text_buf;
    sv_cmd_text.maxsize = 0x10000;
    sv_cmd_text.cmdsize = 0;
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl Cbuf_AddText(int localClientNum, const char *text)
{
    CmdText *cmd_text; // [esp+0h] [ebp-8h]
    int length; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    if ( (*text == 112 || *text == 80) && text[1] == 48 )
    {
        localClientNum = text[1] - 48;
        for ( text += 2; *text == 32; ++text )
            ;
    }
    cmd_text = &cmd_textArray[localClientNum];
    //length = strlen_noncrt(text);
    length = strlen(text);
    if ( cmd_text->cmdsize + length < cmd_text->maxsize )
    {
        //memcpy_noncrt(&cmd_text->data[cmd_text->cmdsize], text, length + 1);
        memcpy(&cmd_text->data[cmd_text->cmdsize], text, length + 1);
        cmd_text->cmdsize += length;
        Scr_MonitorCommand(text, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Com_Printf(16, "Cbuf_AddText: overflow\n");
    }
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl Cbuf_InsertText(int localClientNum, const char *text)
{
    unsigned int v2; // [esp+4h] [ebp-1Ch]
    CmdText *cmd_text; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int length; // [esp+1Ch] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    284,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_CBUF);
    cmd_text = &cmd_textArray[localClientNum];
    v2 = strlen(text);
    length = v2 + 1;
    if ( (signed int)(cmd_text->cmdsize + v2 + 1) <= cmd_text->maxsize )
    {
        for ( i = cmd_text->cmdsize - 1; i >= 0; --i )
            cmd_text->data[length + i] = cmd_text->data[i];
        memcpy(cmd_text->data, (unsigned __int8 *)text, v2);
        cmd_text->data[v2] = 10;
        cmd_text->cmdsize += length;
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
    }
    else
    {
        Com_PrintError(1, "Cbuf_InsertText overflowed\n");
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
    }
}

void __cdecl Cbuf_AddServerText_f()
{
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    330,
                    0,
                    "Cbuf_AddServerText_f was called.") )
        __debugbreak();
}

void __cdecl Cmd_ExecuteServerString(char *text)
{
    const char *v1; // eax
    const char *arg0; // [esp+0h] [ebp-10h]
    const char *predicate; // [esp+4h] [ebp-Ch]
    cmd_function_s **prev; // [esp+8h] [ebp-8h]
    cmd_function_s *cmd; // [esp+Ch] [ebp-4h]

    //if ( !PbTrapPreExecCmd(text) )
    {
        SV_Cmd_TokenizeString(text);
        if ( SV_Cmd_Argc() )
        {
            v1 = SV_Cmd_Argv(0);
            if ( I_stricmp(v1, "if") || SV_Cmd_Argc() <= 2 )
            {
LABEL_7:
                arg0 = SV_Cmd_Argv(0);
                for ( prev = &sv_cmd_functions; *prev; prev = &cmd->next )
                {
                    cmd = *prev;
                    if ( !I_stricmp(arg0, (*prev)->name) )
                    {
                        *prev = cmd->next;
                        cmd->next = sv_cmd_functions;
                        sv_cmd_functions = cmd;
                        if ( cmd->function )
                            cmd->function();
                        goto LABEL_14;
                    }
                }
                goto LABEL_14;
            }
            predicate = SV_Cmd_Argv(1);
            if ( atoi(predicate) )
            {
                Cmd_ShiftArgs(2, &sv_cmd_args);
                goto LABEL_7;
            }
        }
LABEL_14:
        SV_Cmd_EndTokenizedString();
    }
}

void __cdecl Cmd_ShiftArgs(int count, CmdArgs *args)
{
    int v2; // [esp+4h] [ebp-10h]
    int dest_arg; // [esp+Ch] [ebp-8h]
    int src_arg; // [esp+10h] [ebp-4h]

    if ( args->argc[args->nesting] < count )
        v2 = args->argc[args->nesting];
    else
        v2 = count;
    src_arg = v2;
    for ( dest_arg = 0; dest_arg < args->argc[args->nesting]; ++dest_arg )
    {
        if ( src_arg < args->argc[args->nesting] )
            args->argv[args->nesting][dest_arg] = args->argv[args->nesting][src_arg];
        else
            args->argv[args->nesting][dest_arg] = 0;
        ++src_arg;
    }
    args->argshift[args->nesting] += v2;
    args->argc[args->nesting] -= v2;
}

void __cdecl Cbuf_SV_Execute()
{
    char v0; // [esp+0h] [ebp-1010h]
    int count; // [esp+4h] [ebp-100Ch]
    unsigned int counta; // [esp+4h] [ebp-100Ch]
    char dst[4096]; // [esp+8h] [ebp-1008h] BYREF
    unsigned __int8 *src; // [esp+100Ch] [ebp-4h]

    while ( sv_cmd_text.cmdsize )
    {
        src = sv_cmd_text.data;
        v0 = 0;
        for ( count = 0; count < sv_cmd_text.cmdsize; ++count )
        {
            if ( src[count] == 34 )
                ++v0;
            if ( (v0 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13 )
                break;
        }
        if ( count >= 4095 )
            count = 4095;
        memcpy((unsigned __int8 *)dst, src, count);
        dst[count] = 0;
        if ( count == sv_cmd_text.cmdsize )
        {
            sv_cmd_text.cmdsize = 0;
        }
        else
        {
            counta = count + 1;
            sv_cmd_text.cmdsize -= counta;
            memmove(src, &src[counta], sv_cmd_text.cmdsize);
        }
        SV_WaitServer();
        if ( com_inServerFrame )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 454, 0, "%s", "!com_inServerFrame") )
                __debugbreak();
        }
        Cmd_ExecuteServerString(dst);
    }
}

void __cdecl Cmd_AddServerCommandInternal(const char *cmdName, void (__cdecl *function)(), cmd_function_s *allocedCmd)
{
    cmd_function_s *cmd; // [esp+14h] [ebp-4h]

    if ( !cmdName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 471, 0, "%s", "cmdName") )
        __debugbreak();
    for ( cmd = sv_cmd_functions; ; cmd = cmd->next )
    {
        if ( !cmd )
        {
            allocedCmd->name = cmdName;
            allocedCmd->function = function;
            allocedCmd->next = sv_cmd_functions;
            sv_cmd_functions = allocedCmd;
            return;
        }
        if ( !strcmp(cmdName, cmd->name) )
            break;
    }
    if ( cmd != allocedCmd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 478, 0, "%s", "cmd == allocedCmd") )
    {
        __debugbreak();
    }
    if ( function )
        Com_Printf(16, "Cmd_AddServerCommand: %s already defined\n", cmdName);
}

void __cdecl Cbuf_ExecuteBuffer_Restricted(int localClientNum, int controllerIndex, char *buffer)
{
    Cbuf_ExecuteBufferInternal(localClientNum, controllerIndex, 0, buffer, 0, 1);
}

void __cdecl Cbuf_ExecuteBufferInternal(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char *buffer,
                char ***dvarWhitelistArray,
                bool restrict)
{
    char v6; // [esp+10h] [ebp-1018h]
    signed int v7; // [esp+14h] [ebp-1014h]
    char dst[4100]; // [esp+18h] [ebp-1010h] BYREF
    unsigned int count; // [esp+1020h] [ebp-8h]
    unsigned __int8 *src; // [esp+1024h] [ebp-4h]

    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 571, 0, "%s", "buffer") )
        __debugbreak();
    src = (unsigned __int8 *)buffer;
    v7 = strlen(buffer);
    while ( v7 )
    {
        v6 = 0;
        for ( count = 0; (int)count < v7; ++count )
        {
            if ( src[count] == 34 )
                ++v6;
            if ( (v6 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13 )
                break;
        }
        if ( (int)count >= 4095 )
            count = 4095;
        memcpy((unsigned __int8 *)dst, src, count);
        dst[count] = 0;
        if ( count != v7 )
            ++count;
        src += count;
        v7 -= count;
        if ( dvarWhitelistArray )
            Cmd_ExecSafeDvarSingleCommmandOnly(localClientNum, controllerIndex, dst, (const char ***)dvarWhitelistArray);
        else
            Cmd_ExecuteSingleCommandInternal(localClientNum, controllerIndex, item, dst, restrict);
    }
}

void __cdecl Cmd_ExecSafeDvarSingleCommmandOnly(
                int localClientNum,
                int controllerIndex,
                char *singleCommand,
                const char ***dvarWhitelistArray)
{
    int j; // [esp+0h] [ebp-3010h]
    int i; // [esp+4h] [ebp-300Ch]
    char s0[4096]; // [esp+8h] [ebp-3008h] BYREF
    char s1[4096]; // [esp+1008h] [ebp-2008h] BYREF
    char v8; // [esp+2008h] [ebp-1008h] BYREF

    if ( !dvarWhitelistArray
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 497, 0, "%s", "dvarWhitelistArray") )
    {
        __debugbreak();
    }
    if ( dvarWhitelistArray )
    {
        v8 = 0;
        s1[0] = 0;
        s0[0] = 0;
        sscanf(singleCommand, "%s %s %s", s0, s1, &v8);
        if ( s0[0] && s1[0] && v8 && !I_stricmp(s0, "set") )
        {
            for ( i = 0; dvarWhitelistArray[i]; ++i )
            {
                for ( j = 0; dvarWhitelistArray[i][j]; ++j )
                {
                    if ( !I_stricmp(dvarWhitelistArray[i][j], s1) )
                    {
                        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, singleCommand);
                        return;
                    }
                }
            }
            Com_Printf(16, "did not set dvar %s not in white-list\n", s1);
        }
        else
        {
            Com_Printf(16, "did not execute command %s from - not a Dvar set\n", s0);
        }
    }
}

void __cdecl Cbuf_ExecuteBuffer(int localClientNum, int controllerIndex, char *buffer)
{
    Cbuf_ExecuteBufferInternal(localClientNum, controllerIndex, 0, buffer, 0, 0);
}

void __cdecl Cbuf_ExecuteBufferUI(int localClientNum, int controllerIndex, itemDef_s *item, char *buffer)
{
    Cbuf_ExecuteBufferInternal(localClientNum, controllerIndex, item, buffer, 0, 0);
}

void __cdecl Cbuf_Execute(int localClientNum, int controllerIndex)
{
    PROF_SCOPED("Cbuf_Execute");

    if ( cmd_insideCBufExecute[localClientNum]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    717,
                    0,
                    "%s\n\t%s",
                    "!cmd_insideCBufExecute[localClientNum]",
                    "Nesting Cbuf_Execute() is not allowed.") )
    {
        __debugbreak();
    }
    cmd_insideCBufExecute[localClientNum] = 1;
    Cbuf_ExecuteInternal(localClientNum, controllerIndex);
    cmd_insideCBufExecute[localClientNum] = 0;
    Cbuf_SV_Execute();
}

void __cdecl Cbuf_ExecuteInternal(int localClientNum, int controllerIndex)
{
    char v2; // [esp+0h] [ebp-1014h]
    CmdText *v3; // [esp+4h] [ebp-1010h]
    int count; // [esp+8h] [ebp-100Ch]
    unsigned int counta; // [esp+8h] [ebp-100Ch]
    char dst[4096]; // [esp+Ch] [ebp-1008h] BYREF
    unsigned __int8 *src; // [esp+1010h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CBUF);
    v3 = &cmd_textArray[localClientNum];
    while ( v3->cmdsize )
    {
        if ( cmd_wait )
        {
            --cmd_wait;
            break;
        }
        src = v3->data;
        v2 = 0;
        for ( count = 0; count < v3->cmdsize; ++count )
        {
            if ( src[count] == 34 )
                ++v2;
            if ( (v2 & 1) == 0 && src[count] == 59 || src[count] == 10 || src[count] == 13 )
                break;
        }
        if ( count >= 4095 )
            count = 4095;
        memcpy((unsigned __int8 *)dst, src, count);
        dst[count] = 0;
        if ( count == v3->cmdsize )
        {
            v3->cmdsize = 0;
        }
        else
        {
            counta = count + 1;
            v3->cmdsize -= counta;
            memmove(src, &src[counta], v3->cmdsize);
        }
        Sys_LeaveCriticalSection(CRITSECT_CBUF);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, dst);
        Sys_EnterCriticalSection(CRITSECT_CBUF);
    }
    Sys_LeaveCriticalSection(CRITSECT_CBUF);
}

void __cdecl Cmd_Vstr_f()
{
    char *v0; // eax
    const char *dvarName; // [esp+10h] [ebp-8h]
    const dvar_s *dvar; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        dvarName = Cmd_Argv(1);
        dvar = Dvar_FindVar(dvarName);
        if ( dvar )
        {
            if ( dvar->type == DVAR_TYPE_STRING || dvar->type == DVAR_TYPE_ENUM )
            {
                v0 = va("%s\n", dvar->current.string);
                Cbuf_InsertText(0, v0);
            }
            else
            {
                Com_Printf(0, "%s is not a string-based dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "%s doesn't exist\n", dvarName);
        }
    }
    else
    {
        Com_Printf(0, "vstr <variablename> : execute a variable command\n");
    }
}

void __cdecl SV_Cmd_ArgvBuffer(int arg, char *buffer, int bufferLength)
{
    const char *v3; // eax

    v3 = SV_Cmd_Argv(arg);
    I_strncpyz(buffer, v3, bufferLength);
}

void __cdecl Cmd_ArgsBuffer(int start, char *buffer, int bufLength)
{
    CmdArgs *cmd_args; // [esp+4h] [ebp-18h]
    const char *src; // [esp+8h] [ebp-14h]
    int argIndex; // [esp+Ch] [ebp-10h]
    const char **argv; // [esp+10h] [ebp-Ch]
    char *dst; // [esp+14h] [ebp-8h]
    int argc; // [esp+18h] [ebp-4h]
    int bufLengtha; // [esp+2Ch] [ebp+10h]

    cmd_args = Cmd_Args();
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 999, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( start < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1000, 0, "%s", "start >= 0") )
    {
        __debugbreak();
    }
    if ( cmd_args->nesting >= 8u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1001,
                    0,
                    "cmd_args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                    cmd_args->nesting,
                    8) )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1002, 0, "%s", "buffer") )
        __debugbreak();
    if ( bufLength < 2
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1003, 0, "%s", "bufLength >= 2") )
    {
        __debugbreak();
    }
    dst = buffer;
    bufLengtha = bufLength - 1;
    argc = cmd_args->argc[cmd_args->nesting];
    argv = cmd_args->argv[cmd_args->nesting];
    for ( argIndex = start; argIndex < argc; ++argIndex )
    {
        for ( src = argv[argIndex]; *src; ++src )
        {
            *dst++ = *src;
            if ( !--bufLengtha )
            {
                *dst = 0;
                return;
            }
        }
        if ( bufLengtha == 1 )
        {
            *dst = 0;
            return;
        }
        if ( argIndex != argc - 1 )
        {
            *dst++ = 32;
            --bufLengtha;
        }
    }
    *dst = 0;
}

void __cdecl Cmd_TokenizeStringWithLimit(const char *text_in, int max_tokens)
{
    CmdArgs *v2; // eax

    v2 = Cmd_Args();
    Cmd_TokenizeStringKernel(-1, 0, 0, text_in, max_tokens, 1, v2);
}

void __cdecl Cmd_TokenizeStringKernel(
                int localClientNum,
                int localControllerIndex,
                itemDef_s *item,
                const char *text_in,
                int max_tokens,
                bool evalExpressions,
                CmdArgs *args)
{
    if ( max_tokens > 512 - args->totalUsedArgvPool
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1286,
                    0,
                    "max_tokens <= MAX_TOKENS_SIZE - args->totalUsedArgvPool\n\t%i, %i",
                    max_tokens,
                    512 - args->totalUsedArgvPool) )
    {
        __debugbreak();
    }
    AssertCmdArgsConsistency(args);
    if ( ++args->nesting >= 8u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1291,
                    0,
                    "args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                    args->nesting,
                    8) )
    {
        __debugbreak();
    }
    args->usedTextPool[args->nesting] = -args->totalUsedTextPool;
    args->itemDef[args->nesting] = item;
    args->localClientNum[args->nesting] = localClientNum;
    args->controllerIndex[args->nesting] = localControllerIndex;
    args->argv[args->nesting] = &args->argvPool[args->totalUsedArgvPool];
    args->argshift[args->nesting] = 0;
    args->argc[args->nesting] = Cmd_TokenizeStringInternal(
                                                                text_in,
                                                                max_tokens,
                                                                evalExpressions,
                                                                args->argv[args->nesting],
                                                                args);
    args->totalUsedArgvPool += args->argc[args->nesting];
    args->usedTextPool[args->nesting] += args->totalUsedTextPool;
    AssertCmdArgsConsistency(args);
}

int __cdecl Cmd_TokenizeStringInternal(
                const char *text_in,
                int max_tokens,
                bool evalExpressions,
                const char **argv,
                CmdArgs *argsPriv)
{
    int v6; // [esp+0h] [ebp-80h]
    int v7; // [esp+4h] [ebp-7Ch]
    int v8; // [esp+8h] [ebp-78h]
    int v9; // [esp+Ch] [ebp-74h]
    int v10; // [esp+10h] [ebp-70h]
    int v11; // [esp+14h] [ebp-6Ch]
    unsigned int locCharCount; // [esp+3Ch] [ebp-44h]
    const char *translatedString; // [esp+40h] [ebp-40h]
    char locToken[48]; // [esp+44h] [ebp-3Ch] BYREF
    const char *text; // [esp+78h] [ebp-8h] BYREF
    int argc; // [esp+7Ch] [ebp-4h]

    if ( !text_in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1119, 0, "%s", "text_in") )
        __debugbreak();
    argc = 0;
    text = text_in;
    while ( 1 )
    {
        while ( 1 )
        {
            while ( 1 )
            {
                if ( !*text )
                    return argc;
                if ( !Cmd_IsWhiteSpaceChar(*text) )
                    break;
                ++text;
            }
            if ( *text == 47 && text[1] == 47 )
                return argc;
            if ( *text != 47 || text[1] != 42 )
                break;
            for ( text += 2; *text && (*text != 42 || text[1] != 47); ++text )
                ;
            if ( !*text )
                return argc;
            text += 2;
        }
        argv[argc++] = &argsPriv->textPool[argsPriv->totalUsedTextPool];
        if ( !--max_tokens )
            break;
        if ( evalExpressions && *text == 40 )
        {
            Cmd_EvaluateExpression(&text, argsPriv);
            if ( !text || !*text )
                return argc;
        }
        else if ( evalExpressions && *text == 64 )
        {
            for ( locCharCount = 0; locCharCount < 0x30 && *text && !Cmd_IsWhiteSpaceChar(*text); ++locCharCount )
                locToken[locCharCount] = *text++;
            locToken[locCharCount] = 0;
            translatedString = SEH_StringEd_GetString(&locToken[1]);
            if ( translatedString )
                Cmd_EmitString(translatedString, argsPriv);
            else
                Cmd_EmitString(locToken, argsPriv);
            if ( !*text )
                return argc;
        }
        else if ( *text == 34 )
        {
            ++text;
            while ( *text && *text != 34 )
            {
                if ( *text == 92 && text[1] == 34 )
                    ++text;
                argsPriv->textPool[argsPriv->totalUsedTextPool] = *text;
                if ( argsPriv->totalUsedTextPool + 1 < 8190 )
                    v10 = argsPriv->totalUsedTextPool + 1;
                else
                    v10 = 8190;
                argsPriv->totalUsedTextPool = v10;
                ++text;
            }
            argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
            if ( argsPriv->totalUsedTextPool + 1 < 8190 )
                v9 = argsPriv->totalUsedTextPool + 1;
            else
                v9 = 8190;
            argsPriv->totalUsedTextPool = v9;
            if ( !*text )
                return argc;
            if ( !*++text )
                return argc;
        }
        else
        {
            do
            {
                argsPriv->textPool[argsPriv->totalUsedTextPool] = *text;
                if ( argsPriv->totalUsedTextPool + 1 < 8190 )
                    v8 = argsPriv->totalUsedTextPool + 1;
                else
                    v8 = 8190;
                argsPriv->totalUsedTextPool = v8;
                if ( !*++text )
                {
                    argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
                    if ( argsPriv->totalUsedTextPool + 1 < 8190 )
                        v7 = argsPriv->totalUsedTextPool + 1;
                    else
                        v7 = 8190;
                    argsPriv->totalUsedTextPool = v7;
                    return argc;
                }
            }
            while ( !Cmd_IsWhiteSpaceChar(*text) && (*text != 47 || text[1] != 47 && text[1] != 42) );
            argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
            if ( argsPriv->totalUsedTextPool + 1 < 8190 )
                v6 = argsPriv->totalUsedTextPool + 1;
            else
                v6 = 8190;
            argsPriv->totalUsedTextPool = v6;
        }
    }
    while ( 1 )
    {
        argsPriv->textPool[argsPriv->totalUsedTextPool] = *text;
        v11 = argsPriv->totalUsedTextPool + 1 < 8190 ? argsPriv->totalUsedTextPool + 1 : 8190;
        argsPriv->totalUsedTextPool = v11;
        if ( !*text )
            break;
        ++text;
    }
    return argc;
}

bool __cdecl Cmd_IsWhiteSpaceChar(unsigned __int8 letter)
{
    if ( !letter
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1041, 1, "%s", "letter != '\\0'") )
    {
        __debugbreak();
    }
    if ( letter == 20 || letter == 21 || letter == 22 )
        return 0;
    if ( letter <= 0x20u )
        return !Key_IsValidGamePadChar(letter);
    return 0;
}

void __cdecl Cmd_EmitString(const char *str, CmdArgs *argsPriv)
{
    while ( *str && argsPriv->totalUsedTextPool < 0x1FFFu )
        argsPriv->textPool[argsPriv->totalUsedTextPool++] = *str++;
    argsPriv->textPool[argsPriv->totalUsedTextPool++] = 0;
}

void __cdecl Cmd_EvaluateExpression(const char **text_in, CmdArgs *argsPriv)
{
    itemDef_s *v2; // eax
    int v3; // [esp+0h] [ebp-C24h]
    int v4; // [esp+4h] [ebp-C20h]
    // nx-port: twice the x86 3072 -- every entry Expression_Parse carves out
    // of it doubles in size on LP64.
    char compileBuffer[6144]; // [esp+1Ch] [ebp-C08h] BYREF
    const char *result; // [esp+C20h] [ebp-4h]

    if ( !text_in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1084, 0, "%s", "text_in") )
        __debugbreak();
    if ( !*text_in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1085, 0, "%s", "*text_in") )
        __debugbreak();
    v2 = Cmd_ItemDef();
    for ( result = GetExpressionResultStringCompile(0, v2, text_in, compileBuffer, sizeof(compileBuffer)); *result; ++result )
    {
        argsPriv->textPool[argsPriv->totalUsedTextPool] = *result;
        if ( argsPriv->totalUsedTextPool + 1 < 8190 )
            v4 = argsPriv->totalUsedTextPool + 1;
        else
            v4 = 8190;
        argsPriv->totalUsedTextPool = v4;
    }
    argsPriv->textPool[argsPriv->totalUsedTextPool] = 0;
    if ( argsPriv->totalUsedTextPool + 1 < 8190 )
        v3 = argsPriv->totalUsedTextPool + 1;
    else
        v3 = 8190;
    argsPriv->totalUsedTextPool = v3;
}

itemDef_s *__cdecl Cmd_ItemDef()
{
    CmdArgs *cmd_args; // [esp+4h] [ebp-4h]

    cmd_args = Cmd_Args();
    if ( cmd_args->nesting >= 8u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.h",
                    264,
                    0,
                    "cmd_args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                    cmd_args->nesting,
                    8) )
    {
        __debugbreak();
    }
    return cmd_args->itemDef[cmd_args->nesting];
}

void __cdecl AssertCmdArgsConsistency(const CmdArgs *args)
{
    const char *v1; // eax
    int totalUsedTextPool; // [esp+0h] [ebp-10h]
    int totalUsedArgvPool; // [esp+4h] [ebp-Ch]
    int arg; // [esp+8h] [ebp-8h]
    int nesting; // [esp+Ch] [ebp-4h]
    int nestinga; // [esp+Ch] [ebp-4h]

    totalUsedArgvPool = 0;
    totalUsedTextPool = 0;
    for ( nesting = 0; nesting <= args->nesting; ++nesting )
    {
        totalUsedArgvPool += args->argshift[nesting] + args->argc[nesting];
        totalUsedTextPool += args->usedTextPool[nesting];
    }
    if ( totalUsedArgvPool != args->totalUsedArgvPool )
    {
        Com_Printf(16, "About to assert totalUsedArgvPool\n");
        for ( nestinga = 0; nestinga <= args->nesting; ++nestinga )
        {
            for ( arg = 0; arg < args->argc[nestinga]; ++arg )
                Com_Printf(16, "nesting %i, arg %i: '%s'\n", nestinga, arg, args->argv[nestinga][arg]);
        }
    }
    if ( totalUsedArgvPool != args->totalUsedArgvPool )
    {
        v1 = va(
                     "totalUsedArgvPool is %i, args->totalUsedArgvPool is %i, args->nesting is %i, args->argc[0] is %i, args->argc["
                     "1] is %i, args->argc[2] is %i, args->argc[3] is %i",
                     totalUsedArgvPool,
                     args->totalUsedArgvPool,
                     args->nesting,
                     args->argc[0],
                     args->argc[1],
                     args->argc[2],
                     args->argc[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                        1276,
                        0,
                        "%s\n\t%s",
                        "totalUsedArgvPool == args->totalUsedArgvPool",
                        v1) )
            __debugbreak();
    }
    if ( totalUsedTextPool != args->totalUsedTextPool
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1277,
                    0,
                    "totalUsedTextPool == args->totalUsedTextPool\n\t%i, %i",
                    totalUsedTextPool,
                    args->totalUsedTextPool) )
    {
        __debugbreak();
    }
}

void __cdecl Cmd_TokenizeStringWithLimitNoEval(const char *text_in, int max_tokens)
{
    CmdArgs *v2; // eax

    v2 = Cmd_Args();
    Cmd_TokenizeStringKernel(-1, 0, 0, text_in, max_tokens, 0, v2);
}

void __cdecl Cmd_TokenizeString(const char *text_in)
{
    CmdArgs *v1; // eax

    v1 = Cmd_Args();
    Cmd_TokenizeStringWithLimit(text_in, 512 - v1->totalUsedArgvPool);
}

void __cdecl Cmd_TokenizeStringNoEval(const char *text_in)
{
    CmdArgs *v1; // eax

    v1 = Cmd_Args();
    Cmd_TokenizeStringWithLimitNoEval(text_in, 512 - v1->totalUsedArgvPool);
}

void __cdecl Cmd_EndTokenizedString()
{
    CmdArgs *v0; // eax

    v0 = Cmd_Args();
    Cmd_EndTokenizedStringKernel(v0);
}

void __cdecl Cmd_EndTokenizedStringKernel(CmdArgs *args)
{
    AssertCmdArgsConsistency(args);
    if ( args->nesting != -1 )
    {
        if ( args->nesting >= 8u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                        1316,
                        0,
                        "args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                        args->nesting,
                        8) )
        {
            __debugbreak();
        }
        args->totalUsedArgvPool -= args->argc[args->nesting];
        args->totalUsedArgvPool -= args->argshift[args->nesting];
        args->totalUsedTextPool -= args->usedTextPool[args->nesting--];
        AssertCmdArgsConsistency(args);
    }
}

void __cdecl SV_Cmd_TokenizeString(const char *text_in)
{
    Cmd_TokenizeStringKernel(-1, 0, 0, text_in, 512 - sv_cmd_args.totalUsedArgvPool, 1, &sv_cmd_args);
}

void __cdecl SV_Cmd_EndTokenizedString()
{
    Cmd_EndTokenizedStringKernel(&sv_cmd_args);
}

cmd_function_s *__cdecl Cmd_FindCommand(const char *cmdName)
{
    cmd_function_s *cmd; // [esp+14h] [ebp-4h]

    for ( cmd = cmd_functions; cmd; cmd = cmd->next )
    {
        if ( !strcmp(cmdName, cmd->name) )
            return cmd;
    }
    return 0;
}

void __cdecl Cmd_AddCommandInternal(const char *cmdName, void (__cdecl *function)(), cmd_function_s *allocedCmd)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    if ( !cmdName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1403, 0, "%s", "cmdName") )
        __debugbreak();
    cmd = Cmd_FindCommand(cmdName);
    if ( cmd )
    {
        if ( cmd != allocedCmd
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1408, 0, "%s", "cmd == allocedCmd") )
        {
            __debugbreak();
        }
        if ( function )
            Com_Printf(16, "Cmd_AddCommand: %s already defined\n", cmdName);
    }
    else
    {
        allocedCmd->name = cmdName;
        allocedCmd->function = function;
        allocedCmd->next = cmd_functions;
        allocedCmd->consoleAccess = 0;
        cmd_functions = allocedCmd;
    }
}

void __cdecl Cmd_RemoveCommand(const char *cmdName)
{
    cmd_function_s **back; // [esp+14h] [ebp-8h]
    cmd_function_s *cmd; // [esp+18h] [ebp-4h]

    for ( back = &cmd_functions; ; back = (cmd_function_s **)*back )
    {
        cmd = *back;
        if ( !*back )
            break;
        if ( !strcmp(cmdName, cmd->name) )
        {
            *back = cmd->next;
            return;
        }
    }
}

void __cdecl Cmd_SetAutoComplete(const char *cmdName, const char *dir, const char *ext)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    if ( !cmdName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1452, 0, "%s", "cmdName") )
        __debugbreak();
    if ( !dir && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1453, 0, "%s", "dir") )
        __debugbreak();
    if ( !ext && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1454, 0, "%s", "ext") )
        __debugbreak();
    cmd = Cmd_FindCommand(cmdName);
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1458, 0, "%s", "cmd") )
        __debugbreak();
    if ( cmd->autoCompleteDir
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1459,
                    0,
                    "%s",
                    "cmd->autoCompleteDir == NULL") )
    {
        __debugbreak();
    }
    if ( cmd->autoCompleteExt
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1460,
                    0,
                    "%s",
                    "cmd->autoCompleteExt == NULL") )
    {
        __debugbreak();
    }
    cmd->autoCompleteDir = dir;
    cmd->autoCompleteExt = ext;
}

void __cdecl Cmd_Shutdown()
{
    cmd_functions = 0;
    sv_cmd_functions = 0;
}

void __cdecl Cmd_ForEach(void (__cdecl *callback)(const char *))
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    for ( cmd = cmd_functions; cmd; cmd = cmd->next )
        callback(cmd->name);
}

void __cdecl Cmd_ResetConAccessCommands()
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    for ( cmd = cmd_functions; cmd; cmd = cmd->next )
        cmd->consoleAccess = 0;
}

char __cdecl Cmd_SetConAccess(const char *str)
{
    cmd_function_s **prev; // [esp+0h] [ebp-8h]
    cmd_function_s *cmd; // [esp+4h] [ebp-4h]

    for ( prev = &cmd_functions; *prev; prev = &cmd->next )
    {
        cmd = *prev;
        if ( !I_stricmp(str, (*prev)->name) )
        {
            cmd->consoleAccess = 1;
            return 1;
        }
    }
    return 0;
}

void __cdecl Cmd_ForEachConsoleAccessName(void (__cdecl *callback)(const char *))
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    if ( Dvar_GetBool("con_restricted_access") )
    {
        for ( cmd = cmd_functions; cmd; cmd = cmd->next )
        {
            if ( cmd->consoleAccess )
                callback(cmd->name);
        }
    }
    else
    {
        Cmd_ForEach(callback);
    }
}

const char **__cdecl Cmd_GetAutoCompleteFileList(const char *cmdName, int *fileCount)
{
    cmd_function_s *cmd; // [esp+0h] [ebp-4h]

    if ( !cmdName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1541, 0, "%s", "cmdName") )
        __debugbreak();
    if ( !fileCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1542, 0, "%s", "fileCount") )
    {
        __debugbreak();
    }
    *fileCount = 0;
    cmd = Cmd_FindCommand(cmdName);
    if ( cmd && cmd->autoCompleteDir && cmd->autoCompleteExt )
        return FS_ListFiles(cmd->autoCompleteDir, (char *)cmd->autoCompleteExt, FS_LIST_PURE_ONLY, fileCount);
    else
        return 0;
}

void __cdecl Cmd_ComErrorCleanup()
{
    CmdArgs *v0; // eax
    int client; // [esp+4h] [ebp-4h]

    v0 = Cmd_Args();
    Cmd_ResetArgs(v0);
    Cmd_ResetArgs(&sv_cmd_args);
    for ( client = 0; client < 1; ++client )
        cmd_insideCBufExecute[client] = 0;
}

void __cdecl Cmd_ResetArgs(CmdArgs *args)
{
    args->nesting = -1;
    args->totalUsedArgvPool = 0;
    args->totalUsedTextPool = 0;
}

void __cdecl Cmd_ExecuteSingleCommandInternal(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char *text,
                bool restrict)
{
    CmdArgs *v5; // eax
    const char *v6; // eax
    CmdArgs *v7; // eax
    char *v8; // eax
    int v9; // eax
    CmdArgs *v10; // [esp-4h] [ebp-50h]
    char *arg0; // [esp+38h] [ebp-14h]
    const char *predicate; // [esp+3Ch] [ebp-10h]
    bool restricted; // [esp+43h] [ebp-9h]
    cmd_function_s **prev; // [esp+44h] [ebp-8h]
    cmd_function_s *cmd; // [esp+48h] [ebp-4h]

    if ( (localClientNum < -1 || localClientNum >= 1)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp",
                    1592,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= (-1) && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    //if ( !PbTrapPreExecCmd(text) )
    {
        restricted = 0;
        if ( !I_strnicmp(text, "#dcr#", 5) )
        {
            restricted = 1;
            text += 5;
        }
        v10 = Cmd_Args();
        v5 = Cmd_Args();
        Cmd_TokenizeStringKernel(localClientNum, controllerIndex, item, text, 512 - v5->totalUsedArgvPool, 1, v10);
        if ( !Cmd_Argc() )
            goto LABEL_42;
        v6 = Cmd_Argv(0);
        if ( !I_stricmp(v6, "if") && Cmd_Argc() > 2 )
        {
            predicate = Cmd_Argv(1);
            if ( !atoi(predicate) )
                goto LABEL_42;
            v7 = Cmd_Args();
            Cmd_ShiftArgs(2, v7);
        }
        arg0 = (char *)Cmd_Argv(0);
        v8 = strchr(arg0, 0x20u);
        if ( v8 )
        {
            Com_PrintError(1, "Command arg0 contains whitespace (\"%s\"), most likely an error: %s\n", arg0, text);
            Cmd_EndTokenizedString();
            return;
        }
        for ( prev = &cmd_functions; ; prev = &cmd->next )
        {
            if ( !*prev )
                goto LABEL_31;
            cmd = *prev;
            if ( !I_stricmp(arg0, (*prev)->name) )
                break;
        }
        if ( (restricted || restrict) && !cmd->consoleAccess )
        {
            Com_DPrintf(16, "ConAccess: cmd not executed: %s.\n", cmd->name);
            goto LABEL_31;
        }
        *prev = cmd->next;
        cmd->next = cmd_functions;
        cmd_functions = cmd;
        if ( !cmd->function )
        {
LABEL_31:
            if ( restricted || restrict )
                v9 = Dvar_Command(1);
            else
                v9 = Dvar_Command(0);
            if ( !v9 && (!com_sv_running || !com_sv_running->current.enabled || !SV_GameCommand()) && localClientNum > -1 )
                CL_ForwardCommandToServer(localClientNum, text);
            goto LABEL_42;
        }
        if ( cmd->function == Cbuf_AddServerText_f )
        {
            SV_WaitServer();
            if ( com_inServerFrame
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cmd.cpp", 1679, 0, "%s", "!com_inServerFrame") )
            {
                __debugbreak();
            }
            Cmd_ExecuteServerString(text);
        }
        else
        {
            cmd->function();
        }
LABEL_42:
        Cmd_EndTokenizedString();
    }
}

void __cdecl Cmd_ExecuteSingleCommand(int localClientNum, int controllerIndex, char *text)
{
    Cmd_ExecuteSingleCommandInternal(localClientNum, controllerIndex, 0, text, 0);
}

void __cdecl Cmd_List_f()
{
    const char *match; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    cmd_function_s *cmd; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() <= 1 )
        match = 0;
    else
        match = Cmd_Argv(1);
    i = 0;
    for ( cmd = cmd_functions; cmd; cmd = cmd->next )
    {
        if ( !match || Com_Filter(match, (char *)cmd->name, 0) )
        {
            Com_Printf(0, "%s\n", cmd->name);
            ++i;
        }
    }
    Com_Printf(0, "%i commands\n", i);
}

cmd_function_s Cmd_List_f_VAR;
cmd_function_s Cmd_Exec_f_VAR;
cmd_function_s Cmd_ExecAddText_f_VAR;
cmd_function_s Cmd_Vstr_f_VAR;
cmd_function_s Cmd_Wait_f_VAR;

void __cdecl Cmd_Init()
{
    Cmd_ResetArgs(g_cmd_args);
    Cmd_ResetArgs(&g_cmd_args[1]);
    Cmd_ResetArgs(&sv_cmd_args);
    Cmd_AddCommandInternal("cmdlist", Cmd_List_f, &Cmd_List_f_VAR);
    Cmd_AddCommandInternal("exec", Cmd_Exec_f, &Cmd_Exec_f_VAR);
    Cmd_AddCommandInternal("exec_addtext", Cmd_ExecAddText_f, &Cmd_ExecAddText_f_VAR);
    Cmd_AddCommandInternal("vstr", Cmd_Vstr_f, &Cmd_Vstr_f_VAR);
    Cmd_AddCommandInternal("wait", Cmd_Wait_f, &Cmd_Wait_f_VAR);
}

void __cdecl Cmd_Exec_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *pathname; // [esp+Ch] [ebp-ACh]
    char filename[132]; // [esp+28h] [ebp-90h] BYREF
    bool bAddText; // [esp+B3h] [ebp-5h]
    int localClientNum; // [esp+B4h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        bAddText = 0;
        v0 = Cmd_Argv(0);
        I_strncpyz(filename, v0, 128);
        if ( !I_strcmp(filename, "exec_addtext") )
            bAddText = 1;
        v1 = Cmd_Argv(1);
        I_strncpyz(filename, v1, 128);
        Com_DefaultExtension(filename, 0x80u, ".cfg");
        localClientNum = 0;
        pathname = (char *)Com_GetFilenameSubString(filename);
        if ( I_stricmp(pathname, "config_mp.cfg") )
        {
            if ( (!useFastFile->current.enabled || !Cmd_ExecFromFastFile(localClientNum, 0, filename))
                && (!bAddText || !Cmd_ExecFromDiskAddText(localClientNum, 0, filename))
                && !Cmd_ExecFromDisk(localClientNum, 0, filename) )
            {
                v2 = Cmd_Argv(1);
                Com_PrintError(1, "couldn't exec %s\n", v2);
            }
        }
        else
        {
            Cmd_ExecFromDisk(localClientNum, 0, filename);
        }
    }
    else
    {
        Com_Printf(0, "exec <filename> : execute a script file\n");
    }
}

char __cdecl Cmd_ExecFromDisk(int localClientNum, int controllerIndex, const char *filename)
{
    char *text; // [esp+0h] [ebp-4h] BYREF

    FS_ReadFile(filename, (void **)&text);
    if ( text )
    {
        Com_Printf(16, "execing %s from disk\n", filename);
        Con_Restricted_ExecuteBuf(localClientNum, controllerIndex, text);
        FS_FreeFile(text);
        return 1;
    }
    else
    {
        Com_Printf(16, "Exec From Disk failed to open %s\n", filename);
        return 0;
    }
}

char __cdecl Cmd_ExecFromDiskAddText(int localClientNum, int controllerIndex, const char *filename)
{
    char *text; // [esp+0h] [ebp-4h] BYREF

    FS_ReadFile(filename, (void **)&text);
    if ( text )
    {
        Com_Printf(16, "execing %s from disk to command buffer\n", filename);
        Cbuf_AddText(localClientNum, text);
        FS_FreeFile(text);
        return 1;
    }
    else
    {
        Com_Printf(16, "Exec From Disk failed to open %s\n", filename);
        return 0;
    }
}

char __cdecl Cmd_ExecFromFastFile(int localClientNum, int controllerIndex, char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    if ( !DB_IsMinimumFastFileLoaded() )
        return 0;
    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename, 1, -1).rawfile;
    if ( !rawfile )
        return 0;
    Com_Printf(16, "execing %s from fastfile\n", filename);
    Cbuf_ExecuteBuffer(localClientNum, controllerIndex, (char *)rawfile->buffer);
    return 1;
}

void __cdecl Cmd_ExecAddText_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char filename[128]; // [esp+18h] [ebp-88h] BYREF
    int localClientNum; // [esp+9Ch] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        I_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 0x80u, ".cfg");
        localClientNum = 0;
        if ( !Cmd_ExecFromDiskAddText(0, 0, filename) )
        {
            v1 = Cmd_Argv(1);
            Com_PrintError(1, "couldn't exec %s\n", v1);
        }
    }
    else
    {
        Com_Printf(0, "exec_addtext <filename> : execute a script file, appending the contents to the command buffer\n");
    }
}

const char *__cdecl Cmd_Argv(int argIndex)
{
    CmdArgs *cmd_args; // [esp+8h] [ebp-4h]

    cmd_args = Cmd_Args();
    if (cmd_args->nesting >= 8u
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../qcommon/cmd.h",
            228,
            0,
            "cmd_args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args->nesting,
            8))
    {
        __debugbreak();
    }
    if (argIndex < 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../qcommon/cmd.h",
            229,
            0,
            "%s\n\t(argIndex) = %i",
            "(argIndex >= 0)",
            argIndex))
    {
        __debugbreak();
    }
    if (argIndex >= cmd_args->argc[cmd_args->nesting])
        return "";
    else
        return cmd_args->argv[cmd_args->nesting][argIndex];
}

CmdArgs *__cdecl Cmd_Args()
{
    CmdArgs *cmd_args; // [esp+0h] [ebp-4h]

    cmd_args = (CmdArgs *)Sys_GetValue(4);
    if (!cmd_args
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../qcommon/cmd.h", 203, 0, "%s", "cmd_args != NULL"))
    {
        __debugbreak();
    }
    return cmd_args;
}

int __cdecl Cmd_Argc()
{
    CmdArgs *cmd_args; // [esp+4h] [ebp-4h]

    cmd_args = Cmd_Args();
    if (cmd_args->nesting >= 8u
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../qcommon/cmd.h",
            212,
            0,
            "cmd_args->nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args->nesting,
            8))
    {
        __debugbreak();
    }
    return cmd_args->argc[cmd_args->nesting];
}
