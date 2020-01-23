// DigitalFilter.h
#pragma once

#include "PolyRatio.h"

#include <afxtempl.h>
#define _USE_MATH_DEFINES   // for M_PI definition
#include <math.h>
#include <float.h>

#include "DigitalSignalArray.h"

#define MAX_FILTER_ORDER    255
#define FILTER_COMPLEX          0x00000001
#define FILTER_ALLPASS          0x00000002
#define FILTER_ZEROS_KNOWN      0x00000004
#define FILTER_POLES_KNOWN      0x00000008
#define FILTER_IIR              0x00000010
#define FILTER_DECOMPOSABLE     0x00000020
#define FILTER_CREATED          0x00000040
#define FILTER_CANONICAL_KNOWN  0x00000080
#define FILTER_DERIV_KNOWN      0x00000100
#define FILTER_FROM_ZEROS       0x00000200
#define FILTER_FROM_POLES       0x00000400

#define FILTER_CLASS_FIR  0
#define FILTER_CLASS_IIR  1

#define FILTER_COEFF_REAL 0
#define FILTER_COEFF_COMPLEX 1

#define FILTER_IIR_ELLIPTIC    0
#define FILTER_IIR_CHEBYCHEV   1
#define FILTER_IIR_BUTTERWORTH 2
#define FILTER_IIR_BESSEL      3

#define FILTER_FIR_ZEROPHASE 0
#define FILTER_FIR_MINPHASE  1
#define FILTER_FIR_MAXPHASE  2

#define FILTER_LOWPASS 0
#define FILTER_HIGHPASS 1
#define FILTER_BANDPASS 2
#define FILTER_BANDSTOP 3
#define FILTER_HILBERT  4
#define FILTER_SHIFTER  5       // only FIR
#define FILTER_PARTIAL_DELAY 6   // only FIR
#define FILTER_STRING_REFLECTOR 7   // only FIR

struct NewFilterData
{
	int iFilterType;    // low pass, high pass, etc
	int iCoeffType; // 0 - real, 1 - image
	int iFilterClass;   // 0 - FIR, 1 - IIR
	int iFirType;   // 0 - zero phase, 1 - min phase, 2 - max phase
	int iIirType;   // 0 - elliptic, 1 - chebychev, 2 - butterworth
	BOOL bPowerSymm;
	int iOrder;
	double dSamplingRate;
	double dLowFreq;
	double dHighFreq;
	double dPassLoss;
	double dStopLoss;
	double dCenterFreq;
	double dOuterFreqHigh;
	double dOuterFreqLow;
	double dInnerFreqHigh;
	double dInnerFreqLow;
	double dOuterBandWidth;
	double dInnerBandWidth;
	double dDelay;      // for time shifter
	NewFilterData()
		:iFilterType(0),
		iCoeffType(0),
		iFilterClass(0),
		iFirType(0),
		iIirType(0),
		bPowerSymm(0),
		iOrder(0),
		dSamplingRate(44100),
		dLowFreq(DBL_MIN),
		dPassLoss(DBL_MIN),
		dStopLoss(DBL_MIN),
		dCenterFreq(11050),
		dOuterFreqHigh(DBL_MIN),
		dOuterFreqLow(DBL_MIN),
		dInnerFreqHigh(DBL_MIN),
		dInnerFreqLow(DBL_MIN),
		dOuterBandWidth(DBL_MIN),
		dInnerBandWidth(DBL_MIN),
		dDelay(DBL_MIN)
	{
	}
	BOOL IsIIR() const { return iFilterClass; }
	BOOL IsComplex() const { return iCoeffType; }
	void RescaleFrequencies(double);
};

// class for elementary cell in A(z)/B(Z) form
class CPianoString;
class CFilterCell
{
public:
	//constructors
	CFilterCell(const CFilterCell &);
	CFilterCell(const polyRatio &);
	CFilterCell(const poly & numer,const poly & denom);
	CFilterCell();
	CFilterCell(Complex *zeros, int nOrder=0,
				Complex *poles=NULL, int dOrder=0);
	//attributes

	BOOL IsFir() const { return m_Ratio.DenomOrder() != 0; }
	BOOL IsStable() const;
	// data access
	const polyRoots& Zeros() const { return m_Zeros; }
	polyRoots& Zeros() { return m_Zeros; }

	const polyRoots& Poles() const { return m_Poles; }
	polyRoots& Poles() { return m_Poles; }

	const polyRatio& Deriv() const;
	const polyRatio& Ratio() const;

	CFilterCell & operator = (const CFilterCell &);
	CFilterCell & operator = (const polyRatio &);

