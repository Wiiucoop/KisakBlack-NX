// steam_api.h -- header-only fake Steam SDK for the Switch build (shadows
// src/steam/* via include-path priority).
//
// Behavior: pretends a logged-on Steam user with a fixed SteamID so the
// game's online plumbing stays alive; auth always succeeds; callbacks never
// fire (SteamAPI_RunCallbacks is a no-op), P2P/friends/stats are empty.
#ifndef NX_COMPAT_STEAM_API_H
#define NX_COMPAT_STEAM_API_H

#include <stdint.h>
#include <string.h>

// ----- steam base types (match q_shared.h typedefs where they overlap) ------
#ifndef NX_STEAM_BASE_TYPES
#define NX_STEAM_BASE_TYPES
typedef unsigned char      uint8;
typedef signed char        int8;
typedef short              int16;
typedef unsigned short     uint16;
typedef int                int32;
typedef unsigned int       uint32;
typedef long long          int64;
typedef unsigned long long uint64;
typedef int64 lint64;
typedef uint64 ulint64;
typedef intptr_t  intp;
typedef uintptr_t uintp;
#endif

typedef uint32 HAuthTicket;
const HAuthTicket k_HAuthTicketInvalid = 0;
typedef uint64 SteamAPICall_t;
const SteamAPICall_t k_uAPICallInvalid = 0;
typedef uint32 AppId_t;
const AppId_t k_uAppIdInvalid = 0;
typedef uint64 CGameID;

#define S_CALLTYPE
#define STEAM_API

// ----- enums -----------------------------------------------------------------
enum EResult {
    k_EResultNone = 0,
    k_EResultOK = 1,
    k_EResultFail = 2,
    k_EResultNoConnection = 3,
    k_EResultInvalidPassword = 5,
    k_EResultLoggedInElsewhere = 6,
    k_EResultInvalidProtocolVer = 7,
    k_EResultInvalidParam = 8,
    k_EResultFileNotFound = 9,
    k_EResultBusy = 10,
    k_EResultInvalidState = 11,
    k_EResultInvalidName = 12,
    k_EResultInvalidEmail = 13,
    k_EResultDuplicateName = 14,
    k_EResultAccessDenied = 15,
    k_EResultTimeout = 16,
    k_EResultBanned = 17,
    k_EResultAccountNotFound = 18,
    k_EResultInvalidSteamID = 19,
    k_EResultServiceUnavailable = 20,
    k_EResultNotLoggedOn = 21,
    k_EResultPending = 22,
    k_EResultEncryptionFailure = 23,
    k_EResultInsufficientPrivilege = 24,
    k_EResultLimitExceeded = 25,
    k_EResultRevoked = 26,
    k_EResultExpired = 27,
    k_EResultAlreadyRedeemed = 28,
    k_EResultDuplicateRequest = 29,
};

enum EDenyReason {
    k_EDenyInvalid = 0,
    k_EDenyInvalidVersion = 1,
    k_EDenyGeneric = 2,
    k_EDenyNotLoggedOn = 3,
    k_EDenyNoLicense = 4,
    k_EDenyCheater = 5,
    k_EDenyLoggedInElseWhere = 6,
    k_EDenyUnknownText = 7,
    k_EDenyIncompatibleAnticheat = 8,
    k_EDenyMemoryCorruption = 9,
    k_EDenyIncompatibleSoftware = 10,
    k_EDenySteamConnectionLost = 11,
    k_EDenySteamConnectionError = 12,
    k_EDenySteamResponseTimedOut = 13,
    k_EDenySteamValidationStalled = 14,
    k_EDenySteamOwnerLeftGuestUser = 15,
};

enum EBeginAuthSessionResult {
    k_EBeginAuthSessionResultOK = 0,
    k_EBeginAuthSessionResultInvalidTicket = 1,
    k_EBeginAuthSessionResultDuplicateRequest = 2,
    k_EBeginAuthSessionResultInvalidVersion = 3,
    k_EBeginAuthSessionResultGameMismatch = 4,
    k_EBeginAuthSessionResultExpiredTicket = 5,
};

enum EAuthSessionResponse {
    k_EAuthSessionResponseOK = 0,
    k_EAuthSessionResponseUserNotConnectedToSteam = 1,
    k_EAuthSessionResponseNoLicenseOrExpired = 2,
    k_EAuthSessionResponseVACBanned = 3,
    k_EAuthSessionResponseLoggedInElseWhere = 4,
    k_EAuthSessionResponseVACCheckTimedOut = 5,
    k_EAuthSessionResponseAuthTicketCanceled = 6,
    k_EAuthSessionResponseAuthTicketInvalidAlreadyUsed = 7,
    k_EAuthSessionResponseAuthTicketInvalid = 8,
    k_EAuthSessionResponsePublisherIssuedBan = 9,
};

