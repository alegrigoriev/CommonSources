// filter.h
#ifndef __FILTER_H
#define __FILTER_H

#if !defined(_INC_POLYRATIO)
#include "PolyRatio.h"
#endif
#include <afxtempl.h>
#include <float.h>

#ifndef DIGITAL_SIGNAL_ARRAY__H__
#include <DigitalSignalArray.h>
#endif

#ifndef PIANOSTRING_H_
#include "PianoString.h"
#endif

#pragma pack(push, 8)

#ifdef USE_LONGDOUBLE
	#define POLY_ROOTS lpolyRoots
	#define POLY_RATIO lpolyRatio
	#define POLY lpoly
	#define COMPLEX lcomplex
	#define REAL ldouble
	#define COMPLEX_ARRAY lcomplexArray
#else
	#define POLY_ROOTS polyRoots
	#define POLY_RATIO polyRatio
	#define POLY poly
	#define COMPLEX Complex
	#define REAL double
	#define COMPLEX_ARRAY complexArray
#endif

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
	const POLY_ROOTS & Zeros() const { return m_Zeros; }
	POLY_ROOTS & Zeros() { return m_Zeros; }

	const POLY_ROOTS & Poles() const { return m_Poles; }
	POLY_ROOTS & Poles() { return m_Poles; }

	const POLY_RATIO & Deriv() const;
	const POLY_RATIO & Ratio() const;

	CFilterCell & operator = (const CFilterCell &);
	CFilterCell & operator = (const polyRatio &);

	DWORD Flags(DWORD dwMask = 0xffffffff) const
	{ return dwFlags & dwMask; }
	// operations
public:
	BOOL MakePoles();
	BOOL MakeZeros();
	void SetCoeffsPrecision(int iBits);

	COMPLEX FreqResponce(double f) const;
	COMPLEX FreqResponce(Complex z) const
	{ return m_Ratio.eval(z); }
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
	const POLY_ROOTS & Zeros() const { return m_Zeros; }
	POLY_ROOTS & Zeros() { return m_Zeros; }
	const POLY_ROOTS & Poles() const { return m_Poles; }
	POLY_ROOTS & Poles() { return m_Poles; }
	int RatioCount() const { return m_aRatios.GetSize(); }
	const POLY_RATIO & PolyRatio(int n) const
	{ return *(m_aRatios[n]); }
	POLY_RATIO & PolyRatio(int n)
	{ return *(m_aRatios[n]); }
	DWORD Flags(DWORD dwMask = 0xffffffff) const
	{ return dwFlags & dwMask; }
	DWORD ModifyFlags(DWORD FlagsToSet,
					DWORD FlagsToReset = 0)
	{
		return dwFlags = FlagsToSet | (dwFlags & ~FlagsToReset);
	}
	// operations
public:
	POLY_RATIO GetCanonical();
	void MakeCanonical();
	BOOL MakePoles();
	BOOL MakeZeros();
	void Decompose();
	BOOL RemoveRatio(int iIndex);
	BOOL InsertRatio(const POLY_RATIO & pr);
	void SetCoeffsPrecision(int iBits);

	COMPLEX FreqResponce(double f) const;
	REAL GroupDelay(double f) const;

	int TemporalResponce(CSignalStoreIterator * dst,
						CSignalIterator * src, int nCount);
	int TemporalResponce(COMPLEX * dst, const COMPLEX * src, int iCount);
	// TrailLength - Find number of samples when the signal will fade to
	// the specified amount of decibels
	int TrailLength(double dDecay = 90. /* dB */) const;
	COMPLEX operator ()(double f) const { return FreqResponce(f); }
	// implementation
#ifdef _DEBUG
	void Dump(CDumpContext & = afxDump);
#endif

public:
	double dSamplingRate;
	double dCenterFreq;

	BOOL CreateStringReflectorFilter(NewFilterData * pFD,
									CPianoString& PianoString = CPianoString());
protected:
	POLY_ROOTS m_Zeros;
	POLY_ROOTS m_Poles;
	int iPrecision;
	CArray<POLY_RATIO *, POLY_RATIO *> m_aRatios;
	POLY_RATIO m_prCanonical;
	CArray<POLY_RATIO *, POLY_RATIO *> m_aDerivRatios;

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

#pragma pack (pop)

#endif // #ifndef __FILTER_H
