// WaveSupport.cpp

#include "stdafx.h"
#include <mmsystem.h>
#include "WaveSupport.h"

/////////////////////////////////
// CWaveDevice stuff
/////////////////////////////////

CWaveDevice::CWaveDevice():
	m_pwfe(NULL), m_pBufs(NULL), m_id(WAVE_MAPPER-1),
	nBuffers(0),
	hEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	InitializeCriticalSection( & cs);
}

CWaveDevice::~CWaveDevice()
{
	if (hEvent != NULL) CloseHandle(hEvent);
	DeleteCriticalSection( & cs);
	if (NULL != m_pwfe)
	{
		delete[] (char*)m_pwfe;
		m_pwfe = NULL;
	}
}

BOOL CWaveDevice::AllocateBuffers(size_t size, int count)
{
	ASSERT(count != 0);
	ASSERT(size != 0);
	ASSERT(this);

	// Reset(); // made in DeallocateBuffers()
	DeallocateBuffers();
	m_pBufs = new BUFFER_STRUCT[count];
	if (NULL == m_pBufs)
		return FALSE;
	memset(m_pBufs, 0, count * sizeof BUFFER_STRUCT);
	nBuffers = count;
	for (int i = 0; i < count; i++)
	{
		m_pBufs[i].size = size;
		m_pBufs[i].pBuf = new char[size];
		if (NULL == m_pBufs[i].pBuf)
		{
			DeallocateBuffers();
			return FALSE;
		}
	}
	return TRUE;
}

void CWaveDevice::DeallocateBuffers()
{
	ASSERT(this);
	Reset();
	ResetBuffers();
	for (unsigned i = 0; i < nBuffers; i++)
	{
		//VERIFY(MMSYSERR_NOERROR == Unprepare(i + 1));
		// buffers are unprepared in ResetBuffers
		delete[] m_pBufs[i].pBuf;
	}
	nBuffers = 0;
	delete m_pBufs;
	m_pBufs = NULL;
}

int CWaveDevice::GetBuffer(char ** ppbuf, size_t * pSize, BOOL bWait)
{
	ASSERT(this && ppbuf && pSize);

	if (NULL == m_pBufs)
		return 0;

	do {
		if (! IsOpen())
			return 0;
		// find any unused buffer
		::EnterCriticalSection(& cs);
		for (unsigned i = 0; i < nBuffers; i++)
		{
			if (0 == (m_pBufs[i].dwFlags & BUF_USED))
			{
				Unprepare(i + 1);
				m_pBufs[i].dwFlags |= BUF_USED;
				::LeaveCriticalSection(&cs);
				* ppbuf = m_pBufs[i].pBuf;
				* pSize = m_pBufs[i].size;
				return i + 1;
			}
		}
		::LeaveCriticalSection(&cs);
		if ( ! bWait)
		{
			return -1;
		}
		::WaitForSingleObject(hEvent, 500);
	}while(1);
}

BOOL CWaveDevice::ReturnBuffer(UINT hBuffer)
{
	ASSERT(this != NULL);
	ASSERT(hBuffer > 0 && hBuffer <= nBuffers);

	m_pBufs[hBuffer - 1].dwFlags &= ~BUF_USED;
	return TRUE;
}

BOOL CWaveDevice::ResetBuffers()
{
	ASSERT(this != NULL);
	if ( ! IsOpen())
	{
		return FALSE;
	}
	EnterCriticalSection( & cs);
	for (unsigned i = 0; i < nBuffers; i++)
	{
		if (m_pBufs[i].whd.dwFlags & WHDR_INQUEUE)
		{
			LeaveCriticalSection( & cs);
			return FALSE;
		}
		Unprepare(i + 1);
		m_pBufs[i].dwFlags = 0;
	}
	LeaveCriticalSection( & cs);
	return TRUE;
}

