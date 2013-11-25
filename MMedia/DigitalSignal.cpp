// Signal.cpp
#include "afxwin.h"
#include "DigitalSignal.h"

double CSignal::GetDelay(double ) const
{
	return 0.;
}

double CSignal::SamplingRate()
{
	return dSamplingRate;
}

double CSignal::SignalLength()
{
	return (iSamplesAvailable + iFirstSample) / SamplingRate();
}

CSignalIterator * CSignal_Step::GetSignal(int iStart, int iLength)
{
	// zero sample is 0, all others are 1
	// make sure that there are enough samples in the buffer
	if (iStart > 1) iStart = 1;
	int iNeededSamples = iStart + iLength;
	if (FALSE == bValid || iNeededSamples > iSamplesAvailable)
	{
		pInSignal->Allocate(iNeededSamples);
		pInSignal->SetStoreIndex(0);
		for (int i = 0; i < iNeededSamples; i++)
		{
			pInSignal->SetNextDouble(1.);
		}
		pInSignal->SetDoubleAt(0., 0);
		iSamplesAvailable = iNeededSamples;
		bValid = TRUE;
	}
	pInSignal->SetRetrieveIndex(iStart);
	return pInSignal;
}

CSignalIterator * CSignal_Pulse::GetSignal(int iStart, int iLength)
{
	// zero sample is 0, all others are 1
	// make sure that there are enough samples in the buffer
	if (iStart > 1) iStart = 1;
	int iNeededSamples = iStart + iLength;
	if (FALSE == bValid || iNeededSamples > iSamplesAvailable)
	{
		pInSignal->Allocate(iNeededSamples);
		pInSignal->SetStoreIndex(0);
		for (int i = 0; i < iNeededSamples; i++)
		{
			pInSignal->SetNextDouble(0.);
		}
		pInSignal->SetDoubleAt(1., 0);
		iSamplesAvailable = iNeededSamples;
		bValid = TRUE;
	}
	pInSignal->SetRetrieveIndex(iStart);
	return pInSignal;
}

CSignalIterator * CSignal_Sine::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iSignalLength = int(dSignalLength * SamplingRate());
	if (iStart > iSignalLength) iStart = iSignalLength;
	int iLastSample = iStart + iLength;
	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	pInSignal->Allocate(iLength);

	double dOmega = dFrequency * 2 * M_PI / SamplingRate();
	pInSignal->SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < iSignalLength)
		{
			pInSignal->SetNextDouble(sin(dOmega * (i + iStart)));
		}
		else
		{
			pInSignal->SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(0);
	return pInSignal;
}

CSignalIterator * CSignal_Square::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iSignalLength = int(dSignalLength * SamplingRate());
	if (iStart > iSignalLength) iStart = iSignalLength;
	int iLastSample = iStart + iLength;
	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	pInSignal->Allocate(iLength);

	pInSignal->SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < iSignalLength)
		{
			pInSignal->SetNextDouble(1.);
		}
		else
		{
			pInSignal->SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(0);
	return pInSignal;
}

CSignalIterator * CSignal_Triangle::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iSignalLength = int(dSignalLength * SamplingRate());
	if (iStart > iSignalLength) iStart = iSignalLength;
	int iLastSample = iStart + iLength;
	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	pInSignal->Allocate(iLength);

	pInSignal->SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < iSignalLength)
		{
			double tmp = 2. * (i + iStart) / iSignalLength;
			if (tmp > 1.) tmp = 2. - tmp;
			pInSignal->SetNextDouble(tmp);
		}
		else
		{
			pInSignal->SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(0);
	return pInSignal;
}

CSignalIterator * CSignal_FM::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iSignalLength = int(dSignalLength * SamplingRate());
	if (iStart > iSignalLength) iStart = iSignalLength;
	int iLastSample = iStart + iLength;
	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	pInSignal->Allocate(iLength);

	// F = F0 + sin(Wm*t) * dw
	// FM signal phase = W*t - cos(Wm*t) * dW / Wm
	double dOmega = dFrequency * 2 * M_PI / SamplingRate();
	double dModOmega = dModFrequency * 2 * M_PI / SamplingRate();
	double dDevOmega = dDeviation * 2 * M_PI / SamplingRate();
	pInSignal->SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < iSignalLength)
		{
			pInSignal->SetNextDouble(sin(dOmega * (i + iStart)
										- cos(dModOmega * (i + iStart))
										* dDevOmega / dModOmega));
		}
		else
		{
			pInSignal->SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(0);
	return pInSignal;
}