	DWORD Flags(DWORD dwMask = 0xffffffff) const
	{ return dwFlags & dwMask; }
	// operations
public:
	BOOL MakePoles();
	BOOL MakeZeros();
	void SetCoeffsPrecision(int iBits);

	Complex FreqResponce(double f) const;
	Complex FreqResponce(Complex z) const
	{
		return m_Ratio.eval(z);
	}
	//void InitPrevSamples(const Complex * pPrevBuf);
	//void NextOutputSample(const Complex & InSample);

	// implementation
protected:
	polyRatio m_Ratio;
	polyRatio m_Deriv;    // first derivative for group delay calc
	polyRoots m_Poles;
	polyRoots m_Zeros;
	DWORD dwFlags;
	Complex * pTmpBuf;  // is used to keep intermediate data
};

class CDigitalFilter
{
public:
	CDigitalFilter()
		:iPrecision(0),
		dSamplingRate(44100),
		dwFlags(0), dCenterFreq(0.)
	{
	}
	CDigitalFilter(NewFilterData *pNf)
		:iPrecision(0),
		dSamplingRate(44100),
		dwFlags(0), dCenterFreq(0.)
	{
		Create(pNf);
	}
	~CDigitalFilter();
	BOOL Create(NewFilterData *);
	// attributes
public:
	int NumerOrder() const
	{ return m_prCanonical.NumerOrder(); }
	int DenomOrder() const
	{ return m_prCanonical.DenomOrder(); }
	BOOL IsFir() const { return !(dwFlags & FILTER_IIR); }
	BOOL IsStable() const;
	const polyRoots& Zeros() const { return m_Zeros; }
	polyRoots& Zeros() { return m_Zeros; }
	const polyRoots& Poles() const { return m_Poles; }
	polyRoots& Poles() { return m_Poles; }
	int RatioCount() const { return (int)m_aRatios.GetSize(); }
	const polyRatio& PolyRatio(int n) const
	{
		return *(m_aRatios[n]);
	}
	polyRatio& PolyRatio(int n)
	{
		return *(m_aRatios[n]);
	}
	DWORD Flags(DWORD dwMask = 0xffffffff) const
	{ return dwFlags & dwMask; }
	DWORD ModifyFlags(DWORD FlagsToSet,
					DWORD FlagsToReset = 0)
	{
		return dwFlags = FlagsToSet | (dwFlags & ~FlagsToReset);
	}
	// operations
public:
	polyRatio GetCanonical();
	void MakeCanonical();
	BOOL MakePoles();
	BOOL MakeZeros();
	void Decompose();
	BOOL RemoveRatio(int iIndex);
	BOOL InsertRatio(const polyRatio& pr);
	void SetCoeffsPrecision(int iBits);

	Complex FreqResponce(double f) const;
	double GroupDelay(double f) const;

	int TemporalResponce(CSignalStoreIterator* dst,
						CSignalIterator* src, int nCount);
	int TemporalResponce(Complex* dst, const Complex* src, int iCount);
	// TrailLength - Find number of samples when the signal will fade to
	// the specified amount of decibels
	int TrailLength(double dDecay = 90. /* dB */) const;
	Complex operator ()(double f) const { return FreqResponce(f); }
	// implementation
#ifdef _DEBUG
	void Dump(CDumpContext & = afxDump);
#endif

public:
	double dSamplingRate;
	double dCenterFreq;

#ifdef PIANOSTRING_H_
	BOOL CreateStringReflectorFilter(NewFilterData * pFD,
									CPianoString& PianoString = CPianoString());
#endif
protected:
	polyRoots m_Zeros;
	polyRoots m_Poles;
	int iPrecision;
	CArray<polyRatio*, polyRatio*> m_aRatios;
	polyRatio m_prCanonical;
	CArray<polyRatio*, polyRatio*> m_aDerivRatios;

	DWORD dwFlags;

	BOOL CreateFir(NewFilterData * pFD);
	BOOL CreateIir(NewFilterData * pFD);
	BOOL CreateEllipticFilter(NewFilterData * pFD);
	BOOL CreateLowpassElliptic(NewFilterData * pFD);
	BOOL CreateHighpassElliptic(NewFilterData * pFD);
	BOOL CreateBandpassElliptic(NewFilterData * pFD);
	BOOL CreateHilbertElliptic(NewFilterData * pFD);
	BOOL CreateShifter(NewFilterData * pFD);
	BOOL CreatePartialDelayFilter(NewFilterData * pFD);
	BOOL CreateFirZerophase(NewFilterData * pFD);
	//BOOL AllocRatios(int iNumRatios);

	// free all data:
	void Reset();
};