enum EUserHasLicenseForAppResult {
    k_EUserHasLicenseResultHasLicense = 0,
    k_EUserHasLicenseResultDoesNotHaveLicense = 1,
    k_EUserHasLicenseResultNoAuth = 2,
};

enum EPersonaState {
    k_EPersonaStateOffline = 0,
    k_EPersonaStateOnline = 1,
    k_EPersonaStateBusy = 2,
    k_EPersonaStateAway = 3,
    k_EPersonaStateSnooze = 4,
    k_EPersonaStateMax,
};

enum EFriendRelationship {
    k_EFriendRelationshipNone = 0,
    k_EFriendRelationshipBlocked = 1,
    k_EFriendRelationshipRequestRecipient = 2,
    k_EFriendRelationshipFriend = 3,
    k_EFriendRelationshipRequestInitiator = 4,
    k_EFriendRelationshipIgnored = 5,
};

enum EFriendFlags {
    k_EFriendFlagNone = 0x00,
    k_EFriendFlagBlocked = 0x01,
    k_EFriendFlagFriendshipRequested = 0x02,
    k_EFriendFlagImmediate = 0x04,
    k_EFriendFlagAll = 0xFFFF,
};

enum EP2PSend {
    k_EP2PSendUnreliable = 0,
    k_EP2PSendUnreliableNoDelay = 1,
    k_EP2PSendReliable = 2,
    k_EP2PSendReliableWithBuffering = 3,
};

enum EServerMode {
    eServerModeInvalid = 0,
    eServerModeNoAuthentication = 1,
    eServerModeAuthentication = 2,
    eServerModeAuthenticationAndSecure = 3,
};

// ----- CSteamID ----------------------------------------------------------------
class CSteamID {
public:
    CSteamID() : m_id(0) {}
    CSteamID(uint64 id) : m_id(id) {}
    void SetFromUint64(uint64 id) { m_id = id; }
    uint64 ConvertToUint64() const { return m_id; }
    bool IsValid() const { return m_id != 0; }
    uint32 GetAccountID() const { return (uint32)(m_id & 0xFFFFFFFFull); }
    bool operator==(const CSteamID &o) const { return m_id == o.m_id; }
    bool operator!=(const CSteamID &o) const { return m_id != o.m_id; }
    bool operator<(const CSteamID &o) const { return m_id < o.m_id; }

private:
    uint64 m_id;
};

const uint64 k_nxFakeSteamID = 0x0110000140000001ull;

struct SteamNetworkingIdentity {
    uint32 m_ip;
    uint16 m_port;
    SteamNetworkingIdentity() : m_ip(0), m_port(0) {}
    void SetIPv4Addr(uint32 ip, uint16 port) { m_ip = ip; m_port = port; }
    void Clear() { m_ip = 0; m_port = 0; }
};

// ----- callback structs ----------------------------------------------------------
struct ValidateAuthTicketResponse_t {
    enum { k_iCallback = 143 };
    CSteamID m_SteamID;
    EAuthSessionResponse m_eAuthSessionResponse;
    CSteamID m_OwnerSteamID;
};

struct SteamServersConnected_t {
    enum { k_iCallback = 101 };
};

struct SteamServersDisconnected_t {
    enum { k_iCallback = 103 };
    EResult m_eResult;
};

struct GSPolicyResponse_t {
    enum { k_iCallback = 115 };
    uint8 m_bSecure;
};

struct GSClientApprove_t {
    enum { k_iCallback = 201 };
    CSteamID m_SteamID;
    CSteamID m_OwnerSteamID;
};

struct GSClientDeny_t {
    enum { k_iCallback = 202 };
    CSteamID m_SteamID;
    EDenyReason m_eDenyReason;
    char m_rgchOptionalText[128];
};

struct GSClientKick_t {
    enum { k_iCallback = 203 };
    CSteamID m_SteamID;
    EDenyReason m_eDenyReason;
};

struct UserStatsReceived_t {
    enum { k_iCallback = 1101 };
    uint64 m_nGameID;
    EResult m_eResult;
    CSteamID m_steamIDUser;
};

struct UserStatsStored_t {
    enum { k_iCallback = 1102 };
    uint64 m_nGameID;
    EResult m_eResult;
};

struct UserAchievementStored_t {
    enum { k_iCallback = 1103 };
    uint64 m_nGameID;
    bool m_bGroupAchievement;
    char m_rgchAchievementName[128];
    uint32 m_nCurProgress;
    uint32 m_nMaxProgress;
};

