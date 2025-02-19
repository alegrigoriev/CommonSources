// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// WaveSupport.h

#pragma once

#include <vector>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

typedef ULONG CHANNEL_MASK;
#define ALL_CHANNELS ((CHANNEL_MASK)-1)
#define MAX_NUMBER_OF_CHANNELS 32

typedef short NUMBER_OF_CHANNELS;
typedef __int16 WAVE_SAMPLE;
typedef float WAVE_PEAK;

enum {
	WaveFormatMatchBitsPerSample = 1,
	WaveFormatMatchBytesPerSec = 2,
	WaveFormatMatchCnannels = 4,
	WaveFormatMatchSampleRate = 8,
	WaveFormatMatch16Bits = 0x10,
	WaveFormatMatchFormatTag = 0x40,
	WaveFormatExactMatch = 0x100,
	WaveFormatAllFieldsMatch =
		WaveFormatExactMatch
		| WaveFormatMatchBytesPerSec
		| WaveFormatMatchCnannels
		| WaveFormatMatchSampleRate
		| WaveFormatMatchBitsPerSample
		| WaveFormatMatchFormatTag,
	WaveFormatMatchCompatibleFormats = 0x10000,
	WaveFormatExcludeFormats = 0x20000,
};

enum   // flags for CWaveFormat::ValidateFormat
{
	WaveformatInvalidSize = 1,
	WaveformatInvalidNumberChannels = 2,
	WaveformatInvalidNumberOfBits = 4,
	WaveformatInvalidBlockAlign = 8,
	WaveformatInvalidBytesPerSec = 0x10,
	WaveformatInvalidChannelsMask = 0x20,
	WaveformatNoFormatLoaded = 0x40,
	// warnings
	WaveformatInvalidExtendedSize = 0x800,   // wrong size for this format
	WaveformatUnknownTag = 0x1000,
	WaveformatDataPadded = 0x2000,  // there is byte padding in each sample
	WaveformatExtendedNumBits = 0x4000, // number of bits is not 8 or 16 for PCM
	// Information
	WaveFormatMultichannel = 0x40000000,
	WaveFormatCompressed = 0x80000000,
};

enum WaveSampleType
{
	SampleTypeUnknown,
	SampleType16bit,
	SampleType24bit,
	SampleType32bit,
	SampleType8bit,
	SampleTypeFloat32,
	SampleTypeFloat64,
	SampleTypeOtherPcm,
	SampleTypeNotSupported,
	SampleTypeCompressed,
	SampleTypeAny,
};

struct WaveFormatTagEx
{
	WaveFormatTagEx() {}
	WaveFormatTagEx(WORD tag, GUID const & guid)
		: Tag(tag), SubFormat(guid)
	{}
	WaveFormatTagEx(WORD tag)
		: Tag(tag)
	{
		ASSERT(tag != WAVE_FORMAT_EXTENSIBLE);
		memzero(SubFormat);
	}

	WaveFormatTagEx(WAVEFORMATEX const * pwf)
		: Tag(pwf->wFormatTag)
	{
		if (Tag != WAVE_FORMAT_EXTENSIBLE)
		{
			memzero(SubFormat);
		}
		else
		{
			SubFormat = ((PWAVEFORMATEXTENSIBLE)pwf)->SubFormat;
		}
	}

	WORD Tag;
	GUID SubFormat;
	bool operator ==(WaveFormatTagEx const & wfx) const
	{
		return Tag == wfx.Tag
				&& (Tag != WAVE_FORMAT_EXTENSIBLE || SubFormat == wfx.SubFormat);
	}
	bool IsWma() const
	{
		return WAVE_FORMAT_MSAUDIO1 == Tag
				|| WAVE_FORMAT_WMAUDIO2 == Tag
				|| WAVE_FORMAT_WMAUDIO3 == Tag
				|| WAVE_FORMAT_WMAUDIO_LOSSLESS == Tag;
	}
	bool IsCompressed() const
	{
		if (WAVE_FORMAT_PCM == Tag
			|| WAVE_FORMAT_IEEE_FLOAT == Tag)
		{
			// PCM integer or float format
			return false;
		}

		if (WAVE_FORMAT_EXTENSIBLE == Tag)
		{
			if (SubFormat == KSDATAFORMAT_SUBTYPE_PCM
				|| SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			{
				return false;

			}
		}

		return true;
	}

	bool operator ==(WAVEFORMATEX const * wfx) const
	{
		return Tag == wfx->wFormatTag
				&& (Tag != WAVE_FORMAT_EXTENSIBLE
					|| SubFormat == ((PWAVEFORMATEXTENSIBLE)wfx)->SubFormat);
	}

	WaveFormatTagEx & operator =(WAVEFORMATEX const * pwf)
	{
		Tag = pwf->wFormatTag;
		if (Tag != WAVE_FORMAT_EXTENSIBLE)
		{
			memzero(SubFormat);
		}
		else
		{
			SubFormat = ((PWAVEFORMATEXTENSIBLE)pwf)->SubFormat;
		}
		return *this;
	}
};

inline unsigned SizeOfFormatEx(const WAVEFORMATEX * pwf)
{
	return sizeof(WAVEFORMATEX) + pwf->cbSize;
}

struct CWaveFormat
{
private:
	WAVEFORMATEXTENSIBLE wfx;
	WAVEFORMATEX * m_pWf;
	unsigned m_AllocatedSize;
public:
	CWaveFormat();
	CWaveFormat(CWaveFormat const & src);
	CWaveFormat(WAVEFORMATEX const * pWf);
	~CWaveFormat();

