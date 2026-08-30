// winsock.h -- Winsock compatibility over the native libnx BSD sockets.
//
// Strategy: use the NATIVE socket types (sockaddr/sockaddr_in/in_addr/
// hostent/fd_set/timeval from newlib+libnx) — their field names line up with
// what the game pokes — and shim only the genuinely winsock-specific bits:
//   - WSAStartup/WSAGetLastError (errno -> 10xxx codes; the game compares
//     raw numbers like 10035)
//   - closesocket / ioctlsocket (hardcoded winsock FIONBIO value)
//   - setsockopt/getsockopt (winsock numeric SOL_SOCKET=0xFFFF, SO_* values)
//   - select (win_net passes nfds=0, winsock-legal but BSD-broken)
// The few sites that poke winsock fd_set internals (fd_count/fd_array) or
// use in_addr's S_un union spelling are patched under KISAK_NX.
#ifndef NX_COMPAT_WINSOCK_H
#define NX_COMPAT_WINSOCK_H

// real winsock.h pulls in windows.h; monkey_comm.cpp relies on that
#include "windows.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>

typedef int SOCKET;
#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR   (-1)

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffffu
#endif

#define SD_RECEIVE 0 // == SHUT_RD
#define SD_SEND    1 // == SHUT_WR
#define SD_BOTH    2 // == SHUT_RDWR

// --- winsock error codes (the game compares raw decimals) ------------------
#define WSABASEERR          10000
#define WSAEINTR            10004
#define WSAEBADF            10009
#define WSAEACCES           10013
#define WSAEFAULT           10014
#define WSAEINVAL           10022
#define WSAEMFILE           10024
#define WSAEWOULDBLOCK      10035
#define WSAEINPROGRESS      10036
#define WSAEALREADY         10037
#define WSAENOTSOCK         10038
#define WSAEDESTADDRREQ     10039
#define WSAEMSGSIZE         10040
#define WSAEPROTOTYPE       10041
#define WSAENOPROTOOPT      10042
#define WSAEPROTONOSUPPORT  10043
#define WSAESOCKTNOSUPPORT  10044
#define WSAEOPNOTSUPP       10045
#define WSAEPFNOSUPPORT     10046
#define WSAEAFNOSUPPORT     10047
#define WSAEADDRINUSE       10048
#define WSAEADDRNOTAVAIL    10049
#define WSAENETDOWN         10050
#define WSAENETUNREACH      10051
#define WSAENETRESET        10052
#define WSAECONNABORTED     10053
#define WSAECONNRESET       10054
#define WSAENOBUFS          10055
#define WSAEISCONN          10056
#define WSAENOTCONN         10057
#define WSAESHUTDOWN        10058
#define WSAETOOMANYREFS     10059
#define WSAETIMEDOUT        10060
#define WSAECONNREFUSED     10061
#define WSAELOOP            10062
#define WSAENAMETOOLONG     10063
#define WSAEHOSTDOWN        10064
#define WSAEHOSTUNREACH     10065
#define WSASYSNOTREADY      10091
#define WSAVERNOTSUPPORTED  10092
#define WSANOTINITIALISED   10093
#define WSAEDISCON          10101
#define WSAHOST_NOT_FOUND   11001
#define WSATRY_AGAIN        11002
#define WSANO_RECOVERY      11003
#define WSANO_DATA          11004

typedef struct WSAData {
    unsigned short wVersion;
    unsigned short wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA, *LPWSADATA;

#ifdef __cplusplus
extern "C" {
#endif

// WSAGetLastError maps the CURRENT errno to a winsock code -- the native
// socket calls are used directly (their sockaddr field names/offsets match
// what the game pokes), so no send/recv/bind wrappers are needed. Renaming
// those with macros would also break struct members named `send` etc.
int nxws_WSAStartup(unsigned short version, WSADATA *data);
int nxws_WSACleanup(void);
int nxws_WSAGetLastError(void);
int nxws_closesocket(SOCKET s);
int nxws_ioctlsocket(SOCKET s, long cmd, void *argp);
int nxws_setsockopt(SOCKET s, int level, int optname, const void *optval, int optlen);
int nxws_getsockopt(SOCKET s, int level, int optname, void *optval, int *optlen);
int nxws_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int nxws_fdisset(SOCKET s, fd_set *set);

#ifdef __cplusplus
}
#endif

#define WSAStartup      nxws_WSAStartup
#define WSACleanup      nxws_WSACleanup
#define WSAGetLastError nxws_WSAGetLastError
#define closesocket     nxws_closesocket
#define ioctlsocket(s, cmd, argp) nxws_ioctlsocket((s), (long)(cmd), (void *)(argp))
#define __WSAFDIsSet    nxws_fdisset
#define select          nxws_select
#define setsockopt(s, l, o, v, n) nxws_setsockopt((s), (l), (o), (const void *)(v), (n))
#define getsockopt(s, l, o, v, n) nxws_getsockopt((s), (l), (o), (void *)(v), (n))

// winsock's u_long is 32-bit; only used as the ioctlsocket arg in this
// codebase, and nxws_ioctlsocket reads just 32 bits through the void*.

#endif // NX_COMPAT_WINSOCK_H