CSignalIterator * CSignal_LFM::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iSignalLength = int(dSignalLength * SamplingRate());
	if (iStart > iSignalLength) iStart = iSignalLength;
	int iLastSample = iStart + iLength;
	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	pInSignal->Allocate(iLength);

	// F = dStartFrequency + t / iSignalLength *
	//  (dEndFrequency - dStartFrequency)
	// LFM signal phase = t * dStartFrequency + t * t / 2
	//  / iSignalLength * (dEndFrequency - dStartFrequency)
	double dOmegaStart = dStartFrequency * 2 * M_PI / SamplingRate();
	double dOmegaEnd = dEndFrequency * 2 * M_PI / SamplingRate();

	pInSignal->SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < iSignalLength)
		{
			double t = i + iStart;
			pInSignal->SetNextDouble(
									sin(t * (dOmegaStart + t / 2.  / iSignalLength
											* (dOmegaEnd - dOmegaStart))));
		}
		else
		{
			pInSignal->SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(0);
	return pInSignal;
}

CSignalIterator * CSignal_Filtered::GetSignal(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iLastSample = iStart + iLength;
	int iTrailLength = pFilter->TrailLength();

	int iReqBegin = iStart - iTrailLength;
	if (iReqBegin < 0) iReqBegin = 0;

	int iReqLength = iLastSample - iReqBegin;

	if (IsValid() && iReqBegin >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	// we need some amount of source signal before
	// starting point - TrailLength. It depends on
	// the filter technology
	pInSignal->Allocate(iReqLength);
	pInSignal->SetStoreIndex(0);

	CSignalIterator * pSrc =
		pSourceSignal->GetSignal(iReqBegin, iReqLength);

	SamplingRate();
	pFilter->TemporalResponce(pInSignal, pSrc, iReqLength);

	iFirstSample = iReqBegin;
	iSamplesAvailable = iReqLength;
	bValid = TRUE;

	pInSignal->SetRetrieveIndex(iStart - iFirstSample);
	return pInSignal;
}

double CSignal_Filtered::GetDelay(double dFrequency) const
{
	return pSourceSignal->GetDelay(dFrequency)
			+ pFilter->GroupDelay(dFrequency) / pSourceSignal->SamplingRate();
}

double CSignal_Filtered::SignalLength()
{
	return pSourceSignal->SignalLength();
}

double CSignal_Filtered::SamplingRate()
{
	if (pSourceSignal != NULL)
	{
		dSamplingRate = pSourceSignal->SamplingRate();
	}
	return dSamplingRate;
}

CSignalIterator * CSignalTransparent::GetSignal(int iStart, int iLength)
{
	ASSERT(pSourceSignal != NULL);
	CSignalIterator * tmp = pSourceSignal->GetSignal(iStart, iLength);
	bValid = TRUE;
	dSamplingRate = pSourceSignal->SamplingRate();
	return tmp;
}

double CSignalTransparent::SignalLength()
{
	return pSourceSignal->SignalLength();
}

double CSignalTransparent::SamplingRate()
{
	if (pSourceSignal != NULL)
	{
		dSamplingRate = pSourceSignal->SamplingRate();
	}
	return dSamplingRate;
}

double CSignalTransparent::GetDelay(double dFrequency) const
{
	return pSourceSignal->GetDelay(dFrequency);
}

CSignalWave::CSignalWave()
	:hMemoryMapping(INVALID_HANDLE_VALUE), pFileBase(NULL),
	nTotalSize(NULL), dwFlags(NULL), pDataAddress(NULL),
	nCurrChannel(0), nWaveSamples(0), m_IsOpen(false)
{
}

CSignalWave::~CSignalWave()
{
	Close();
}

void CSignalWave::Close()
{
	// unmap file view
	m_IsOpen = false;
	if (pFileBase != NULL)
	{
		UnmapViewOfFile(pFileBase);
		pFileBase = NULL;
	}
	// close file mapping
	if (INVALID_HANDLE_VALUE != hMemoryMapping)
	{
		CloseHandle(hMemoryMapping);
		hMemoryMapping = INVALID_HANDLE_VALUE;
	}
	// close wave file
	if (CFile::hFileNull != m_File.m_hFile)
	{
		m_File.Close();
	}
	// deallocate decimators
	for (int i = 0; i <= aDecimators.GetUpperBound(); i++)
	{
		delete aDecimators[i];
	}
	aDecimators.RemoveAll();

	dwFlags = 0;
	pDataAddress = NULL;
	nTotalSize = 0;

	delete pInSignal;
	pInSignal = NULL;

	Invalidate();
}

BOOL CSignalWave::Open(LPCTSTR szFilename)
{
	Close();
	if (FALSE == m_File.Open(szFilename,
							CFile::modeRead | CFile::shareDenyWrite))
	{
		return FALSE;
	}
	nTotalSize = m_File.GetLength();

	HANDLE hMapping = CreateFileMapping((HANDLE)m_File.m_hFile,
										NULL,
										PAGE_READONLY,
										0, 0, NULL);
	if (NULL == hMapping)
	{
		Close();
		return FALSE;
	}
	hMemoryMapping = hMapping;
	pFileBase = MapViewOfFile(hMemoryMapping,
							FILE_MAP_READ,
							0, 0,   // offset
							0); // the entire file is mapped

	if (NULL == pFileBase)
	{
		Close();
		return FALSE;
	}
	// parse Wave header
	MMIOINFO mmio;
	memset(&mmio, 0, sizeof mmio);
	mmio.fccIOProc = FOURCC_MEM;
	mmio.pchBuffer = (HPSTR) pFileBase;
	if (nTotalSize > 0x10000)
	{
		mmio.cchBuffer = 0x10000; //nTotalSize;
	}
	else
	{
		mmio.cchBuffer = nTotalSize;
	}

	HMMIO hmmio = mmioOpen(NULL, &mmio, MMIO_READ);

	MMCKINFO    mmckinfoParent;     // parent chunk information

	MMCKINFO    mmckinfoSubchunk;   // subchunk information structure
	DWORD       dwFmtSize;          // size of "FMT" chunk
	DWORD       dwDataSize;         // size of "DATA" chunk

	// Locate a "RIFF" chunk with a "WAVE" form type to make
	// sure the file is a waveform-audio file.

	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL,
					MMIO_FINDRIFF))
	{
		AfxMessageBox("This is not a waveform-audio file.");
		mmioClose(hmmio, 0);
		Close();
		return FALSE;
	}
	// Find the "FMT" chunk (form type "FMT"); it must be
	// a subchunk of the "RIFF" chunk.
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
					MMIO_FINDCHUNK))
	{
		AfxMessageBox("Waveform-audio file has no \"FMT\" chunk.");
		mmioClose(hmmio, 0);
		Close();
		return FALSE;
	}

	// Get the size of the "FMT" chunk. Allocate
	// and lock memory for it.
	dwFmtSize = mmckinfoSubchunk.cksize;
	// Read the "FMT" chunk.
	if (dwFmtSize != sizeof wfFormat
		|| mmioRead(hmmio, (HPSTR) &wfFormat, dwFmtSize) != int(dwFmtSize)
		|| wfFormat.wf.wFormatTag != WAVE_FORMAT_PCM
		|| wfFormat.wBitsPerSample != 8
		&& wfFormat.wBitsPerSample != 16
		|| wfFormat.wf.nChannels != 1
		&& wfFormat.wf.nChannels != 2
		|| wfFormat.wf.nSamplesPerSec == 0)
	{
		AfxMessageBox("Failed to read format chunk.");
		mmioClose(hmmio, 0);
		Close();
		return FALSE;
	}

	// Ascend out of the "FMT" subchunk.
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);

	// Find the data subchunk. The current file position should be at
	// the beginning of the data chunk; however, you should not make
	// this assumption. Use mmioDescend to locate the data chunk.
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
					MMIO_FINDCHUNK))
	{
		AfxMessageBox("Waveform-audio file has no data chunk.");
		mmioClose(hmmio, 0);
		Close();
		return FALSE;
	}

	// Get the size of the data subchunk.
	dwDataSize = mmckinfoSubchunk.cksize;
	if (dwDataSize == 0L)
	{
		AfxMessageBox("The data chunk contains no data.");
		mmioClose(hmmio, 0);
		Close();
		return FALSE;
	}

	// Read the waveform-audio data subchunk.

	memset(&mmio, 0, sizeof mmio);
	mmioGetInfo(hmmio, &mmio, 0);
	pDataAddress = mmio.pchNext;

	dSamplingRate = wfFormat.wf.nSamplesPerSec;
	nWaveSamples = dwDataSize / ((wfFormat.wBitsPerSample / 8)
								* wfFormat.wf.nChannels);

	mmioClose(hmmio, 0);
	// create source signal array
	if (1 == wfFormat.wf.nChannels)
	{
		if (8 == wfFormat.wBitsPerSample)
		{
			pInSignal = new CMappedSignalArray<unsigned __int8, 1>((unsigned __int8*) pDataAddress, nWaveSamples);
		}
		else
		{
			pInSignal = new CMappedSignalArray<__int16, 1>((__int16*)pDataAddress, nWaveSamples);
		}
	}
	else
	{
		if (8 == wfFormat.wBitsPerSample)
		{
			pInSignal = new CMappedSignalArray<unsigned __int8, 2>((unsigned __int8*) pDataAddress, nWaveSamples);
		}
		else
		{
			pInSignal = new CMappedSignalArray<__int16, 2>((__int16*)pDataAddress, nWaveSamples);
		}
	}

	SetChannel(0);
	CreateDecimators(8);
	m_IsOpen = true;
	return TRUE;
}

