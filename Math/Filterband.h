// Filterband.h
#ifndef __FILTERBAND_H__
#define __FILTERBAND_H__

#ifndef __DIGITAL_SIGNAL_H__
#include "DigitalSignal.h"
#endif

class CFilterband : public CSignal_Filtered
{
public:
	int nDecimationRate;
	double dLowFreq;
	double dHighFreq;
	double dTransitionWidth;
	double dPassLoss;
	double dStopLoss;
	BOOL CreateFilter(int bFIR = FILTER_CLASS_IIR);
	CFilterband()
		:dLowFreq(10.),
		dHighFreq(11.),
		dTransitionWidth(0.1),
		dPassLoss(1.),
		dStopLoss(60.),
		d0dBLevel(1.),
		dNormCoeff(1.),
		pFreqArray(NULL),
		pLevelDBArray(NULL),
		pAmplArray(NULL),
		bFreqArrayValid(FALSE),
		bLevelDBArrayValid(FALSE),
		bAmplArrayValid(FALSE),
		nDecimationRate(1)
	{
	}
	~CFilterband();
	// get filtered signal
	virtual const complex * GetSignal(int iStart, int iLength);
	// get level of the filtered signal
	const float * GetAmplitude(int iStart, int iLength);
	// get level of the filtered signal in DB
	const float * GetLevelDB(int iStart, int iLength);
	// get instaneous frequency of the filtered signal
	const float * GetFrequency(int iStart, int iLength);
	// set signal level corresponding 0 dB
	void Set0dBLevel(double level);    // set 0 dB level
	double Get0dBLevel() { return d0dBLevel; }

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

#endif  //#ifndef __FILTERBAND_H__