BOOL CWaveDevice::WaitForQueueEmpty(DWORD timeout)
{
	ASSERT(this != NULL);
	DWORD StartTime = GetTickCount();
	do {
		DWORD flag = 0;
		for (unsigned i = 0; i < nBuffers; i++)
		{
			flag |= m_pBufs[i].whd.dwFlags;
		}

		if (0 == (flag & WHDR_INQUEUE))
		{
			return TRUE;
		}
		if (0 == timeout)
		{
			return FALSE;
		}

		WaitForSingleObject(hEvent, timeout);
	} while (GetTickCount() - StartTime < timeout);
	return FALSE;
}

/////////////////////////////////
// CWaveOut stuff
/////////////////////////////////

CWaveOut::CWaveOut():
	m_hwo(NULL)
{
}

CWaveOut::~CWaveOut()
{
	Close();
	DeallocateBuffers();
}

MMRESULT CWaveOut::Open(UINT id, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags)
{
	ASSERT(pwfe != NULL);
	ASSERT((dwAuxFlags & ~(WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED)) == 0);

	if (dwAuxFlags & WAVE_FORMAT_QUERY)
	{
		HWAVEOUT hwo;
		return waveOutOpen( & hwo, id, const_cast<WAVEFORMATEX*>(pwfe), NULL, NULL,
							dwAuxFlags & (WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED));
	}

	ASSERT(this != NULL);
	Close();

	MMRESULT err = waveOutOpen( & m_hwo, id, pwfe, ULONG(waveOutProc), (DWORD)this,
								CALLBACK_FUNCTION | (dwAuxFlags & (WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED)));

	if (MMSYSERR_NOERROR == err)
	{
		m_pwfe = CopyWaveformat(pwfe);
		m_id = id;
	}

	return err;
}

MMRESULT CWaveOut::Open(LPCSTR szName, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags)
{
	//ASSERT(this != NULL);
	//ASSERT(pwfe != NULL);
	// member variables are not used until internal Open call
	// find the device by name
	UINT nDevCount = waveOutGetNumDevs();
	WAVEOUTCAPSA woc;
	for (UINT id = 0; id < nDevCount; id++)
	{
		if (MMSYSERR_NOERROR == waveOutGetDevCapsA(id, & woc, sizeof woc))
		{
			if (0 == _stricmp(szName, woc.szPname))
			{
				m_id = id;
				return Open(id, pwfe, dwAuxFlags);
			}
		}
	}
	return MMSYSERR_NODRIVER;
}

MMRESULT CWaveOut::Close()
{
	ASSERT(this);
	if (! IsOpen())
		return MMSYSERR_INVALHANDLE;
	Reset();
	ResetBuffers();
	MMRESULT err = waveOutClose(m_hwo);
	if (err != MMSYSERR_NOERROR)
		return err;
	m_hwo = NULL;
	m_id = WAVE_MAPPER - 1;
	if (NULL != m_pwfe)
	{
		delete[] (char*)m_pwfe;
		m_pwfe = NULL;
	}
	return MMSYSERR_NOERROR;
}

MMRESULT CWaveOut::Play(UINT hBuffer, size_t UsedSize, DWORD AuxFlags)
{
	ASSERT(this != NULL);
	ASSERT(m_hwo != NULL);
	ASSERT(0 == (AuxFlags & ~(WHDR_BEGINLOOP | WHDR_ENDLOOP)));
	ASSERT(hBuffer >= 1 && hBuffer <= nBuffers);
	ASSERT(m_pBufs != NULL);
	ASSERT(UsedSize > 0 && UsedSize <= m_pBufs[hBuffer - 1].size);

	m_pBufs[hBuffer - 1].whd.lpData = m_pBufs[hBuffer - 1].pBuf;
	m_pBufs[hBuffer - 1].whd.dwBufferLength = UsedSize;
	m_pBufs[hBuffer - 1].whd.dwBytesRecorded = UsedSize;
	m_pBufs[hBuffer - 1].whd.dwUser = (DWORD) & m_pBufs[hBuffer - 1];
	m_pBufs[hBuffer - 1].whd.dwFlags = 0;

	waveOutPrepareHeader(m_hwo, & m_pBufs[hBuffer - 1].whd,
						sizeof m_pBufs[hBuffer - 1].whd);
	m_pBufs[hBuffer - 1].whd.dwFlags |= AuxFlags;

	return waveOutWrite(m_hwo, & m_pBufs[hBuffer - 1].whd,
						sizeof m_pBufs[hBuffer - 1].whd);
}

