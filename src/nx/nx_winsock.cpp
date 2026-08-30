// nx_winsock.cpp -- winsock semantics over libnx BSD sockets.
// Companion to src/nx/compat/winsock.h (which macro-renames the socket API
// to these nxws_* wrappers in game TUs; this file uses the native API).
#include <switch.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct WSAData {
    unsigned short wVersion;
    unsigned short wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA;

static int s_wsaLastError = 0;

static int nxwsMapErrno(int e)
{
    switch (e) {
    case 0:            return 0;
    case EINTR:        return 10004;
    case EBADF:        return 10009;
    case EACCES:       return 10013;
    case EFAULT:       return 10014;
    case EINVAL:       return 10022;
    case EMFILE:       return 10024;
    case EWOULDBLOCK:  return 10035;
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:       return 10035;
#endif
    case EINPROGRESS:  return 10036;
    case EALREADY:     return 10037;
    case ENOTSOCK:     return 10038;
    case EDESTADDRREQ: return 10039;
    case EMSGSIZE:     return 10040;
    case EPROTOTYPE:   return 10041;
    case ENOPROTOOPT:  return 10042;
    case EPROTONOSUPPORT: return 10043;
    case EOPNOTSUPP:   return 10045;
    case EAFNOSUPPORT: return 10047;
    case EADDRINUSE:   return 10048;
    case EADDRNOTAVAIL: return 10049;
    case ENETDOWN:     return 10050;
    case ENETUNREACH:  return 10051;
    case ENETRESET:    return 10052;
    case ECONNABORTED: return 10053;
    case ECONNRESET:   return 10054;
    case ENOBUFS:      return 10055;
    case EISCONN:      return 10056;
    case ENOTCONN:     return 10057;
    case ETIMEDOUT:    return 10060;
    case ECONNREFUSED: return 10061;
    case ELOOP:        return 10062;
    case ENAMETOOLONG: return 10063;
    case EHOSTUNREACH: return 10065;
    default:           return 10000 + e;
    }
}

static inline int nxwsFail(void)
{
    s_wsaLastError = nxwsMapErrno(errno);
    return -1;
}