struct P2PSessionRequest_t {
    enum { k_iCallback = 1202 };
    CSteamID m_steamIDRemote;
};

struct P2PSessionConnectFail_t {
    enum { k_iCallback = 1203 };
    CSteamID m_steamIDRemote;
    uint8 m_eP2PSessionError;
};

struct EncryptedAppTicketResponse_t {
    enum { k_iCallback = 154 };
    EResult m_eResult;
};

// ----- callback machinery (inert: RunCallbacks never dispatches) ----------------
class CCallbackBase {
public:
    CCallbackBase() : m_nCallbackFlags(0), m_iCallback(0) {}
    virtual ~CCallbackBase() {}
    virtual void Run(void *pvParam) = 0;
    virtual void Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;
    virtual int GetCallbackSizeBytes() = 0;
    int GetICallback() const { return m_iCallback; }

protected:
    uint8 m_nCallbackFlags;
    int m_iCallback;
};

template <class T, class P, int bGameserver = 0>
class CCallback : public CCallbackBase {
public:
    typedef void (T::*func_t)(P *);
    CCallback(T *pObj, func_t func) : m_pObj(pObj), m_Func(func) {}
    void Run(void *pvParam) { (m_pObj->*m_Func)((P *)pvParam); }
    void Run(void *pvParam, bool, SteamAPICall_t) { Run(pvParam); }
    int GetCallbackSizeBytes() { return (int)sizeof(P); }

private:
    T *m_pObj;
    func_t m_Func;
};

template <class T, class P>
class CCallResult : public CCallbackBase {
public:
    typedef void (T::*func_t)(P *, bool);
    CCallResult() : m_hAPICall(k_uAPICallInvalid), m_pObj(0), m_Func(0) {}
    void Set(SteamAPICall_t hAPICall, T *pObj, func_t func)
    {
        m_hAPICall = hAPICall;
        m_pObj = pObj;
        m_Func = func;
    }
    bool IsActive() const { return m_hAPICall != k_uAPICallInvalid; }
    void Cancel() { m_hAPICall = k_uAPICallInvalid; }
    void Run(void *pvParam) { (m_pObj->*m_Func)((P *)pvParam, false); }
    void Run(void *pvParam, bool bIOFailure, SteamAPICall_t) { (m_pObj->*m_Func)((P *)pvParam, bIOFailure); }
    int GetCallbackSizeBytes() { return (int)sizeof(P); }

private:
    SteamAPICall_t m_hAPICall;
    T *m_pObj;
    func_t m_Func;
};

#define STEAM_CALLBACK(thisclass, func, param, var) \
    CCallback<thisclass, param, 0> var;             \
    void func(param *pParam)

#define STEAM_GAMESERVER_CALLBACK(thisclass, func, param, var) \
    CCallback<thisclass, param, 1> var;                        \
    void func(param *pParam)

// ----- interfaces -----------------------------------------------------------------
class ISteamUser {
public:
    bool BLoggedOn() { return true; }
    CSteamID GetSteamID() { return CSteamID(k_nxFakeSteamID); }
    HAuthTicket GetAuthSessionTicket(void *ticket, int maxTicket, uint32 *ticketSize,
                                     const SteamNetworkingIdentity * = 0)
    {
        int n = maxTicket < 64 ? maxTicket : 64;
        for (int i = 0; i < n; ++i)
            ((uint8 *)ticket)[i] = (uint8)(0xA5 ^ i);
        if (ticketSize) *ticketSize = (uint32)n;
        return 1;
    }
    void CancelAuthTicket(HAuthTicket) {}
    EBeginAuthSessionResult BeginAuthSession(const void *, int, CSteamID) { return k_EBeginAuthSessionResultOK; }
    void EndAuthSession(CSteamID) {}
    SteamAPICall_t RequestEncryptedAppTicket(void *, int) { return k_uAPICallInvalid; }
    bool GetEncryptedAppTicket(void *, int, uint32 *ticketSize)
    {
        if (ticketSize) *ticketSize = 0;
        return false;
    }
    int InitiateGameConnection(void *, int, CSteamID, uint32, uint16, bool) { return 0; }
    void TerminateGameConnection(uint32, uint16) {}
};