MMRESULT CWaveOut::Reset()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_NOERROR;
	return waveOutReset(m_hwo);
}

MMRESULT CWaveOut::BreakLoop()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_NOERROR;
	return waveOutBreakLoop(m_hwo);
}

MMRESULT CWaveOut::Pause()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_INVALHANDLE;
	return waveOutPause(m_hwo);
}

MMRESULT CWaveOut::Resume()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_INVALHANDLE;
	return waveOutRestart(m_hwo);
}

DWORD CWaveOut::GetPosition(UINT type)
{
	ASSERT(this != NULL);
	ASSERT(TIME_SAMPLES == type || TIME_BYTES == type
			|| TIME_MS == type || TIME_TICKS == type);

	MMTIME mmt;
	mmt.wType = type;
	if ( ! IsOpen()
		|| waveOutGetPosition(m_hwo, & mmt, sizeof mmt) != MMSYSERR_NOERROR)
		return DWORD(-1);
	return mmt.u.ms;
}

MMRESULT CWaveOut::Unprepare(UINT hBuffer)
{
	ASSERT(this != NULL);
	ASSERT(IsOpen());
	ASSERT(hBuffer > 0 && hBuffer <= nBuffers);

	return waveOutUnprepareHeader(m_hwo, & m_pBufs[hBuffer - 1].whd,
								sizeof m_pBufs[0].whd);
}

void CALLBACK CWaveOut::waveOutProc(HWAVEOUT hwo,
									UINT uMsg,	DWORD dwInstance, DWORD dwParam1,
									DWORD dwParam2	)
{
	CWaveOut * pWo = (CWaveOut *) dwInstance;
	// can't use ASSERT in wave out callback
#ifdef _DEBUG
	if (pWo == NULL || pWo->m_hwo != hwo)
	{
		TRACE("Wrong dwInstance in waveOutProc");
	}
#endif
	switch (uMsg)
	{
	case WOM_OPEN:
		TRACE("WOM_OPEN\r\n");
		break;
	case WOM_DONE:
	{
		WAVEHDR * pWhdr = (WAVEHDR *) dwParam1;
		BUFFER_STRUCT * pBufStruct = (BUFFER_STRUCT *)pWhdr->dwUser;
		pBufStruct->dwFlags &= ~BUF_USED;
		SetEvent(pWo->hEvent);
	}
		break;
	case WOM_CLOSE:
		TRACE("WOM_CLOSE\r\n");
		break;
	default:
		TRACE("Wrong Audio Callback Msg\r\n");
		break;
	}
}

/////////////////////////////////
// CWaveIn stuff
/////////////////////////////////

CWaveIn::CWaveIn():
	m_hwi(NULL)
{
}

CWaveIn::~CWaveIn()
{
	DeallocateBuffers();
	Close();
}

MMRESULT CWaveIn::Open(UINT id, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags)
{
	ASSERT(this != NULL);
	ASSERT(pwfe != NULL);
	ASSERT((dwAuxFlags & ~(WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED)) == 0);

	if (dwAuxFlags & WAVE_FORMAT_QUERY)
	{
		HWAVEIN hwi;
		return waveInOpen( & hwi, id, const_cast<WAVEFORMATEX*>(pwfe), NULL, NULL,
							dwAuxFlags & (WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED));
	}

	ASSERT(this != NULL);
	Close();

	size_t size = sizeof (WAVEFORMATEX);
	if (pwfe->wFormatTag != WAVE_FORMAT_PCM)
	{
		size += pwfe->cbSize;
	}

	WAVEFORMATEX * pwfeTmp = (WAVEFORMATEX*) new char[size];
	if (NULL == pwfeTmp) return MMSYSERR_NOMEM;

	memcpy(pwfeTmp, pwfe, size);

	MMRESULT err = waveInOpen( & m_hwi, id, pwfeTmp, ULONG(waveInProc), (DWORD)this,
								CALLBACK_FUNCTION | (dwAuxFlags & (WAVE_ALLOWSYNC | WAVE_FORMAT_QUERY | WAVE_MAPPED)));
	if (MMSYSERR_NOERROR == err)
	{
		m_pwfe = pwfeTmp;
		m_id = id;
	}
	else
	{
		delete [] (char*) pwfeTmp;
	}

	return err;
}