	WAVEFORMATEX * Allocate(unsigned Size = sizeof (WAVEFORMATEX), bool bCopy = true);
	CWaveFormat & operator =(WAVEFORMATEX const * pWf);
	CWaveFormat & operator =(CWaveFormat const & src)
	{
		return operator=(src.m_pWf);
	}
	operator const WAVEFORMATEX *() const
	{
		return m_pWf;
	}
	operator WAVEFORMATEX *()
	{
		return m_pWf;
	}

	void InitCdAudioFormat();
#if 0
	WAVEFORMATEX * Detach()
	{
		WAVEFORMATEX * pwf = m_pWf;
		m_pWf = NULL;
		m_AllocatedSize = 0;
		return pwf;
	}
#endif
	WORD & FormatTag()
	{
		return m_pWf->wFormatTag;
	}

	void FormatTag(WaveFormatTagEx const & tag)
	{
		m_pWf->wFormatTag = WORD(tag.Tag);
		if (WAVE_FORMAT_EXTENSIBLE == tag.Tag
			&& m_AllocatedSize >= sizeof (WAVEFORMATEXTENSIBLE))
		{
			((PWAVEFORMATEXTENSIBLE)m_pWf)->SubFormat = tag.SubFormat;
		}
	}

	WORD FormatTag() const
	{
		return m_pWf->wFormatTag;
	}
	DWORD & SampleRate()
	{
		return m_pWf->nSamplesPerSec;
	}

	WORD BlockAlign() const
	{
		return m_pWf->nBlockAlign;
	}

	DWORD SampleRate() const
	{
		return m_pWf->nSamplesPerSec;
	}
	DWORD & BytesPerSec()
	{
		return m_pWf->nAvgBytesPerSec;
	}
	DWORD BytesPerSec() const
	{
		return m_pWf->nAvgBytesPerSec;
	}
	WORD & NumChannels()
	{
		return m_pWf->nChannels;
	}
	WORD NumChannels() const
	{
		return m_pWf->nChannels;
	}
	WORD & BitsPerSample()
	{
		return m_pWf->wBitsPerSample;
	}

	WORD BitsPerSample() const
	{
		return m_pWf->wBitsPerSample;
	}
	WORD & SampleSize()
	{
		return m_pWf->nBlockAlign;
	}

	WORD SampleSize() const
	{
		return m_pWf->nBlockAlign;
	}
	void * FormatExtension() const
	{
		return m_pWf + 1;
	}

	// FormatSize is used as the size to read, write, copy and compare WAVEFORMATEX, and as the minimum FMT chunk size
	// Format is always allocated at least sizeof WAVEFORMATEX
	unsigned FormatSize() const
	{
		if (NULL == m_pWf)
		{
			return 0;
		}
		if (WAVE_FORMAT_PCM == m_pWf->wFormatTag)
		{
			return sizeof (PCMWAVEFORMAT);
		}
		return SizeOfFormatEx(m_pWf);
	}

	bool IsCompressed() const
	{
		return SampleTypeCompressed == GetSampleType();
	}

	WaveSampleType GetSampleType() const;

	bool IsPcm() const
	{
		WaveSampleType type = GetSampleType();

		return SampleType16bit == type
				|| SampleType8bit == type
				|| SampleType24bit == type
				|| SampleType32bit == type
				|| SampleTypeFloat32 == type
				|| SampleTypeFloat64 == type;
	}

	NUMBER_OF_CHANNELS NumChannelsFromMask(CHANNEL_MASK Channels) const;
	// mask of all channels
	CHANNEL_MASK ChannelsMask() const;
	static CHANNEL_MASK ChannelsMaskFromNumberOfChannels(int channels)
	{
		return 0xFFFFFFFFUL >> (32 - channels);
	}