CFilterband * CSignalWave::CreateFilterband
	(double dLowFreq, double dHighFreq)
{
	// Create the filterband object
	CFilterband * pFilterband = new CFilterband;
	if (NULL == pFilterband)
	{
		return NULL;
	}
	pFilterband->dLowFreq = dLowFreq;
	pFilterband->dHighFreq = dHighFreq;
	pFilterband->dTransitionWidth = 0.1; //0.05;
	pFilterband->dPassLoss = 1.;
	// find appropriate decimated signal source

	CSignal * pSignal;
#if 0
	if ((dHighFreq - dLowFreq) * 10. >= SamplingRate() * 0.5)
	{
		pSignal = this;
	}
	else
	{
		for (int subband = 0; subband
			< aDecimators.GetUpperBound(); subband++)
		{
			CSignalDecimator * pDecimator = aDecimators[subband];
			if ((dHighFreq - dLowFreq) * 10. >= pDecimator->dBandwidth
				|| dHighFreq > aDecimators[subband + 1]->dBandwidth)
			{
				break;
			}
		}

		pSignal = aDecimators[subband];
	}

#else
	int subband;
	for (subband = 0; subband
		< aDecimators.GetUpperBound(); subband++)
	{
		CSignalDecimator * pDecimator = aDecimators[subband];
		if (dHighFreq < aDecimators[subband]->dBandwidth)
		{
			pSignal = aDecimators[subband];
			break;
		}
	}

	if (subband == aDecimators.GetUpperBound())
	{
		pSignal = this;
	}

#endif
	// Create band filter
	pFilterband->dSamplingRate = pSignal->SamplingRate();
	pFilterband->SetSourceSignal(pSignal);
	pFilterband->CreateFilter();
	pFilterband->Set0dBLevel(32768.);
	return pFilterband;
}