MMRESULT CWaveIn::Open(LPCSTR szName, const WAVEFORMATEX * pwfe, DWORD dwAuxFlags)
{
	// find the device by name
	UINT nDevCount = waveInGetNumDevs();
	WAVEINCAPSA wic;
	for (UINT id = 0; id < nDevCount; id++)
	{
		if (MMSYSERR_NOERROR == waveInGetDevCapsA(id, & wic, sizeof wic))
		{
			if (0 == _stricmp(szName, wic.szPname))
			{
				m_id = id;
				return Open(id, pwfe, dwAuxFlags);
			}
		}
	}
	return MMSYSERR_NODRIVER;
}

MMRESULT CWaveIn::Close()
{
	ASSERT(this);
	if (! IsOpen())
		return MMSYSERR_INVALHANDLE;
	Reset();
	ResetBuffers();
	MMRESULT err = waveInClose(m_hwi);
	if (err != MMSYSERR_NOERROR)
		return err;
	m_hwi = NULL;
	m_id = WAVE_MAPPER - 1;
	if (NULL != m_pwfe)
	{
		delete[] (char*)m_pwfe;
		m_pwfe = NULL;
	}
	return MMSYSERR_NOERROR;
}

MMRESULT CWaveIn::Record(UINT hBuffer, size_t UsedSize)
{
	ASSERT(this != NULL);
	ASSERT(m_hwi != NULL);

	ASSERT(hBuffer >= 1 && hBuffer <= nBuffers);
	ASSERT(m_pBufs != NULL);
	ASSERT(UsedSize > 0 && UsedSize <= m_pBufs[hBuffer - 1].size);

	m_pBufs[hBuffer - 1].whd.lpData = m_pBufs[hBuffer - 1].pBuf;
	m_pBufs[hBuffer - 1].whd.dwBufferLength = UsedSize;
	m_pBufs[hBuffer - 1].whd.dwBytesRecorded = UsedSize;
	m_pBufs[hBuffer - 1].whd.dwUser = (DWORD) & m_pBufs[hBuffer - 1];
	m_pBufs[hBuffer - 1].whd.dwFlags = 0;
	return waveInAddBuffer(m_hwi, & m_pBufs[hBuffer - 1].whd,
							sizeof m_pBufs[hBuffer - 1].whd);
}

MMRESULT CWaveIn::Reset()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_NOERROR;
	return waveInReset(m_hwi);
}

MMRESULT CWaveIn::Stop()
{
	ASSERT(this != NULL);

	if ( ! CWaveIn::IsOpen())
		return MMSYSERR_INVALHANDLE;
	return waveInStop(m_hwi);
}

MMRESULT CWaveIn::Start()
{
	ASSERT(this != NULL);

	if ( ! IsOpen())
		return MMSYSERR_INVALHANDLE;
	return waveInStart(m_hwi);
}

DWORD CWaveIn::GetPosition(UINT type)
{
	ASSERT(this != NULL);
	ASSERT(TIME_SAMPLES == type || TIME_BYTES == type
			|| TIME_MS == type || TIME_TICKS == type);

	MMTIME mmt;
	mmt.wType = type;
	if (! IsOpen()
		|| waveInGetPosition(m_hwi, & mmt, sizeof mmt) != MMSYSERR_NOERROR)
		return DWORD(-1);
	return mmt.u.ms;
}