	ULONG ValidateFormat() const;

	void InitFormat(WORD wFormatTag, DWORD nSampleRate,
					WORD nNumChannels, WORD nBitsPerSample = 16, WORD Size = sizeof (WAVEFORMATEX));

	void InitFormat(WaveSampleType type, DWORD nSampleRate, WORD nNumChannels);
	int MatchFormat(WAVEFORMATEX const * pWf);
	//comparision operator is used for sorting
	bool operator <(CWaveFormat const & cmp) const
	{
		return SampleRate() < cmp.SampleRate()
				|| (SampleRate() == cmp.SampleRate()
					&& BytesPerSec() < cmp.BytesPerSec());
	}
	bool operator >(CWaveFormat const & cmp) const
	{
		return SampleRate() > cmp.SampleRate()
				|| (SampleRate() == cmp.SampleRate()
					&& BytesPerSec() > cmp.BytesPerSec());
	}
	bool operator ==(CWaveFormat const & cmp) const
	{
		return m_pWf->cbSize == cmp.m_pWf->cbSize
				&& 0 == memcmp(m_pWf, cmp.m_pWf,
								(WAVE_FORMAT_PCM == m_pWf->wFormatTag) ?
									sizeof (PCMWAVEFORMAT) : SizeOfFormatEx(m_pWf));
	}
	CString GetFormatNameString(HACMDRIVER had = NULL);
	CString GetFormatTagNameString(HACMDRIVER had = NULL);
	static WAVEFORMATEX const CdAudioFormat;
};

inline bool operator ==(WAVEFORMATEX const & wf1, WAVEFORMATEX const & wf2)
{
	return wf1.cbSize == wf2.cbSize
			&& 0 == memcmp( & wf1, & wf2,
							(WAVE_FORMAT_PCM == wf1.wFormatTag) ?
								sizeof (PCMWAVEFORMAT) : SizeOfFormatEx(&wf1));
}

class CWaveDevice
{
public:
	CWaveDevice();
	virtual ~CWaveDevice();

	virtual BOOL IsOpen() const = 0;
	BOOL AllocateBuffers(unsigned size = 8192, int count = 4);
	int GetBuffer(char ** ppbuf, unsigned * pSize, BOOL bWait = TRUE);
	BOOL ReturnBuffer(UINT hBuffer);    // return unused buffer
	BOOL ResetBuffers();
	BOOL WaitForQueueEmpty(DWORD timeout);
	void DeallocateBuffers();

	virtual MMRESULT Reset() = 0;

protected:
	virtual MMRESULT Unprepare(UINT index)= 0;
	struct BUFFER_STRUCT
	{
		WAVEHDR whd;
		char * pBuf;
		unsigned size;
		DWORD dwFlags;
	};
	enum {BUF_USED = 1 };

	INT m_id;
	CWaveFormat m_wfe;
	CRITICAL_SECTION cs;
	HANDLE hEvent;
	BUFFER_STRUCT * m_pBufs;
	UINT nBuffers;
	// the class doesn't allow assignment and copy
private:
	CWaveDevice(const CWaveDevice &);
	CWaveDevice & operator=(const CWaveDevice &);
};

class CWaveOut : public CWaveDevice
{
public:
	CWaveOut();
	virtual ~CWaveOut();

	static UINT GetNumDevs()
	{
		return waveOutGetNumDevs();
	}

	static MMRESULT GetDevCaps(UINT_PTR id, LPWAVEOUTCAPS pCaps)
	{
		return waveOutGetDevCaps(id, pCaps, sizeof *pCaps);
	}

	virtual BOOL IsOpen() const
	{
		return m_hwo != NULL;
	}

	MMRESULT Open(UINT id, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags = 0u);
	MMRESULT Open(LPCSTR szName, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags = 0u);

	MMRESULT GetDevCaps(LPWAVEOUTCAPS pCaps) const;

	MMRESULT Close();
	MMRESULT Reset();
	MMRESULT Pause();
	MMRESULT Resume();
	MMRESULT BreakLoop();
	DWORD GetPosition(UINT type=TIME_SAMPLES) const;

	MMRESULT Play(UINT hBuffer, unsigned UsedSize, DWORD AuxFlags = 0);

private:
	HWAVEOUT m_hwo;

	virtual MMRESULT Unprepare(UINT index);
	static void CALLBACK waveOutProc(HWAVEOUT hwo,
									UINT uMsg,	DWORD_PTR dwInstance, DWORD_PTR dwParam1,
									DWORD dwParam2);

