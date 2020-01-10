// Signal.h
#ifndef __DIGITAL_SIGNAL_H__
#define __DIGITAL_SIGNAL_H__


#ifndef __FILTER_H
#include "DigitalFilter.h"
#endif
#include "mmsystem.h"

#include "DigitalSignalArray.h"

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

#pragma pack(push, 8)

struct CSignal
{
	CSignal()
		:pInSignal(NULL),
		dSamplingRate(1.), iFirstSample(INT_MAX),
		iSamplesAvailable(0), bValid(TRUE)
	{
	}

	virtual ~CSignal()
	{
		delete pInSignal;
		pInSignal = NULL;
	}

	int iFirstSample;
	int iSamplesAvailable;
	virtual CSignalIterator * GetSignal(int iStart, int iLength) = 0;
	virtual double GetDelay(double dFrequency) const;
	void Reset()
	{
		iFirstSample = INT_MAX;
		iSamplesAvailable = 0;
	}

	double dSamplingRate;
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext & = afxDump);
#endif
	virtual BOOL IsValid();
	void Invalidate() { bValid = FALSE; }
	virtual double SignalLength();
	virtual double SamplingRate();
	DWORD Flags(DWORD mask=0xFFFFFFFF)
	{
		ASSERT(pInSignal != NULL);
		return pInSignal->Flags(mask);
	}
protected:
	CSignalArrayBase * pInSignal;
	BOOL bValid;
};

struct CSignal_Step : CSignal
{
	CSignal_Step()
	{
		pInSignal = new CSignalArray<float>;
	}
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
};

struct CSignal_Pulse : CSignal
{
	CSignal_Pulse()
	{
		pInSignal = new CSignalArray<float>;
	}
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
};

struct CSignal_Triangle : CSignal
{
	double dSignalLength;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	CSignal_Triangle(double length)
		:dSignalLength(length)
	{
		pInSignal = new CSignalArray<float>;
	}
};

struct CSignal_Square : CSignal
{
	double dSignalLength;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	CSignal_Square(double length)
		:dSignalLength(length)
	{
		pInSignal = new CSignalArray<float>;
	}
};

struct CSignal_Sine : CSignal
{
	double dFrequency;
	double dSignalLength;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	CSignal_Sine(double freq = 0.5, double SignalLength = 1E6)
		:dFrequency(freq),
		dSignalLength(SignalLength)
	{
		pInSignal = new CSignalArray<float>;
	}
};

struct CSignal_FM : CSignal
{
	double dFrequency;
	double dModFrequency;
	double dDeviation;  // frequency deviation
	double dSignalLength;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	CSignal_FM(double freq = 0.5,
				double ModFrequency = 0.1,
				double Deviation = 0.1,
				double SignalLength = 1E6)
		:dFrequency(freq),
		dModFrequency(ModFrequency),
		dDeviation(Deviation),
		dSignalLength(SignalLength)
	{
		pInSignal = new CSignalArray<float>;
	}
};

struct CSignal_LFM : CSignal
{
	double dStartFrequency;
	double dEndFrequency;
	double dSignalLength;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	CSignal_LFM(double StartFrequency = 0.1,
				double EndFrequency = 0.1,
				double SignalLength = 1e6)

		:dStartFrequency(StartFrequency),
		dEndFrequency(EndFrequency),
		dSignalLength(SignalLength)
	{
		pInSignal = new CSignalArray<float>;
	}
};

struct CSignal_Filtered : CSignal
{
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	virtual double GetDelay(double dFrequency) const;
	CSignal_Filtered(CDigitalFilter * filter = NULL,
					CSignal * signal = NULL)
		:pFilter(filter), pSourceSignal(signal)
	{
	}
	virtual double SignalLength();
	virtual double SamplingRate();
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext & = afxDump);
#endif
	virtual BOOL IsValid();
	void SetFilter(CDigitalFilter * pSourceFilter);
	void SetSourceSignal(CSignal * pSignal);
//    BOOL CreateFilter(NewFilterData * pNfd)
//        {
//        return pFilter->Create(pNfd);
//        }
protected:
	CDigitalFilter * pFilter;
	CSignal * pSourceSignal;
};

struct CSignalTransparent : CSignal
{
	CSignal * pSourceSignal;
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	virtual double GetDelay(double dFrequency) const;
	CSignalTransparent()
		:pSourceSignal(NULL)
	{
	}
	virtual double SignalLength();
	virtual double SamplingRate();
public:
	virtual BOOL IsValid();
};

class CFilterband;
struct CSignalDecimator;

struct CSignalWave : CSignal
{
	CSignalWave();
	~CSignalWave();
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	BOOL Open(LPCTSTR szFilename);
	void Close();
	// attributes
	int GetSamplesCount() const { return nWaveSamples; }
	int GetNumberOfChannels() const { return wfFormat.wf.nChannels; }
	void SetChannel(int);
	int GetChannel() const { return nCurrChannel; }
	virtual double SignalLength();
	bool IsOpen() const
	{
		return m_IsOpen;
	}
	CFilterband * CreateFilterband(double dLowFreq, double dHighFreq);

private:
	CFile m_File;
	bool m_IsOpen;
	HANDLE hMemoryMapping;
	LPVOID pFileBase;
	LPVOID pDataAddress;
	DWORD nTotalSize;
	int nWaveSamples;
	int nCurrChannel;
	CArray<CSignalDecimator *, CSignalDecimator *>
		aDecimators;
	CSignalArray<float> OutSignal;
	DWORD dwFlags;
	PCMWAVEFORMAT  wfFormat;           // "FMT" chunk

public:
	void CreateDecimators(int nCount = 8);
};