extern "C" {

// Native socket calls are used directly by the game; report the current
// errno translated to the winsock code the game compares against (10035 etc).
int nxws_WSAGetLastError(void)
{
    int e = nxwsMapErrno(errno);
    return e ? e : s_wsaLastError;
}

int nxws_WSAStartup(unsigned short, WSADATA *data)
{
    if (data) memset(data, 0, sizeof(*data));
    return 0; // socketInitializeDefault() ran in nx_main
}

int nxws_WSACleanup(void) { return 0; }

int nxws_socket(int af, int type, int protocol)
{
    int r = socket(af, type, protocol);
    return r < 0 ? nxwsFail() : r;
}

int nxws_bind(int s, const struct sockaddr *name, int namelen)
{
    // game hardcodes namelen=16 and writes sa_family as u16 2; native
    // sockaddr_in wants sin_len/sin_family bytes -- rebuild cleanly.
    struct sockaddr_in na;
    memset(&na, 0, sizeof(na));
    na.sin_family = AF_INET;
    memcpy(&na.sin_port, name->sa_data, 2);
    memcpy(&na.sin_addr, name->sa_data + 2, 4);
    (void)namelen;
    int r = bind(s, (struct sockaddr *)&na, sizeof(na));
    return r < 0 ? nxwsFail() : r;
}

int nxws_connect(int s, const struct sockaddr *name, int namelen)
{
    struct sockaddr_in na;
    memset(&na, 0, sizeof(na));
    na.sin_family = AF_INET;
    memcpy(&na.sin_port, name->sa_data, 2);
    memcpy(&na.sin_addr, name->sa_data + 2, 4);
    (void)namelen;
    int r = connect(s, (struct sockaddr *)&na, sizeof(na));
    return r < 0 ? nxwsFail() : r;
}

int nxws_listen(int s, int backlog)
{
    int r = listen(s, backlog);
    return r < 0 ? nxwsFail() : r;
}

int nxws_accept(int s, struct sockaddr *addr, int *addrlen)
{
    struct sockaddr_in na;
    socklen_t nal = sizeof(na);
    int r = accept(s, addr ? (struct sockaddr *)&na : NULL, addr ? &nal : NULL);
    if (r < 0) return nxwsFail();
    if (addr) {
        memset(addr, 0, 16);
        addr->sa_family = AF_INET;
        memcpy(addr->sa_data, &na.sin_port, 2);
        memcpy(addr->sa_data + 2, &na.sin_addr, 4);
        if (addrlen) *addrlen = 16;
    }
    return r;
}

int nxws_send(int s, const char *buf, int len, int flags)
{
    int r = (int)send(s, buf, (size_t)len, flags);
    return r < 0 ? nxwsFail() : r;
}

int nxws_recv(int s, char *buf, int len, int flags)
{
    int r = (int)recv(s, buf, (size_t)len, flags);
    return r < 0 ? nxwsFail() : r;
}

int nxws_sendto(int s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
    int r;
    if (to) {
        struct sockaddr_in na;
        memset(&na, 0, sizeof(na));
        na.sin_family = AF_INET;
        memcpy(&na.sin_port, to->sa_data, 2);
        memcpy(&na.sin_addr, to->sa_data + 2, 4);
        (void)tolen;
        r = (int)sendto(s, buf, (size_t)len, flags, (struct sockaddr *)&na, sizeof(na));
    } else {
        r = (int)send(s, buf, (size_t)len, flags);
    }
    return r < 0 ? nxwsFail() : r;
}

int nxws_recvfrom(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    int r;
    if (from) {
        struct sockaddr_in na;
        socklen_t nal = sizeof(na);
        r = (int)recvfrom(s, buf, (size_t)len, flags, (struct sockaddr *)&na, &nal);
        if (r >= 0) {
            memset(from, 0, 16);
            from->sa_family = AF_INET;
            memcpy(from->sa_data, &na.sin_port, 2);
            memcpy(from->sa_data + 2, &na.sin_addr, 4);
            if (fromlen) *fromlen = 16;
        }
    } else {
        r = (int)recv(s, buf, (size_t)len, flags);
    }
    return r < 0 ? nxwsFail() : r;
}

int nxws_shutdown(int s, int how)
{
    int r = shutdown(s, how);
    return r < 0 ? nxwsFail() : r;
}

int nxws_closesocket(int s)
{
    int r = close(s);
    return r < 0 ? nxwsFail() : r;
}

int nxws_ioctlsocket(int s, long cmd, void *argp)
{
    if ((unsigned int)cmd == 0x8004667eu) { // winsock FIONBIO
        int enable = argp && *(unsigned int *)argp != 0;
        int flags = fcntl(s, F_GETFL, 0);
        if (flags < 0) return nxwsFail();
        flags = enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        if (fcntl(s, F_SETFL, flags) < 0) return nxwsFail();
        return 0;
    }
    s_wsaLastError = 10022; // WSAEINVAL
    return -1;
}

static int nxwsXlatOpt(int level, int optname, int *nlevel, int *nopt)
{
    if (level == 0xFFFF) { // winsock SOL_SOCKET
        *nlevel = SOL_SOCKET;
        switch (optname) {
        case 0x0020: *nopt = SO_BROADCAST; return 0;
        case 0x0004: *nopt = SO_REUSEADDR; return 0;
        case 0x1001: *nopt = SO_SNDBUF; return 0;
        case 0x1002: *nopt = SO_RCVBUF; return 0;
        default: return -1;
        }
    }
    *nlevel = level;
    *nopt = optname;
    return 0;
}

int nxws_setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    int nlevel, nopt;
    if (nxwsXlatOpt(level, optname, &nlevel, &nopt) != 0)
        return 0; // ignore unknown winsock options
    int r = setsockopt(s, nlevel, nopt, optval, (socklen_t)optlen);
    return r < 0 ? nxwsFail() : r;
}

int nxws_getsockopt(int s, int level, int optname, void *optval, int *optlen)
{
    int nlevel, nopt;
    if (nxwsXlatOpt(level, optname, &nlevel, &nopt) != 0)
        return -1;
    socklen_t l = optlen ? (socklen_t)*optlen : 0;
    int r = getsockopt(s, nlevel, nopt, optval, &l);
    if (r < 0) return nxwsFail();
    if (optlen) *optlen = (int)l;
    return 0;
}

int nxws_fdisset(int s, fd_set *set)
{
    return (s >= 0 && s < FD_SETSIZE) ? FD_ISSET(s, set) : 0;
}

// winsock select ignores nfds (win_net passes 0); compute it here.
int nxws_select(int, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int maxFd = -1;
    for (int fd = 0; fd < FD_SETSIZE; ++fd) {
        if ((readfds && FD_ISSET(fd, readfds)) ||
            (writefds && FD_ISSET(fd, writefds)) ||
            (exceptfds && FD_ISSET(fd, exceptfds)))
            maxFd = fd;
    }
    int r = select(maxFd + 1, readfds, writefds, exceptfds, timeout);
    return r < 0 ? nxwsFail() : r;
}

struct hostent *nxws_gethostbyname(const char *name)
{
    struct hostent *he = gethostbyname(name);
    if (!he)
        s_wsaLastError = 11001; // WSAHOST_NOT_FOUND
    return he;
}

int nxws_gethostname(char *name, int namelen)
{
    int r = gethostname(name, (size_t)namelen);
    if (r != 0) {
        strncpy(name, "switch", (size_t)namelen);
        if (namelen > 0) name[namelen - 1] = 0;
    }
    return 0;
}

} // extern "C"
