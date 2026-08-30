#include "dl_main.h"
#include "common.h"
#include <string.h>

// LWSS: Downloader func's stubbed again

int __cdecl DL_VPrintf(const char *fmt, char *argptr)
{
    char msg[1028]; // [esp+10h] [ebp-408h] BYREF

#ifdef KISAK_NX
    strncpy(msg, fmt, 0x400u); // downloader is stubbed; argptr is a fake va_list
    msg[0x400] = 0;
    (void)argptr;
#else
    _vsnprintf(msg, 0x400u, fmt, argptr);
#endif
    Com_Printf(0, "%s", msg);
    return &msg[strlen(msg) + 1] - &msg[1];
}

int __cdecl terminate_handler_WINDOWSNAMINGCLASH(_HTRequest *request, struct _HTResponse *response, void *param, int status)
{
#if 0
    terminate_status = status;
    HTEventList_stopLoop();
#endif
    return 0;
}

int __cdecl HTAlertCallback_progress(_HTRequest *request, _HTAlertOpcode op)
{
#if 0
    if ( op == HT_PROG_READ )
    {
        if ( dl_is_ftp )
        {
            if ( !HTNet_rawBytesCount((int)request->net) )
            {
                Com_DPrintf(0, "Force raw byte count on request->net %p\n", request->net);
                HTFTP_setRawBytesCount((int)request);
            }
            legacyHacks.cl_downloadCount = HTFTP_getDNetRawBytesCount((int)request);
        }
        else
        {
            legacyHacks.cl_downloadCount = HTRequest_bytesRead((int)request);
        }
    }
#endif
    return 1;
}

int __cdecl HTAlertCallback_confirm(_HTRequest *request, _HTAlertOpcode op, int msgnum)
{
#if 0
    if ( msgnum == 9 )
    {
        Com_Printf(0, "Replace existing download target file\n");
        return 1;
    }
    else
    {
        Com_Printf(0, "Aborting, unknown libwww confirm message id: %d\n", msgnum);
        HTEventList_stopLoop();
        return 0;
    }
#else
    return 0;
#endif
}

int __cdecl HTAlertCallback_prompt(_HTRequest *request, _HTAlertOpcode op, int msgnum)
{
    //Com_Printf(0, "Aborting, libwww prompt message id: %d (prompted for a login/password?)\n", msgnum);
    //HTEventList_stopLoop();
    return 0;
}

void __cdecl DL_CancelDownload()
{
#if 0
    if ( dl_running )
    {
        HTEventList_unregisterAll();
        HTHost_setEventTimeout(-1);
        dl_running = 0;
    }
    if ( dl_request )
    {
        HTRequest_kill((int)dl_request);
        HTRequest_delete(dl_request);
        dl_request = 0;
    }
#endif
}

void __cdecl DL_InitDownload()
{
#if 0
    if ( !dl_initialized )
    {
        HTProfile_newNoCacheClient("ID_DOWNLOAD", "1.0");
        HTAlertInit();
        HTAlert_setInteractive(1);
        HTPrint_setCallback((int)DL_VPrintf);
        HTTrace_setCallback((int)DL_VPrintf);
        HTNet_addAfter(terminate_handler_WINDOWSNAMINGCLASH, 0, 0, (void *)1, 0xFFFF);
        HTAlert_add((int)HTAlertCallback_progress, 0xFFFF);
        HTAlert_add((int)HTAlertCallback_confirm, 0x20000);
        HTAlert_add((int)HTAlertCallback_prompt, 1835008);
        Com_Printf(0, "Client download subsystem initialized\n");
        dl_initialized = 1;
    }
#endif
}