struct CSignalDecimator : CSignal
{
	int nDecimFactor;
	double dBandwidth;
	CSignalDecimator();
	~CSignalDecimator();
	virtual CSignalIterator * GetSignal(int iStart, int iLength);
	virtual double GetDelay(double dFrequency) const;
	void Init(int decim_factor = 2,
			double dSafetyBand = 0.9,
			double dPassbandLossDB = 0.5,
			double dStopbandLossDB = 70.);
	virtual double SignalLength();
	virtual double SamplingRate();
public:
	virtual BOOL IsValid();
	void SetSourceSignal(CSignal * pSourceSignal);
protected:
	CSignalTransparent InSignal;
	CSignal_Filtered DecimFilter;
	CDigitalFilter * pFilter;
};

class CFilterband : public CSignal_Filtered
{
public:
	double dLowFreq;
	double dHighFreq;
	double dTransitionWidth;
	double dPassLoss;
	double dStopLoss;
	BOOL CreateFilter(int bFIR = FILTER_CLASS_IIR);
	CFilterband();
	~CFilterband();
	// get filtered signal
	//virtual CSignalIterator * GetSignal(int iStart, int iLength);
	// get level of the filtered signal
	const float * GetAmplitude(int iStart, int iLength);
	double GetAmplitude(int nSample);
	// get level of the filtered signal in DB
	const float * GetLevelDB(int iStart, int iLength);
	double GetLevelDB(int nSample);
	// get instaneous frequency of the filtered signal
	const float * GetFrequency(int iStart, int iLength);
	double GetFrequency(int nSample);
	// set signal level corresponding 0 dB
	void Set0dBLevel(double level);    // set 0 dB level
	double Get0dBLevel() const { return d0dBLevel; }
#ifdef _DEBUG
	virtual void Dump(CDumpContext & = afxDump);
#endif

protected:
	// array of instaneous frequencies
	float * pFreqArray;
	int nFirstFreqSample;
	int nFreqSamplesAvailable;
	int nFreqArraySize;
	BOOL bFreqArrayValid;


	// array of levels in dB
	float * pLevelDBArray;
	int nFirstLevelSample;
	int nLevelSamplesAvailable;
	int nLevelArraySize;
	BOOL bLevelDBArrayValid;
	double d0dBLevel;   // reference level 0 dB (amplitude)
	double dNormCoeff; // 1/d0dBLevel

	// array of signal amplitude
	float * pAmplArray;    // signal amplitude
	int nFirstAmplSample;
	int nAmplSamplesAvailable;
	int nAmplArraySize;
	BOOL bAmplArrayValid;
};
#pragma pack(pop)

inline double CFilterband::GetAmplitude(int nSample)
{
	// make sure that there are enough samples in the buffer
	if (nSample >= iFirstSample
		&& nSample < iSamplesAvailable + iFirstSample)
	{
		if(pInSignal->Flags(SIGNAL_COMPLEX))
		{
			return abs(pInSignal->GetComplexAt(nSample - iFirstSample));
		}
		else
		{
			return pInSignal->GetDoubleAt(nSample - iFirstSample);
		}
	}
	else
	{
		return 0.;
	}
}

inline double CFilterband::GetLevelDB(int nSample)
{
	// make sure that there are enough samples in the buffer
	if (nSample >= iFirstSample
		&& nSample < iSamplesAvailable + iFirstSample)
	{
		if(pInSignal->Flags(SIGNAL_COMPLEX))
		{
			Complex tmp = pInSignal->GetComplexAt(nSample - iFirstSample);
			double level = dNormCoeff * (tmp.real() * tmp.real() + tmp.imag() * tmp.imag());
			if (level != 0.)
			{
				return 10. * log10(level);
			}
			else
				return -100.;
		}
		else
		{
			double level = sqrt(dNormCoeff) * fabs(pInSignal->GetDoubleAt(nSample - iFirstSample));
			if (level != 0.)
			{
				return 20. * log10(level);
			}
			else
				return -100.;
		}
	}
	else
	{
		return -100.;
	}
}

inline double CFilterband::GetFrequency(int nSample)
{
	// make sure that there are enough samples in the buffer
	if (nSample >= iFirstSample
		&& nSample < iSamplesAvailable + iFirstSample - 1
		&& pInSignal->Flags(SIGNAL_COMPLEX))
	{
		Complex rotator = pInSignal->GetComplexAt(nSample - iFirstSample + 1)
						* conj(pInSignal->GetComplexAt(nSample - iFirstSample));
		return dSamplingRate / (2. * M_PI) * arg(rotator);
	}
	else
	{
		return 0.;
	}
}

#endif //#ifndef __DIGITAL_SIGNAL_H__
