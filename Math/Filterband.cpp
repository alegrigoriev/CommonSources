// Filterband.cpp
#include "DigitalSignal.h"

CFilterband::CFilterband()
	:dLowFreq(10.),
	dHighFreq(11.),
	dTransitionWidth(0.1),
	dPassLoss(.5),
	dStopLoss(80.),
	d0dBLevel(16384.),
	dNormCoeff(1./16384.),
	pFreqArray(NULL),
	pLevelDBArray(NULL),
	pAmplArray(NULL),
	bFreqArrayValid(FALSE),
	bLevelDBArrayValid(FALSE),
	bAmplArrayValid(FALSE)
{
}

CFilterband::~CFilterband()
{
	delete[] pFreqArray;
	delete[] pLevelDBArray;
	delete[] pAmplArray;
	delete pFilter;
}

BOOL CFilterband::CreateFilter(int bFIR)
{
	NewFilterData nfd;
	nfd.iFilterType =    FILTER_BANDPASS;
	nfd.iCoeffType =    FILTER_COEFF_COMPLEX;
	nfd.iFilterClass =    bFIR;
	nfd.iFirType  =  FILTER_FIR_ZEROPHASE;
	nfd.iIirType  =  FILTER_IIR_ELLIPTIC;
	nfd.bPowerSymm  =  FALSE;
	nfd.iOrder =   0;
	nfd.dSamplingRate  =  dSamplingRate;
	nfd.dPassLoss  =  dPassLoss;
	nfd.dStopLoss  =  dStopLoss;
	nfd.dCenterFreq  =  (dHighFreq + dLowFreq) * 0.5;
	nfd.dOuterFreqHigh  =  0;
	nfd.dOuterFreqLow  =  0;
	nfd.dInnerFreqHigh  =  0;
	nfd.dInnerFreqLow  =  0;
	nfd.dOuterBandWidth  =  0;
	nfd.dInnerBandWidth =  0;
	nfd.dLowFreq  =  (dHighFreq - dLowFreq);    // band width
	nfd.dHighFreq =  nfd.dLowFreq * (1 + dTransitionWidth);
	if (0 && nfd.dLowFreq / dSamplingRate < 0.05)
	{
		nfd.bPowerSymm = TRUE;
	}

	CDigitalFilter * pNewFilter = new CDigitalFilter;
	if (pNewFilter != NULL && pNewFilter->Create(& nfd))
	{
		delete pFilter;
		SetFilter(pNewFilter);
		bValid = FALSE;
		nLevelSamplesAvailable = 0;
		nAmplSamplesAvailable = 0;
		nFreqSamplesAvailable = 0;
		return TRUE;
	}
	else
	{
		delete pNewFilter;
		return FALSE;
	}
}

void CFilterband::Set0dBLevel(double level)
{
	ASSERT(level != 0.);
	if (0. == level) return;

	d0dBLevel = level;
	dNormCoeff = 1. / (level * level);
	nLevelSamplesAvailable = 0; // invalidate level array contents
}

#if 0
const Complex * CFilterband::GetSignal(int iStart, int iLength)
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
		pInSignal->SetRetrieveIndex(iStart - iFirstSample)
			return pInSignal;
	}

	// we need some amount of source signal before
	// starting point - TrailLength. It depends on
	// the filter technology
	pInSignal->Allocate(iReqLength);

	const Complex * pSrc =
		pSourceSignal->GetSignal(iReqBegin, iReqLength);
	pFilter->TemporalResponce(*pInSignal, pSrc, iReqLength);

	iFirstSample = iReqBegin;
	iSamplesAvailable = iReqLength;
	bValid = TRUE;

	return iStart - iFirstSample + (Complex*)*pInSignal;
}
#endif
	// get instaneous frequency of the filtered signal