CSignalIterator * CSignalWave::GetSignal(int iStart, int iLength)
{
	if (NULL == pDataAddress)
		return NULL;
	// make sure that there are enough samples in the buffer
	if (iStart > nWaveSamples)
	{
		iStart = nWaveSamples;
	}
	else
	{
		pInSignal->SetRetrieveIndex(iStart);
	}
	int iLastSample = iStart + iLength;

	if (bValid && iStart >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		OutSignal.SetRetrieveIndex(iStart - iFirstSample);
		return & OutSignal;
	}

	OutSignal.Allocate(iLength);
	OutSignal.SetStoreIndex(0);
	for (int i = 0; i < iLength; i++)
	{
		if (i + iStart < nWaveSamples)
		{
			OutSignal.SetNextDouble(pInSignal->GetNextDouble());
		}
		else
		{
			OutSignal.SetNextDouble(0.);
		}
	}
	iFirstSample = iStart;
	iSamplesAvailable = iLength;
	bValid = TRUE;
	OutSignal.SetRetrieveIndex(0);
	return & OutSignal;
}

void CSignalWave::SetChannel(int nChan)
{
	if (nChan < 0 || nChan >= wfFormat.wf.nChannels)
		return;
	nCurrChannel = nChan;
	pInSignal->SetChannel(nChan);
	Invalidate();  // invalidate the data
}