	// the class doesn't allow assignment and copy
private:
	CWaveOut(const CWaveOut &);
	CWaveOut & operator=(const CWaveOut&);
};

class CWaveIn : public CWaveDevice
{
public:
	CWaveIn();
	virtual ~CWaveIn();

	static UINT GetNumDevs()
	{
		return waveInGetNumDevs();
	}

	static MMRESULT GetDevCaps(UINT_PTR id, LPWAVEINCAPS pCaps)
	{
		return waveInGetDevCaps(id, pCaps, sizeof *pCaps);
	}

	virtual BOOL IsOpen() const
	{
		return m_hwi != NULL;
	}
	MMRESULT Open(UINT id, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags = 0u);
	MMRESULT Open(LPCSTR szName, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags = 0u);

	MMRESULT GetDevCaps(LPWAVEINCAPS pCaps) const;

	MMRESULT Close();
	MMRESULT Reset();
	MMRESULT Start();
	MMRESULT Stop();

	MMRESULT Record(UINT hBuffer, unsigned UsedSize);

private:
	virtual MMRESULT Unprepare(UINT index);
	HWAVEIN m_hwi;
	DWORD GetPosition(UINT type=TIME_SAMPLES) const;
	static void CALLBACK waveInProc(HWAVEIN hwi,
									UINT uMsg,	DWORD_PTR dwInstance, DWORD dwParam1,
									DWORD dwParam2	);

private:
	// the class doesn't allow assignment and copy
	CWaveIn(const CWaveIn &);
	CWaveIn & operator=(const CWaveIn &);
};

class CAudioMixer
{
	HMIXER m_hmix;
};

class CAudioCompressionManager
{
public:
	CAudioCompressionManager(CWaveFormat const & wf)
		: m_Wf(wf)
	{
	}

	struct FormatTagItem
	{
		FormatTagItem() : m_hadid(NULL) {}

		void SetData(WaveFormatTagEx const & pwf, CString const & name, HACMDRIVERID hadid)
		{
			Tag = pwf;
			Name = name;
			m_hadid = hadid;
		}

		WaveFormatTagEx Tag;
		HACMDRIVERID m_hadid;
		CString Name;
	};

	struct FormatItem
	{
		FormatItem() {}
		FormatItem(WAVEFORMATEX const* pwf, LPCTSTR name, int index)
			: Wf(pwf), Name(name), TagIndex(index) {}
		CWaveFormat Wf;
		CString Name;
		int TagIndex;
		bool operator <(FormatItem const & cmp) const
		{
			return Wf < cmp.Wf;
		}
		bool operator >(FormatItem const & cmp) const
		{
			return Wf > cmp.Wf;
		}
		bool operator ==(FormatItem const & cmp) const
		{
			return Wf == cmp.Wf;
		}
	};

	std::vector<FormatTagItem> m_FormatTags;
	std::vector<FormatItem> m_Formats;

	bool FillMultiFormatArray(unsigned nSelFrom, unsigned nSelTo, int Flags);
	bool FillFormatArray(unsigned nSel, int Flags)
	{
		return FillMultiFormatArray(nSel, nSel, Flags);
	}

	void FillFormatTagArray(WAVEFORMATEX const * pwf,
							WaveFormatTagEx const ListOfTags[],
							int NumTags, DWORD flags = 0);
	void FillWmaFormatTags();
	void FillMp3EncoderTags(DWORD Flags);
	void FillMp3FormatArray(DWORD Flags);
	void FillLameEncoderFormats();

	static CString GetFormatName(HACMDRIVER had, WAVEFORMATEX const * pWf);
	static CString GetFormatTagName(HACMDRIVER had, DWORD Tag);

	int GetFormatsStrings(std::vector<CString> &Strings, CWaveFormat & Wf,
						WaveFormatTagEx SelectedTag, int SelectedBitrate);
protected:
	static BOOL _stdcall FormatTestEnumCallback(
												HACMDRIVERID hadid, LPACMFORMATDETAILS pafd,
												DWORD_PTR dwInstance, DWORD fdwSupport);
	static BOOL _stdcall FormatTagEnumCallback(
												HACMDRIVERID hadid, LPACMFORMATTAGDETAILS paftd,
												DWORD_PTR dwInstance, DWORD fdwSupport);
	static BOOL _stdcall FormatEnumCallback(
											HACMDRIVERID hadid, LPACMFORMATDETAILS pafd,
											DWORD_PTR dwInstance, DWORD fdwSupport);
	// source wave format for comparing:
	CWaveFormat m_Wf;

private:
	// the class doesn't allow assignment and copy
	CAudioCompressionManager(const CAudioCompressionManager &);
	CAudioCompressionManager & operator=(const CAudioCompressionManager &);
};

class AudioStreamConvertor
{
	typedef void (CALLBACK * AcmStreamCallback)(HACMSTREAM , UINT uMsg,
												DWORD_PTR dwInstance, LPARAM , LPARAM );
	typedef void (CALLBACK * AcmStreamCallbackPtr)(HACMSTREAM , UINT uMsg,
													PVOID pInstance, LPARAM , LPARAM );
public:
	AudioStreamConvertor(HACMDRIVER drv = NULL);
	~AudioStreamConvertor();