class ISteamFriends {
public:
    const char *GetPersonaName() { return "SwitchPlayer"; }
    int GetFriendCount(int) { return 0; }
    CSteamID GetFriendByIndex(int, int) { return CSteamID(); }
    const char *GetFriendPersonaName(CSteamID) { return ""; }
    EPersonaState GetFriendPersonaState(CSteamID) { return k_EPersonaStateOffline; }
    EFriendRelationship GetFriendRelationship(CSteamID) { return k_EFriendRelationshipNone; }
    void ActivateGameOverlayToUser(const char *, CSteamID) {}
    void ActivateGameOverlayToStore(AppId_t, int = 0) {}
};

class ISteamUtils {
public:
    const char *GetIPCountry() { return "US"; }
    uint32 GetAppID() { return 42710; }
    bool IsAPICallCompleted(SteamAPICall_t, bool *failed)
    {
        if (failed) *failed = true;
        return true;
    }
};

typedef void (*SteamAPIWarningMessageHook_t)(int, const char *);

class ISteamClient {
public:
    void SetWarningMessageHook(SteamAPIWarningMessageHook_t) {}
};

class ISteamNetworking {
public:
    bool SendP2PPacket(CSteamID, const void *, uint32, EP2PSend, int = 0) { return false; }
    bool IsP2PPacketAvailable(uint32 *msgSize, int = 0)
    {
        if (msgSize) *msgSize = 0;
        return false;
    }
    bool ReadP2PPacket(void *, uint32, uint32 *msgSize, CSteamID *remote, int = 0)
    {
        if (msgSize) *msgSize = 0;
        if (remote) *remote = CSteamID();
        return false;
    }
    bool AcceptP2PSessionWithUser(CSteamID) { return false; }
    bool CloseP2PSessionWithUser(CSteamID) { return true; }
};

class ISteamUserStats {
public:
    bool RequestCurrentStats() { return false; }
    bool SetAchievement(const char *) { return false; }
    bool ClearAchievement(const char *) { return false; }
    bool StoreStats() { return false; }
    bool ResetAllStats(bool) { return false; }
    bool GetAchievement(const char *, bool *achieved)
    {
        if (achieved) *achieved = false;
        return false;
    }
    const char *GetAchievementDisplayAttribute(const char *, const char *) { return ""; }
};

class ISteamGameServer {
public:
    bool BLoggedOn() { return false; }
    bool BSecure() { return false; }
    CSteamID GetSteamID() { return CSteamID(k_nxFakeSteamID + 1); }
    EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t) { return k_EUserHasLicenseResultHasLicense; }
    void SendUserDisconnect(CSteamID) {}
    bool SendUserConnectAndAuthenticate(uint32, const void *, uint32, CSteamID *steamID)
    {
        if (steamID) *steamID = CSteamID();
        return false;
    }
    void LogOnAnonymous() {}
    void LogOff() {}
    void EnableHeartbeats(bool) {}
};

class ISteamMasterServerUpdater {
public:
    void SetActive(bool) {}
};

// ----- accessors / flat API ---------------------------------------------------------
inline ISteamUser *SteamUser()
{
    static ISteamUser s;
    return &s;
}
inline ISteamFriends *SteamFriends()
{
    static ISteamFriends s;
    return &s;
}
inline ISteamUtils *SteamUtils()
{
    static ISteamUtils s;
    return &s;
}
inline ISteamClient *SteamClient()
{
    static ISteamClient s;
    return &s;
}
inline ISteamNetworking *SteamNetworking()
{
    static ISteamNetworking s;
    return &s;
}
inline ISteamUserStats *SteamUserStats()
{
    static ISteamUserStats s;
    return &s;
}
inline ISteamGameServer *SteamGameServer()
{
    static ISteamGameServer s;
    return &s;
}
inline ISteamMasterServerUpdater *SteamMasterServerUpdater()
{
    static ISteamMasterServerUpdater s;
    return &s;
}

inline bool SteamAPI_Init() { return true; }
inline void SteamAPI_Shutdown() {}
inline void SteamAPI_RunCallbacks() {}
inline void SteamAPI_RegisterCallback(CCallbackBase *, int) {}
inline void SteamAPI_UnregisterCallback(CCallbackBase *) {}
inline void SteamAPI_RegisterCallResult(CCallbackBase *, SteamAPICall_t) {}
inline void SteamAPI_UnregisterCallResult(CCallbackBase *, SteamAPICall_t) {}
inline bool SteamAPI_RestartAppIfNecessary(uint32) { return false; }

inline bool SteamGameServer_Init(uint32, uint16, uint16, uint16, EServerMode, const char *) { return false; }
inline void SteamGameServer_Shutdown() {}
inline void SteamGameServer_RunCallbacks() {}
inline uint64 SteamGameServer_GetSteamID() { return 0; }

#endif // NX_COMPAT_STEAM_API_H
