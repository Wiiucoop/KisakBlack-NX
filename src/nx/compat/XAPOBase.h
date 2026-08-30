// XAPOBase.h -- XAPO framework compatibility declarations for the Switch
// build. Only the SDXA2*Effect class DEFINITIONS in snd_driver_xaudio2_dsp.h
// need this to parse; their method bodies live in the excluded driver TU.
#ifndef NX_COMPAT_XAPOBASE_H
#define NX_COMPAT_XAPOBASE_H

#include "XAudio2.h"

// GCC has no __uuidof; the DSP header compares REFIID against
// __uuidof(IUnknown) only.
#ifndef __uuidof
template <typename T> const GUID &nx_uuidof();
template <> inline const GUID &nx_uuidof<IUnknown>()
{
    static const GUID g = { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
    return g;
}
#define __uuidof(T) nx_uuidof<T>()
#endif

#define XAPO_FLAG_CHANNELS_MUST_MATCH      0x00000001u
#define XAPO_FLAG_FRAMERATE_MUST_MATCH     0x00000002u
#define XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH 0x00000004u
#define XAPO_FLAG_BUFFERCOUNT_MUST_MATCH   0x00000008u
#define XAPO_FLAG_INPLACE_REQUIRED         0x00000020u
#define XAPO_FLAG_INPLACE_SUPPORTED        0x00000010u

#define XAPO_REGISTRATION_STRING_LENGTH 256

typedef struct XAPO_REGISTRATION_PROPERTIES {
    GUID  clsid;
    WCHAR FriendlyName[XAPO_REGISTRATION_STRING_LENGTH];
    WCHAR CopyrightInfo[XAPO_REGISTRATION_STRING_LENGTH];
    UINT32 MajorVersion;
    UINT32 MinorVersion;
    UINT32 Flags;
    UINT32 MinInputBufferCount;
    UINT32 MaxInputBufferCount;
    UINT32 MinOutputBufferCount;
    UINT32 MaxOutputBufferCount;
} XAPO_REGISTRATION_PROPERTIES;

typedef struct XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS {
    const WAVEFORMATEX *pFormat;
    UINT32 MaxFrameCount;
} XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS, XAPO_LOCKFORPROCESS_PARAMETERS;

typedef enum XAPO_BUFFER_FLAGS {
    XAPO_BUFFER_SILENT = 0,
    XAPO_BUFFER_VALID  = 1
} XAPO_BUFFER_FLAGS;

typedef struct XAPO_PROCESS_BUFFER_PARAMETERS {
    void *pBuffer;
    XAPO_BUFFER_FLAGS BufferFlags;
    UINT32 ValidFrameCount;
} XAPO_PROCESS_BUFFER_PARAMETERS;

class IXAPO : public IUnknown {
public:
    STDMETHOD(GetRegistrationProperties) (THIS_ XAPO_REGISTRATION_PROPERTIES **ppRegistrationProperties) PURE;
    STDMETHOD(IsInputFormatSupported) (THIS_ const WAVEFORMATEX *pOutputFormat, const WAVEFORMATEX *pRequestedInputFormat,
                                       WAVEFORMATEX **ppSupportedInputFormat) PURE;
    STDMETHOD(IsOutputFormatSupported) (THIS_ const WAVEFORMATEX *pInputFormat, const WAVEFORMATEX *pRequestedOutputFormat,
                                        WAVEFORMATEX **ppSupportedOutputFormat) PURE;
    STDMETHOD(Initialize) (THIS_ const void *pData, UINT32 DataByteSize) PURE;
    STDMETHOD_(void, Reset) (THIS) PURE;
    STDMETHOD(LockForProcess) (THIS_ UINT32 InputLockedParameterCount,
                               const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
                               UINT32 OutputLockedParameterCount,
                               const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters) PURE;
    STDMETHOD_(void, UnlockForProcess) (THIS) PURE;
    STDMETHOD_(void, Process) (THIS_ UINT32 InputProcessParameterCount,
                               const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
                               UINT32 OutputProcessParameterCount,
                               XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
                               int IsEnabled) PURE;
    STDMETHOD_(UINT32, CalcInputFrames) (THIS_ UINT32 OutputFrameCount) PURE;
    STDMETHOD_(UINT32, CalcOutputFrames) (THIS_ UINT32 InputFrameCount) PURE;
};

class IXAPOParameters {
public:
    STDMETHOD_(void, SetParameters) (THIS_ const void *pParameters, UINT32 ParameterByteSize) PURE;
    STDMETHOD_(void, GetParameters) (THIS_ void *pParameters, UINT32 ParameterByteSize) PURE;
};

// Minimal CXAPOBase: enough for SDXA2Effect (which overrides the interesting
// virtuals and pokes m_lReferenceCount directly).
class CXAPOBase : public IXAPO {
protected:
    volatile LONG m_lReferenceCount;
    const XAPO_REGISTRATION_PROPERTIES *m_pRegistrationProperties;
    BOOL m_fIsLocked;

public:
    CXAPOBase(const XAPO_REGISTRATION_PROPERTIES *props)
        : m_lReferenceCount(1), m_pRegistrationProperties(props), m_fIsLocked(FALSE) {}
    virtual ~CXAPOBase() {}

    HRESULT QueryInterface(REFIID riid, void **object)
    {
        if (!object) return E_POINTER;
        (void)riid;
        *object = 0;
        return E_NOINTERFACE;
    }
    ULONG AddRef() { return (ULONG)InterlockedIncrement(&m_lReferenceCount); }
    ULONG Release()
    {
        LONG r = InterlockedDecrement(&m_lReferenceCount);
        if (r <= 0) delete this;
        return (ULONG)r;
    }

    HRESULT GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **props)
    {
        if (props) *props = 0;
        return E_NOTIMPL;
    }
    HRESULT IsInputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **) { return E_NOTIMPL; }
    HRESULT IsOutputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **) { return E_NOTIMPL; }
    HRESULT Initialize(const void *, UINT32) { return S_OK; }
    void Reset() {}
    HRESULT LockForProcess(UINT32, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *,
                           UINT32, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *)
    {
        m_fIsLocked = TRUE;
        return S_OK;
    }
    void UnlockForProcess() { m_fIsLocked = FALSE; }
    UINT32 CalcInputFrames(UINT32 outputFrameCount) { return outputFrameCount; }
    UINT32 CalcOutputFrames(UINT32 inputFrameCount) { return inputFrameCount; }
};

#endif // NX_COMPAT_XAPOBASE_H