	BOOL SuggestFormat(WAVEFORMATEX const * pWf1,
						WAVEFORMATEX * pWf2, unsigned MaxFormat2Size, DWORD flags);

	BOOL QueryOpen(WAVEFORMATEX const * pWfSrc,
					WAVEFORMATEX const * pWfDst, DWORD flags = ACM_STREAMOPENF_NONREALTIME);

	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst, DWORD flags = ACM_STREAMOPENF_NONREALTIME);

	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst, HWND hCallbackWnd, DWORD_PTR dwInstance,
			DWORD flags = ACM_STREAMOPENF_NONREALTIME);

	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst, HWND hCallbackWnd, PVOID pInstance,
			DWORD flags = ACM_STREAMOPENF_NONREALTIME)
	{
		return Open(pWfSrc, pWfDst, hCallbackWnd,
					reinterpret_cast<DWORD_PTR>(pInstance), flags);
	}

	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst, HANDLE hEvent,
			DWORD flags = ACM_STREAMOPENF_NONREALTIME);

	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst,
			AcmStreamCallback Callback,
			DWORD_PTR dwInstance, DWORD flags = ACM_STREAMOPENF_NONREALTIME);
	BOOL Open(WAVEFORMATEX const * pWfSrc,
			WAVEFORMATEX const * pWfDst,
			AcmStreamCallbackPtr Callback,
			PVOID pInstance, DWORD flags = ACM_STREAMOPENF_NONREALTIME)
	{
		return Open(pWfSrc, pWfDst,
					reinterpret_cast<AcmStreamCallback>(Callback),
					reinterpret_cast<DWORD_PTR>(pInstance), flags);
	}

	void Close();
	BOOL Reset(DWORD flags = 0);

	BOOL AllocateBuffers(unsigned PreferredInBufSize = 0x10000,
						unsigned PreferredOutBufSize = 0x10000);

	// convert data to the internal buffer
	BOOL Convert(void const * pSrcBuf, unsigned SrcBufSize, unsigned * pSrcBufUsed,
				void* * ppDstBuf /*optional*/,
				unsigned * pDstBufFilled /*optional*/,
				DWORD flags = ACM_STREAMCONVERTF_BLOCKALIGN);

	// read data from the internal buffer
	unsigned GetConvertedData(void * pDstBuf, unsigned DstBufSize);

protected:
	ACMSTREAMHEADER m_ash;
	HACMSTREAM m_acmStr;
	HACMDRIVER m_acmDrv;
	MMRESULT m_MmResult;

	DWORD m_SrcBufSize;
	DWORD m_DstBufSize;
	DWORD m_DstBufRead;

	// the class doesn't allow assignment and copy
private:
#ifdef _DEBUG
	ULONG64 m_ProcessedInputBytes;
	ULONG64 m_SavedOutputBytes;
	ULONG64 m_GotOutputBytes;
#endif

	AudioStreamConvertor(const AudioStreamConvertor &);
	AudioStreamConvertor & operator=(const AudioStreamConvertor &);
};

inline CString CWaveFormat::GetFormatNameString(HACMDRIVER had)
{
	return CAudioCompressionManager::GetFormatName(had, *this);
}

inline CString CWaveFormat::GetFormatTagNameString(HACMDRIVER had)
{
	return CAudioCompressionManager::GetFormatTagName(had, FormatTag());
}

WAVEFORMATEX * CopyWaveformat(const WAVEFORMATEX * src);

void CopyWaveSamples(void * pDstBuf, CHANNEL_MASK DstChannels,
					NUMBER_OF_CHANNELS NumDstChannels,
					void const * pSrcBuf, CHANNEL_MASK SrcChannels,
					NUMBER_OF_CHANNELS NumSrcChannels,
					unsigned Samples,
					WaveSampleType DstType = SampleType16bit, WaveSampleType SrcType = SampleType16bit);