double CSignalWave::SignalLength()
{
	return nWaveSamples / SamplingRate();
}

CSignalDecimator::CSignalDecimator()
	: DecimFilter(NULL),
	nDecimFactor(-1), dBandwidth(0.), pFilter(new CDigitalFilter)
{
}

CSignalDecimator::~CSignalDecimator()
{
	delete pFilter;
}

void CSignalDecimator::Init(int decim_factor /* = 2 */,
							double dSafetyBand /* = 0.9 */,
							double dPassbandLossDB /* = 0.5 */,
							double dStopbandLossDB /* = 70.*/ )
{
	ASSERT(dSafetyBand > 0. && dSafetyBand < 1.);

	NewFilterData nfd;
	nDecimFactor = decim_factor;
	memset(&nfd, 0, sizeof nfd);
	nfd.iFilterType = FILTER_LOWPASS;
	nfd.iCoeffType = FILTER_COEFF_REAL;
	nfd.iFilterClass = FILTER_CLASS_IIR;
	nfd.iFirType = FILTER_FIR_ZEROPHASE;
	nfd.iIirType = FILTER_IIR_ELLIPTIC;
	nfd.bPowerSymm = FALSE;
	nfd.dSamplingRate = DecimFilter.SamplingRate();
	nfd.dHighFreq = DecimFilter.SamplingRate() * 0.5 / decim_factor;
	nfd.dLowFreq = nfd.dHighFreq * dSafetyBand;
	nfd.dPassLoss = dPassbandLossDB;
	nfd.dStopLoss = dStopbandLossDB;

	iSamplesAvailable = 0;  // invalidate the data
	if(pFilter->Create(&nfd))
	{
		DecimFilter.SetFilter(pFilter);
		dBandwidth = nfd.dLowFreq * dSafetyBand;;
		Invalidate();
	}
	else
	{
		nDecimFactor = -1;
	}
}

void CSignalDecimator::SetSourceSignal(CSignal * pSignal)
{
	ASSERT(pSignal != NULL);
	delete pInSignal;
	pInSignal = NULL;
	DecimFilter.SetSourceSignal(pSignal);
	if (pSignal->Flags(SIGNAL_COMPLEX))
	{
		pInSignal = new CSignalArray<fcomplex>;
	}
	else
	{
		pInSignal = new CSignalArray<float>;
	}
}

CSignalIterator * CSignalDecimator::GetSignal(int iStart, int iLength)
{
	if (nDecimFactor <= 0)
	{
		return NULL;
	}
	// make sure that there are enough samples in the buffer
	int iLastSample = iStart + iLength;

	int iReqBegin = iStart;
	if (iReqBegin < 0) iReqBegin = 0;

	int iReqLength = iLastSample - iReqBegin;

	if (IsValid() && iReqBegin >= iFirstSample
		&& iLastSample <= iSamplesAvailable + iFirstSample)
	{
		pInSignal->SetRetrieveIndex(iStart - iFirstSample);
		return pInSignal;
	}

	// we need some amount of source signal before
	// starting point - TrailLength. It depends on
	// the filter technology
	pInSignal->Allocate(iReqLength);

	CSignalIterator * pSrc =
		DecimFilter.GetSignal(iReqBegin * nDecimFactor,
							iReqLength * nDecimFactor);

	SamplingRate();

	pInSignal->SetStoreIndex(0);

	if (pInSignal->Flags(SIGNAL_COMPLEX))
	{
		for (int i = 0; i < iReqLength; i++)
		{
			pInSignal->SetNextComplex(pSrc->GetNextComplex());
			for (int j = 1; j < nDecimFactor; j++)
			{
				pSrc->GetNextComplex();
			}
		}
	}
	else
	{
		for (int i = 0; i < iReqLength; i++)
		{
			pInSignal->SetNextDouble(pSrc->GetNextDouble());
			for (int j = 1; j < nDecimFactor; j++)
			{
				pSrc->GetNextDouble();
			}
		}
	}

	iFirstSample = iReqBegin;
	iSamplesAvailable = iReqLength;
	bValid = TRUE;
	pInSignal->SetRetrieveIndex(iStart - iFirstSample);
	return pInSignal;
}

