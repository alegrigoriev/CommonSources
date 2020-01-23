#pragma once

#define _USE_MATH_DEFINES   // for M_PI definition
#include <math.h>
#include <complex>
#include "PolyMath.h"
#include "PolyRatio.h"

#define TWO_PI (M_PI*2.)

int LaguerreMethod(poly& coef,
					Complex* zz,
					double epsilon,
					double epsilon2,
					int maxIterations);

void UnwrapPhase( int ix, double *phase);

void EllipticPolesZeros(double omegaPass,
						double omegaStop,
						double minStopLossDB,
						double maxPassLossDB,
						int order,
						polyRoots& zeros,
						polyRoots& poles,
						Complex& rNormCoeff);

void TwoAllpassDecompose(const polyRoots& poles,
						double T,
						poly& denom1,
						poly& numer1,
						poly& denom2,
						poly& numer2,
						double angle = 0.);

void TwoAllpassPassbandDecompose(const polyRoots& poles,
								double W0,
								double T,
								poly& denom1,
								poly& numer1,
								poly& denom2,
								poly& numer2,
								polyRoots& ZPlanePoles);


void BilinearLowPass(const polyRoots& SrcPoles,
					const polyRoots& SrcZeros,
					double T,
					polyRoots& ZPlanePoles,
					polyRoots& ZPlaneZeros,
					Complex rotator = Complex(1., 0.));

void BilinearTransform(const poly & src, poly & dst,
						double T, Complex rotator, int nAddZeros);

void BilinearTransform(const polyRatio & src, polyRatio & dst,
						double T, Complex rotator = 1.);

Complex BilinearNormCoeff(const polyRoots& SrcPoles,
						const polyRoots& SrcZeros,
						double T, Complex NormCoeff);



void LowpassToBandpass(const polyRoots& SrcPoles,
						const polyRoots& SrcZeros,
						double W0, // center frequency
						double T,
						polyRoots& ZPlanePoles,
						poly& Denom);

void EllipticPassbandPolesZeros(polyRoots& poles,
								polyRoots& zeros,
								int order,
								int bilinear,
								double T,
								double PassFreqLow,
								double PassFreqHigh,
								double StopFreqLow,
								double StopFreqHigh,
								double &CenterFreq,
								double maxPassLossDB,
								double minStopLossDB);

void HilbertTwoAllpassDecompose(const polyRoots& poles,
								poly& denom1,
								poly& numer1,
								poly& denom2,
								poly& numer2);

void EllipticHilbertPoles(double omegaPass,
						double &minStopLossDB,
						double &maxPassLossDB, // 0 - power-symmetric filter
						int order,
						polyRoots& poles);

void Allpass2Canonical(poly& numer,
						poly& denom,
						const poly& numer1,
						const poly& denom1,
						const poly& numer2,
						const poly& denom2);

int EstimateChebyshev2FilterOrder(double w_pass, double passband_attenuation, double w_stop, double stopband_attenuation);

//////////////////////////////////////////////////////////////////
//////////////// Filter
enum { MaxFilterOrder = 16, };
class LowpassFilter
{
public:
	LowpassFilter()
		: m_Order(0),
		m_ZeroPhase(FALSE)
	{
		Reset();
	}

	void Reset();
	int GetFilterOrder() const
	{
		return m_Order;
	}
	void SetZeroPhase(bool ZeroPhase)
	{
		m_ZeroPhase = ZeroPhase;
	}
	std::complex<double> CalculateResponse(std::complex<double> z) const;
	BOOL CreateElliptic(double PassFreq, double PassLoss,
						double StopFreq, double StopLoss);
	void GetCoefficients(double Coeffs[MaxFilterOrder][6]) const;

	bool	m_ZeroPhase;
	int     m_Order;    // low pass filter order
	double  m_Coeffs[MaxFilterOrder][6];
};

class HighpassFilter
{
public:
	HighpassFilter()
		: m_Order(0),
		m_ZeroPhase(FALSE)
	{
		Reset();
	}

	void Reset();
	int GetFilterOrder() const
	{
		return m_Order;
	}
	void SetZeroPhase(bool ZeroPhase)
	{
		m_ZeroPhase = ZeroPhase;
	}
	std::complex<double> CalculateResponse(std::complex<double> z) const;
	BOOL CreateElliptic(double PassFreq, double PassLoss,
						double StopFreq, double StopLoss);
	void GetCoefficients(double Coeffs[MaxFilterOrder][6]) const;

	BOOL	m_ZeroPhase;
	int     m_Order;    // low pass filter order
	double  m_Coeffs[MaxFilterOrder][6];
};

class NotchFilter
{
public:
	NotchFilter()
		: m_Order(0),
		m_ZeroPhase(FALSE)
	{
		Reset();
	}

	void Reset();
	int GetFilterOrder() const
	{
		return m_Order;
	}
	void SetZeroPhase(bool ZeroPhase)
	{
		m_ZeroPhase = ZeroPhase;
	}
	std::complex<double> CalculateResponse(std::complex<double> z) const;
	void Create(double PassFreq, double StopFreq);
	void GetCoefficients(double Coeffs[MaxFilterOrder][6]) const;

	bool	m_ZeroPhase;
	int     m_Order;    // low pass filter order
	double  m_Coeffs[MaxFilterOrder][6];
};

class HilbertTransformFilter
{
public:
	HilbertTransformFilter()
		: m_Order(0)
	{
		Reset();
	}

	void Reset();
	int GetFilterOrder() const
	{
		return m_Order;
	}

	std::complex<double> CalculateResponse(std::complex<double> /*z*/) const
	{
		return 1.;
	}
	BOOL CreateElliptic(double PassFreq, double PassLoss,
						double StopFreq, double StopLoss);
	void GetCoefficients(double Coeffs[MaxFilterOrder][6]) const;

	int     m_Order;    // low pass filter order
	double  m_Coeffs[MaxFilterOrder][6];
};