const float * CFilterband::GetFrequency(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iLastSample = iStart + iLength;

	int iReqBegin = iStart;
	if (iReqBegin < 0) iReqBegin = 0;

	int iReqLength = iLastSample - iReqBegin;

	if (IsValid() && bFreqArrayValid && iReqBegin >= nFirstFreqSample
		&& iLastSample <= nFreqSamplesAvailable + nFirstFreqSample)
	{
		return pFreqArray + iStart - nFirstFreqSample;
	}

	if (FALSE == bFreqArrayValid || iReqLength > nFreqArraySize)
	{
		delete[] pFreqArray;
		pFreqArray = NULL;
		nFreqArraySize = 0;
		bFreqArrayValid = FALSE;
		pFreqArray = new float[iReqLength];

		if (NULL == pFreqArray)
		{
			return NULL;
		}
		nFreqArraySize = iReqLength;
	}

	CSignalIterator * pSrc =
		GetSignal(iReqBegin, iReqLength + 1);
	SamplingRate();

	Complex prev = pSrc->GetNextComplex();
	for (int i = 0; i < iReqLength; i++, pSrc++)
	{
		Complex curr = pSrc->GetNextComplex();
		double frequency = dSamplingRate
							/ (2. * M_PI)
							* arg(curr * conj(prev));
		pFreqArray[i] = float(frequency);
		prev = curr;
	}
	nFirstFreqSample = iReqBegin;
	nFreqSamplesAvailable = iReqLength;
	bFreqArrayValid = TRUE;

	return pFreqArray + iStart - nFirstFreqSample;
}

	// get instaneous amplitude of the filtered signal
const float * CFilterband::GetAmplitude(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iLastSample = iStart + iLength;

	int iReqBegin = iStart;
	if (iReqBegin < 0) iReqBegin = 0;

	int iReqLength = iLastSample - iReqBegin;

	if (IsValid() && bAmplArrayValid && iReqBegin >= nFirstAmplSample
		&& iLastSample <= nAmplSamplesAvailable + nFirstAmplSample)
	{
		return pAmplArray + iStart - nFirstAmplSample;
	}

	if (FALSE == bAmplArrayValid || iReqLength > nAmplArraySize)
	{
		delete[] pAmplArray;
		pAmplArray = NULL;
		nAmplArraySize = 0;
		bAmplArrayValid = FALSE;
		pAmplArray = new float[iReqLength];

		if (NULL == pAmplArray)
		{
			return NULL;
		}
		nAmplArraySize = iReqLength;
	}

	CSignalIterator * pSrc = GetSignal(iReqBegin, iReqLength);

	SamplingRate();
	for (int i = 0; i < iReqLength; i++)
	{
		pAmplArray[i] = (float) abs(pSrc->GetNextComplex());
	}
	nFirstAmplSample = iReqBegin;
	nAmplSamplesAvailable = iReqLength;
	bAmplArrayValid = TRUE;

	return pAmplArray + iStart - nFirstAmplSample;
}

	// get instaneous level (in dB) of the filtered signal
const float * CFilterband::GetLevelDB(int iStart, int iLength)
{
	// make sure that there are enough samples in the buffer
	int iLastSample = iStart + iLength;

	int iReqBegin = iStart;
	if (iReqBegin < 0) iReqBegin = 0;

	int iReqLength = iLastSample - iReqBegin;

	if (IsValid() && bLevelDBArrayValid && iReqBegin >= nFirstLevelSample
		&& iLastSample <= nLevelSamplesAvailable + nFirstLevelSample)
	{
		return pLevelDBArray + iStart - nFirstLevelSample;
	}

	if (FALSE == bLevelDBArrayValid || iReqLength > nLevelArraySize)
	{
		delete[] pLevelDBArray;
		pLevelDBArray = NULL;
		nLevelArraySize = 0;
		bLevelDBArrayValid = FALSE;
		pLevelDBArray = new float[iReqLength];

		if (NULL == pLevelDBArray)
		{
			return NULL;
		}
		nLevelArraySize = iReqLength;
	}

	pSourceSignal->dSamplingRate = dSamplingRate;
	CSignalIterator * pSrc = GetSignal(iReqBegin, iReqLength);

	for (int i = 0; i < iReqLength; i++)
	{
		Complex ctmp = pSrc->GetNextComplex();
		double tmp = ctmp.real() * ctmp.real() + ctmp.imag() * ctmp.imag();
		if (tmp != 0.)
		{
			pLevelDBArray[i] =
				float(10. * log10(tmp * dNormCoeff));
		}
	}
	nFirstLevelSample = iReqBegin;
	nLevelSamplesAvailable = iReqLength;
	bLevelDBArrayValid = TRUE;

	return pLevelDBArray + iStart - nFirstLevelSample;
}