double CSignalDecimator::GetDelay(double dFrequency) const
{
	return DecimFilter.GetDelay(dFrequency);
}

double CSignalDecimator::SignalLength()
{
	return DecimFilter.SignalLength();
}

double CSignalDecimator::SamplingRate()
{
	dSamplingRate = DecimFilter.SamplingRate() / nDecimFactor;
	return dSamplingRate;
}

void CSignalWave::CreateDecimators(int nCount)
{
	CSignal * pSignal = this;
	for (int i = 0; i < nCount; i++)
	{
		CSignalDecimator * pDecimator =
			new CSignalDecimator;
		pDecimator->SetSourceSignal(pSignal);
		pDecimator->Init(2, 0.8, 1., 80.);
		pSignal = pDecimator;
#if 0
		aDecimators.Add(pDecimator);
#else
		aDecimators.InsertAt(0, pDecimator);
#endif
	}
}

BOOL CSignal::IsValid()
{
	return bValid;
}

BOOL CSignal_Filtered::IsValid()
{
	return bValid && pSourceSignal->IsValid();
}

BOOL CSignalDecimator::IsValid()
{
	return bValid && DecimFilter.IsValid();
}

BOOL CSignalTransparent::IsValid()
{
	return pSourceSignal->IsValid();
}

void CSignal_Filtered::SetFilter(CDigitalFilter * pSourceFilter)
{
	ASSERT(pSourceFilter != NULL);
	delete pInSignal;
	pInSignal = NULL;
	pFilter = pSourceFilter;
	if (pSourceSignal != NULL && pFilter != NULL)
	{
		if (pSourceFilter->Flags(FILTER_COMPLEX) ||
			pSourceSignal->Flags(SIGNAL_COMPLEX))
		{
			pInSignal = new CSignalArray<fcomplex>;
		}
		else
		{
			pInSignal = new CSignalArray<float>;
		}
	}
	iSamplesAvailable = 0;
}

void CSignal_Filtered::SetSourceSignal(CSignal * pSignal)
{
	ASSERT(pSignal != NULL);
	delete pInSignal;
	pInSignal = NULL;
	pSourceSignal = pSignal;
	if (pSourceSignal != NULL && pFilter != NULL)
	{
		if (pFilter->Flags(FILTER_COMPLEX) ||
			pSourceSignal->Flags(SIGNAL_COMPLEX))
		{
			pInSignal = new CSignalArray<fcomplex>;
		}
		else
		{
			pInSignal = new CSignalArray<float>;
		}
	}
	iSamplesAvailable = 0;
}

#ifdef _DEBUG
void CSignal::Dump(CDumpContext & dc)
{
	// no action
}

void CSignal_Filtered::Dump(CDumpContext & dc)
{
	pFilter->Dump(dc);
}

void CFilterband::Dump(CDumpContext & dc)
{
	CString s;
	s.Format("Low Freq = %f Hz\n"
			"High Freq = %f Hz\n"
			"Pass Loss = %f dB\n"
			"Stop Loss = %f db\n"
			"Transition width = %f\n",
			dLowFreq, dHighFreq, dPassLoss, dStopLoss,
			dTransitionWidth);
	dc << s;
	CSignal_Filtered::Dump(dc);
}
#endif