int __cdecl DL_BeginDownload(char *localName, char *remoteName)
{
#if 0
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *url; // [esp+20h] [ebp-1Ch] BYREF
    char *path; // [esp+24h] [ebp-18h]
    char *login; // [esp+28h] [ebp-14h]
    char *access; // [esp+2Ch] [ebp-10h]
    char *ptr; // [esp+30h] [ebp-Ch]
    char *passwd; // [esp+34h] [ebp-8h]
    _HTBasic *basic; // [esp+38h] [ebp-4h]

    access = 0;
    url = 0;
    login = 0;
    path = 0;
    ptr = 0;
    if ( dl_running )
    {
        if ( !dl_isMotd
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dl_main.cpp", 253, 0, "%s", "dl_isMotd") )
        {
            __debugbreak();
        }
        DL_CancelDownload();
    }
    terminate_status = -1000;
    if ( localName && remoteName )
    {
        DL_InitDownload();
        access = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 16);
        if ( !_stricmp(access, "ftp") )
        {
            dl_is_ftp = 1;
            HTHost_setEventTimeout(-1);
        }
        else
        {
            dl_is_ftp = 0;
            HTHost_setEventTimeout(30000);
        }
        dl_request = (_HTRequest *)HTRequest_new();
        if ( !_stricmp(access, "http")
            && (login = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 8),
                    path = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 5),
                    strchr((unsigned __int8 *)login, 0x40u),
                    (ptr = v3) != 0) )
        {
            *ptr = 0;
            strchr((unsigned __int8 *)login, 0x3Au);
            passwd = v4;
            if ( v4 )
            {
                *passwd++ = 0;
                HTUnEscape(passwd);
            }
            HTUnEscape(login);
            basic = (_HTBasic *)HTBasic_new();
            HTSACopy((void **)&basic->uid, (unsigned __int8 *)login);
            HTSACopy((void **)&basic->pw, (unsigned __int8 *)passwd);
            basic_credentials((int)dl_request, (int)basic);
            HTBasic_delete((void **)&basic->uid);
            url = (char *)HTMemory_malloc(strlen(ptr + 1) + strlen(path) + 8);
            sprintf(url, "http://%s%s", ptr + 1, path);
            Com_DPrintf(0, "HTTP Basic Auth - %s %s %s\n", login, passwd, url);
            HTMemory_free(login);
            login = 0;
            HTMemory_free(path);
            path = 0;
        }
        else
        {
            HTSACopy((void **)&url, (unsigned __int8 *)remoteName);
        }
        HTMemory_free(access);
        access = 0;
        FS_CreatePath(localName);
        if ( HTLoadToFile((unsigned __int8 *)url, (int)dl_request, localName) == 1 )
        {
            HTMemory_free(url);
            url = 0;
            access = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 16);
            login = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 8);
            path = (char *)HTParse((unsigned __int8 *)remoteName, (unsigned __int8 *)"", 5);
            strchr((unsigned __int8 *)login, 0x40u);
            ptr = v5;
            if ( v5 )
                Com_sprintf(legacyHacks.cl_downloadName, 0x40u, "%s://*:*%s%s", access, ptr, path);
            else
                Com_sprintf(legacyHacks.cl_downloadName, 0x40u, remoteName);
            HTMemory_free(path);
            path = 0;
            HTMemory_free(login);
            login = 0;
            HTMemory_free(access);
            access = 0;
            if ( dl_is_ftp )
                HTHost_setEventTimeout(30000);
            HTEventList_init();
            dl_running = 1;
            return 1;
        }
        else
        {
            Com_DPrintf(0, "HTLoadToFile failed\n");
            HTMemory_free(url);
            url = 0;
            HT//Profile_delete();
            return 0;
        }
    }
    else
    {
        Com_DPrintf(0, "Empty download URL or empty local file name\n");
        return 0;
    }
#endif
    return 0;
}

int __cdecl DL_DownloadLoop()
{
#if 0
    if ( !dl_running
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dl_main.cpp", 451, 0, "%s", "dl_running") )
    {
        __debugbreak();
    }
    if ( HTEventList_pump() )
        return 0;
    HTEventList_unregisterAll();
    HTHost_setEventTimeout(-1);
    HTRequest_kill((int)dl_request);
    HTRequest_delete(dl_request);
    dl_request = 0;
    dl_running = 0;
    if ( terminate_status >= 0 )
        return 1;
    Com_Printf(0, "DL_DownloadLoop: request terminated with failure status %d\n", terminate_status);
    return 2;
#endif
    return 0;
}

bool __cdecl DL_InProgress()
{
#if 0
    return dl_running > 0;
#endif
    return false;
}

bool __cdecl DL_DLIsMotd()
{
#if 0
    return dl_isMotd;
#endif
    return false;
}