MMRESULT CWaveIn::Unprepare(UINT hBuffer)
{
	ASSERT(this != NULL);
	ASSERT(IsOpen());
	ASSERT(hBuffer > 0 && hBuffer <= nBuffers);

	return waveInUnprepareHeader(m_hwi, & m_pBufs[hBuffer - 1].whd,
								sizeof m_pBufs[0].whd);
}

void CALLBACK CWaveIn::waveInProc(HWAVEIN hwi,
								UINT uMsg,	DWORD dwInstance, DWORD dwParam1,
								DWORD dwParam2	)
{
}

WAVEFORMATEX * CopyWaveformat(const WAVEFORMATEX * src)
{
	int size = src->cbSize + sizeof (WAVEFORMATEX);
	if (WAVE_FORMAT_PCM == src->wFormatTag)
	{
		size = sizeof (WAVEFORMATEX);
	}
	WAVEFORMATEX * dst = ( WAVEFORMATEX *) new char[size];
	if (NULL == dst)
	{
		return NULL;
	}
	memcpy(dst, src, size);
	if (WAVE_FORMAT_PCM == src->wFormatTag)
	{
		dst->cbSize = 0;
	}
	return dst;
}

CWaveFormat::~CWaveFormat()
{
	delete[] (char*) m_pWf;
}

void CWaveFormat::Allocate(int ExtraSize, bool bCopy)
{
	int SizeToAllocate = ExtraSize + sizeof (WAVEFORMATEX);
	if (m_AllocatedSize >= SizeToAllocate)
	{
		return;
	}
	void * NewBuf = new char[SizeToAllocate];
	if (NULL == NewBuf)
	{
		return;
	}
	if (m_pWf)
	{
		if (bCopy) memcpy(NewBuf, m_pWf, m_AllocatedSize);
		delete[] (char*) m_pWf;
	}
	m_pWf = (WAVEFORMATEX *)NewBuf;
	m_AllocatedSize = SizeToAllocate;
}

void CWaveFormat::InitCdAudioFormat()
{
	Allocate(0);
	m_pWf->cbSize = 0;
	m_pWf->nSamplesPerSec = 44100;
	m_pWf->wFormatTag = WAVE_FORMAT_PCM;
	m_pWf->wBitsPerSample = 16;
	m_pWf->nChannels = 2;
	m_pWf->nBlockAlign = 4;
	m_pWf->nAvgBytesPerSec = 176400;
}

CWaveFormat & CWaveFormat::operator =(WAVEFORMATEX const * pWf)
{
	if (pWf != m_pWf)
	{
		if (WAVE_FORMAT_PCM == pWf->wFormatTag)
		{
			Allocate(0);
			memcpy(m_pWf, pWf, sizeof (PCMWAVEFORMAT));
			m_pWf->cbSize = 0;
		}
		else
		{
			Allocate(pWf->cbSize);
			memcpy(m_pWf, pWf, pWf->cbSize + sizeof WAVEFORMATEX);
		}
	}
	return *this;
}

int CWaveFormat::MatchFormat(WAVEFORMATEX const * pwf)
{
	int match = 0;
	if (pwf->wFormatTag == FormatTag())
	{
		if (WAVE_FORMAT_PCM == pwf->wFormatTag)
		{
			if (0 == memcmp(pwf, m_pWf, sizeof (PCMWAVEFORMAT)))
			{
				// exact match found
				return WaveFormatExactMatch;
			}
		}
		else
		{
			if (pwf->cbSize == m_pWf->cbSize
				&& 0 == memcmp(pwf, m_pWf, pwf->cbSize + sizeof (WAVEFORMATEX)))
			{
				// exact match found
				return WaveFormatExactMatch;
			}
		}
		match += WaveFormatMatchFormatTag;
	}
	if (pwf->nSamplesPerSec == SampleRate())
	{
		match += WaveFormatMatchSampleRate;
	}
	if (pwf->nChannels == NumChannels())
	{
		match += WaveFormatMatchCnahhels;
	}
	if (pwf->nAvgBytesPerSec == BytesPerSec())
	{
		match += WaveFormatMatchBytesPerSec;
	}
	return match;
}

