// Filter.cpp

#include "DigitalFilter.h"
#include "FilterMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDigitalFilter::Create(NewFilterData * pFD)
{
	switch (pFD->iFilterClass)
	{
	case FILTER_CLASS_FIR:
		return CreateFir(pFD);
	case FILTER_CLASS_IIR:
		return CreateIir(pFD);
	default:
		return FALSE;
	}
}

CDigitalFilter::~CDigitalFilter()
{
	Reset();
}

void CDigitalFilter::Reset()
{
	for (auto r : m_aRatios)
	{
		delete r;
	}
	m_aRatios.clear();

	for (auto r : m_aDerivRatios)
	{
		delete r;
	}
	m_aDerivRatios.clear();
	dwFlags = 0;
}

BOOL CDigitalFilter::CreateFir(NewFilterData * pFD)
{
	ASSERT(pFD != NULL);
	Reset();
	dSamplingRate = pFD->dSamplingRate;
	switch (pFD->iFirType)
	{
	case FILTER_FIR_ZEROPHASE:
		return CreateFirZerophase(pFD);
		break;
	case FILTER_FIR_MINPHASE:
		break;
	case FILTER_FIR_MAXPHASE:
		break;
	}
	return FALSE;
}

BOOL CDigitalFilter::CreateIir(NewFilterData * pFD)
{
	ASSERT(pFD != NULL);
	Reset();
	dSamplingRate = pFD->dSamplingRate;
	switch (pFD->iIirType)
	{
	case FILTER_IIR_ELLIPTIC:
		return CreateEllipticFilter(pFD);
		break;
	case FILTER_IIR_CHEBYCHEV:
		break;
	case FILTER_IIR_BUTTERWORTH:
		break;
	case FILTER_IIR_BESSEL:
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL CDigitalFilter::CreateEllipticFilter(NewFilterData * pFD)
{
	switch (pFD->iFilterType)
	{
	case FILTER_LOWPASS:
		return CreateLowpassElliptic(pFD);
		break;
	case FILTER_HIGHPASS:
		return CreateHighpassElliptic(pFD);
		break;
	case FILTER_BANDPASS:
		return CreateBandpassElliptic(pFD);
		break;
	case FILTER_BANDSTOP:
		break;
	case FILTER_HILBERT:
		return CreateHilbertElliptic(pFD);
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL CDigitalFilter::CreateLowpassElliptic(NewFilterData * pFD)
{
	dCenterFreq = 0.;
	double OmegaPass = 2 * tan(pFD->dLowFreq / dSamplingRate * M_PI);
	double OmegaStop = 2 * tan(pFD->dHighFreq / dSamplingRate * M_PI);
	double MinStopLossDB = pFD->dStopLoss;
	double MaxPassLossDB = pFD->dPassLoss;
	polyRoots zeros;
	polyRoots poles;
	Complex NormCoeff;
	if (pFD->bPowerSymm)
	{
		pFD->iOrder |= 1;
	}
	EllipticPolesZeros(OmegaPass, OmegaStop, MinStopLossDB,
						MaxPassLossDB, pFD->iOrder,
						zeros, poles, NormCoeff);
	polyRoots ZPlanePoles, ZPlaneZeros;
	BilinearLowPass(poles, zeros, 1., ZPlanePoles, ZPlaneZeros);
	// perform bilinear transform or two allpass
	// decomposition
	if (pFD->dPassLoss == 0. || pFD->bPowerSymm)
	{
		poly denom1, denom2, numer1, numer2;
		TwoAllpassDecompose(poles, 1., denom1, numer1, denom2, numer2);
		InsertRatio(polyRatio(numer1, denom1));
		InsertRatio(polyRatio(numer2, denom2));
		MakeCanonical();

		dwFlags |= FILTER_DECOMPOSABLE;
	}
	else
	{

		m_prCanonical = polyRatio(
								poly(ZPlaneZeros,
									BilinearNormCoeff(poles, zeros, 1., NormCoeff)),
								poly(ZPlanePoles));

#if 1
		std::vector<polyRatio*>* pDecomposed
			= polyRatio(poly(zeros, NormCoeff), poly(poles)).Decompose(2, &poles);
		for (auto ratio : *pDecomposed)
		{
#if 0 && defined(_DEBUG)
			ratio->Dump();
#endif
			polyRatio pr(poly(zeros, NormCoeff), poly(poles));
			polyRatio prBil;
			BilinearTransform(*ratio, prBil, 1.);

			InsertRatio(prBil);
			delete ratio;
		}
#else
		std::vector<polyRatio*>* pDecomposed
			= m_prCanonical.Decompose(2, &ZPlanePoles);
		for (auto ratio : *pDecomposed)
		{
#if 0 && defined(_DEBUG)
			ratio->Dump();
#endif
			InsertRatio(*ratio);
			delete ratio;
		}
#endif
		delete pDecomposed;
		dwFlags |= FILTER_CANONICAL_KNOWN | FILTER_ZEROS_KNOWN;
		m_Zeros = ZPlaneZeros;
	}
	m_Poles = ZPlanePoles;
	dwFlags |= FILTER_CREATED | FILTER_IIR | FILTER_POLES_KNOWN
				| FILTER_DECOMPOSABLE;
	//int trail = TrailLength();
	return TRUE;
}

BOOL CDigitalFilter::CreateHighpassElliptic(NewFilterData * pFD)
{
	NewFilterData fd = *pFD;
	fd.dLowFreq = dSamplingRate * 0.5 - pFD->dHighFreq;
	fd.dHighFreq = dSamplingRate * 0.5 - pFD->dLowFreq;

	if (CreateLowpassElliptic( & fd) == FALSE)
		return FALSE;

	dCenterFreq = dSamplingRate * 0.5;
	// convert all polynoms to z = -z
	for (auto ratio : m_aRatios)
	{
		ratio->ScaleRoots(Complex(-1., 0.));
	}
	for (auto ratio : m_aDerivRatios)
	{
		ratio->ScaleRoots(Complex(-1., 0.));
	}
	m_prCanonical.ScaleRoots(Complex(-1., 0.));
	// reflect poles and zeros
	m_Zeros.MakeUnique();
	m_Poles.MakeUnique();
	for (int i = 0; i < m_Zeros.count(); i++)
	{
		m_Zeros[i] = - m_Zeros[i];
	}
	for (int i = 0; i < m_Poles.count(); i++)
	{
		m_Poles[i] = - m_Poles[i];
	}

	return TRUE;
}

BOOL CDigitalFilter::CreateHilbertElliptic(NewFilterData * pFD)
{
	dCenterFreq = dSamplingRate * 0.25;
	double OmegaPass = 2 * tan((0.25 - pFD->dHighFreq / dSamplingRate) * M_PI);
	double MinStopLossDB = pFD->dStopLoss;
	double MaxPassLossDB;
	polyRoots poles;
	EllipticHilbertPoles(OmegaPass, MinStopLossDB,
						MaxPassLossDB, pFD->iOrder, poles);
	poly denom1, numer1, denom2, numer2;
	HilbertTwoAllpassDecompose(poles, denom1, numer1, denom2, numer2);
	InsertRatio(polyRatio(numer1, denom1));
	InsertRatio(polyRatio(numer2, denom2));
	m_Poles = poles;
	dwFlags |= FILTER_CREATED | FILTER_IIR | FILTER_POLES_KNOWN
				| FILTER_COMPLEX | FILTER_DECOMPOSABLE;

	return TRUE;
}

BOOL CDigitalFilter::CreateBandpassElliptic(NewFilterData * pFD)
{
	if (pFD->iCoeffType != FILTER_COEFF_REAL)
	{
		dCenterFreq = pFD->dCenterFreq;
		double dAngle = pFD->dCenterFreq / pFD->dSamplingRate * 2 * M_PI;
		Complex rotator(cos(dAngle), sin(dAngle));


		double OmegaPass = 2 * tan(0.5 * pFD->dLowFreq / dSamplingRate * M_PI);
		double OmegaStop = 2 * tan(0.5 * pFD->dHighFreq / dSamplingRate * M_PI);
		double MinStopLossDB = pFD->dStopLoss;
		double MaxPassLossDB = pFD->dPassLoss;

		polyRoots zeros;
		polyRoots poles;
		Complex NormCoeff;
		if (pFD->bPowerSymm)
		{
			pFD->iOrder |= 1;
		}
		// create low-pass prototype
		EllipticPolesZeros(OmegaPass, OmegaStop, MinStopLossDB,
							MaxPassLossDB, pFD->iOrder,
							zeros, poles, NormCoeff);

		// perform bilinear transform or two allpass
		// decomposition
		BilinearLowPass(poles, zeros, 1., m_Poles, m_Zeros, rotator);

		// perform bilinear transform or two allpass
		// decomposition

		if (pFD->dPassLoss == 0. || pFD->bPowerSymm)
		{
			// create power-symmetric filter from two allpass cells
			poly denom1, denom2, numer1, numer2;
			TwoAllpassDecompose(poles, 1.,
								denom1, numer1, denom2, numer2, dAngle);
			InsertRatio(polyRatio(numer1, denom1));
			InsertRatio(polyRatio(numer2, denom2));
			dwFlags |= FILTER_CREATED | FILTER_IIR | FILTER_POLES_KNOWN
						| FILTER_COMPLEX | FILTER_DECOMPOSABLE;
			MakeCanonical();
			m_prCanonical.denom() = poly(m_Poles);
		}
		else
		{
			m_prCanonical = polyRatio(
									poly(m_Zeros,
										BilinearNormCoeff(poles, zeros, 1., NormCoeff)),
									poly(m_Poles));

#if 1
			std::vector<polyRatio*>* pDecomposed
				= polyRatio(poly(zeros, NormCoeff), poly(poles)).Decompose(2, &poles);
			polyRatio prRem; // constant term
			for (auto ratio : *pDecomposed)
			{
#if 0 && defined(_DEBUG)
				ratio->Dump();
#endif
				polyRatio pr(poly(zeros, NormCoeff), poly(poles));
				polyRatio prBil;
				BilinearTransform(*ratio, prBil, 1., rotator);

				// reduce the ratio
				if (0 && prBil.numer().order() > 0)
				{
					polyRatio pr1;
					PolyDiv(&(pr1.numer()), &(prBil.numer()),
							prBil.numer(), prBil.denom());
					prRem += pr1;
				}
				InsertRatio(prBil);
				delete ratio;
			}

			if (prRem.numer()[0] != 0.)
			{
				if (m_aRatios.front()->denom().order() <= 1)
				{
					*m_aRatios.front() += prRem;
				}
				else if (m_aRatios.size() != 0)
				{
					*m_aRatios.back() += prRem;
				}
				else
				{
					InsertRatio(prRem);
				}
			}
#else
			std::vector<polyRatio*>* pDecomposed
				= m_prCanonical.Decompose(2, &m_Poles);
			for (auto ratio : *pDecomposed)
			{
#if 0 && defined(_DEBUG)
				ratio->Dump();
#endif
				InsertRatio(*ratio);
				delete ratio;
			}
#endif

			delete pDecomposed;
#if 1 && defined (_DEBUG)
			Dump();
#endif
			dwFlags |= FILTER_CREATED | FILTER_IIR | FILTER_POLES_KNOWN
						| FILTER_CANONICAL_KNOWN | FILTER_COMPLEX
						| FILTER_ZEROS_KNOWN;
		}
		return TRUE;
	}
	else
	{
		// create bandpass filter with _real_ coefficients
		dCenterFreq = pFD->dCenterFreq;
		double OmegaPass = 2 * tan(0.5 * pFD->dLowFreq / dSamplingRate * M_PI);
		double OmegaStop = 2 * tan(0.5 * pFD->dHighFreq / dSamplingRate * M_PI);
		double MinStopLossDB = pFD->dStopLoss + 6.;
		double MaxPassLossDB = pFD->dPassLoss;
		polyRoots zeros;
		polyRoots poles;
		Complex NormCoeff;
		if (pFD->bPowerSymm)
		{
			pFD->iOrder |= 1;
		}
		EllipticPolesZeros(OmegaPass, OmegaStop, MinStopLossDB,
							MaxPassLossDB, pFD->iOrder,
							zeros, poles, NormCoeff);
		// perform bilinear transform or two allpass
		// decomposition
		polyRoots ZPlanePoles, ZPlaneZeros;
		NormCoeff = BilinearNormCoeff(poles, zeros, 1., NormCoeff);
		if (pFD->dPassLoss == 0. || pFD->bPowerSymm)
		{
			poly denom1, denom2, numer1, numer2;
			TwoAllpassPassbandDecompose(poles,
										pFD->dCenterFreq / pFD->dSamplingRate * 2 * M_PI,
										1., denom1, numer1, denom2, numer2, ZPlanePoles);

			InsertRatio(polyRatio(numer1, denom1));
			InsertRatio(polyRatio(numer2, denom2));
			dwFlags |= FILTER_DECOMPOSABLE;
		}
		else
		{
			poly numer;
			polyRoots Zpoles1, Zzeros1;
			double w = pFD->dCenterFreq / pFD->dSamplingRate * 2 * M_PI;
			Complex rotator(cos(w), sin(w));
			BilinearLowPass(poles, zeros, 1.,
							ZPlanePoles, ZPlaneZeros, rotator);
			BilinearLowPass(poles, zeros, 1.,
							Zpoles1, Zzeros1, conj(rotator));
			numer = poly(ZPlaneZeros) * poly (Zpoles1);

			int i;
			for (i = 0; i <= numer.order(); i++)
			{
				numer[i] = numer[i].real() * 2;
			}
			Zpoles1 = ZPlanePoles;
			ZPlanePoles.SetCount(0);
			for (i = 0; i < Zpoles1.count(); i++)
			{
				ZPlanePoles += Zpoles1[i];
				ZPlanePoles += conj(Zpoles1[i]);
			}

			m_prCanonical = polyRatio(numer * NormCoeff.real(),
									poly(ZPlanePoles));

			InsertRatio(m_prCanonical);
			dwFlags |= FILTER_CANONICAL_KNOWN | FILTER_FROM_POLES;
		}
		m_Poles = ZPlanePoles;
		dwFlags |= FILTER_CREATED | FILTER_IIR | FILTER_POLES_KNOWN;
		dwFlags &= ~FILTER_ZEROS_KNOWN;
	}

	return TRUE;
}

BOOL CDigitalFilter::CreateFirZerophase(NewFilterData * pFD)
{
	switch (pFD->iFilterType)
	{
	case FILTER_LOWPASS:
		break;
	case FILTER_HIGHPASS:
		break;
	case FILTER_BANDPASS:
		break;
	case FILTER_BANDSTOP:
		break;
	case FILTER_HILBERT:
		break;
#if 0
	case FILTER_SHIFTER:
		return CreateShifter(pFD);
		break;
#endif
	case FILTER_PARTIAL_DELAY:
		return CreatePartialDelayFilter(pFD);
		break;
	case FILTER_STRING_REFLECTOR:
		//return CreateStringReflectorFilter(pFD);
		break;
	default:
		break;
	}
	return FALSE;
}

#if 0
BOOL CDigitalFilter::CreateShifter(NewFilterData * pFD)
{
	dCenterFreq = 0.;
	if (pFD->dDelay >= 1. || pFD->dDelay <= 0.)
		return FALSE;
	poly p;
	pFD->iOrder |= 1;   // make it odd
	p.SetOrder(pFD->iOrder);
	// the responce if sin(n*Pi + dt)/(n*Pi + dt - T0)
	int T0 = (pFD->iOrder + 1) / 2;
	for (int i = 0; i <= pFD->iOrder; i++)
	{
		double sinc = sin(((i - T0) * 2. * pFD->dLowFreq
							/ pFD->dSamplingRate + pFD->dDelay) * M_PI)
					/ (((i - T0) + pFD->dDelay) * M_PI);
		double window = 0.54
						- 0.46 * cos (2. * (i + pFD->dDelay) / pFD->iOrder * M_PI);
		p[i] = sinc * window;
	}
	// scale for unity responce
	Complex factor = p(1.);
	p *= 1. / factor;
	m_prCanonical = polyRatio(p, poly(0, Complex(1.)));
	InsertRatio(m_prCanonical);
	dwFlags |= FILTER_CANONICAL_KNOWN | FILTER_CREATED;

	return TRUE;
}
#endif

BOOL CDigitalFilter::CreatePartialDelayFilter(NewFilterData * pFD)
{
	dCenterFreq = 0.;
	if (pFD->dDelay >= 1. || pFD->dDelay <= 0.)
		return FALSE;
	// calculate using center frequency
	// Use interpolating Lagrange polynome
	int n = pFD->iOrder;
	if (0 == n
		|| n > (int(pFD->dSamplingRate / pFD->dCenterFreq) & ~1))
	{
		n = int(pFD->dSamplingRate / pFD->dCenterFreq) & ~1;
	}
	if (n <= 1) return FALSE;
	if (n >= MAX_FILTER_ORDER) n = MAX_FILTER_ORDER;
	double delay = (n / 2) + pFD->dDelay;
	polyRoots freqs;
	polyRoots values;
	int i;
#if 1
	for (i = 0; i < n / 2; i++)
	{
		Complex freq = Complex(0,
								pFD->dCenterFreq* (i + 1) * 2. * M_PI /
								pFD->dSamplingRate);
		Complex Z = exp(freq);
#if 0
		Complex value = exp(delay * freq);
#else
		double phase = (delay * pFD->dCenterFreq* (i + 1) /
							pFD->dSamplingRate - int (delay * pFD->dCenterFreq* (i + 1) /
													pFD->dSamplingRate)) * 2. * M_PI;
		Complex value = exp(Complex(0., phase));
#endif
		freqs += Z;
		values += value;
	}
	for (i = 0; i < n / 2; i++)
	{
		freqs += conj(freqs[i]);
		values += conj(values[i]);
	}
#else
	for (i = 0; i < n / 2; i++)
	{
		Complex freq = Complex(0,
								pFD->dCenterFreq* (i + 1) * 2. * M_PI /
								pFD->dSamplingRate);
		Complex Z = exp(freq);
		double phase = (delay * pFD->dCenterFreq* (i + 1) /
							pFD->dSamplingRate - int (delay * pFD->dCenterFreq* (i + 1) /
													pFD->dSamplingRate)) * 2. * M_PI;
		Complex value = exp(Complex(0., phase));
		freqs += Z;
		freqs += conj(Z);
		values += value;
		values += conj(value);
	}
#endif
//    if (n & 1)
	{
		freqs += 1.;
		values += 1.;
		freqs += -1.;
		if (int(delay + 0.5) & 1)
		{
			values += -1.;
		}
		else
		{
			values += 1.;
		}
	}
#if 0 //def _DEBUG
	freqs.Dump();
	values.Dump();
#endif
#if 1
	poly p;
	p.FromPoints(polyRoots(freqs).array(), polyRoots(values).array(), freqs.count());
#else
	poly p;
	p.FromPoints(freqs.array(), values.array(), freqs.count());
#endif
	for (i = 0; i <= p.order(); i++)
	{
		p[i].imag(0.);
	}
	m_prCanonical = polyRatio(p, poly(0, Complex(1.)));
	InsertRatio(m_prCanonical);
	dwFlags |= FILTER_CANONICAL_KNOWN | FILTER_CREATED;
	//Complex H0 = (*this)(0.);
	return TRUE;
}

#if 0
BOOL CDigitalFilter::CreateStringReflectorFilter(NewFilterData * pFD,
												CPianoString& PianoString)
{
	dCenterFreq = 0.;
	double frequency = pFD->dLowFreq;
	ASSERT(frequency > 0 && frequency < dSamplingRate / 2);
	double vel0 = PianoString.GetVelocity(frequency);
	double dWaveSamples = dSamplingRate / frequency;
	double dZeroVelocity = PianoString.GetVelocity(0.);
	// build filter of order 20.
	// find phase shift on the main frequency and on higher freqs
	// calculate using center frequency
	// Use poly::FromPoints
	double MaxVelocity = vel0 * 1.1;
	//double MaxVelocity = PianoString.GetVelocity(frequency * 10);
	if (pFD->iOrder == 0)
	{
		double CurrDelay = dWaveSamples * vel0 / MaxVelocity;
		pFD->iOrder = int((dWaveSamples - CurrDelay) * 2);
	}
	int n = pFD->iOrder;
	double delay = (3 * n / 4) + dWaveSamples - floor(dWaveSamples);
	//double delay = (n / 2) + dWaveSamples - floor(dWaveSamples);
	// build the responce using DFT
	int i;
#if 1
	std::vector<Complex> InArray;
	std::vector<Complex> OutArray;
	n = (n + 1) & ~1;
	InArray.resize(n);
	OutArray.resize(n);
	for (i = 1; i < n / 2; i++)
	{
		double CurrFreq = (i * dSamplingRate) / n;
		double CurrVelocity = PianoString.GetVelocity(CurrFreq);
		//double CurrVelocity = PianoString.GetVelocity(frequency);
		if (CurrVelocity > MaxVelocity)
		{
			CurrVelocity = MaxVelocity;
		}
		//if (0) if (CurrFreq < frequency * 10.)
		//{
		//CurrVelocity = PianoString.GetVelocity(CurrFreq);
		//}
		Complex jw = Complex(0,
							CurrFreq * 2. * M_PI /
							dSamplingRate);
		if (CurrVelocity <= 0)
		{
			//break;
		}
		double CurrTrip = dWaveSamples * vel0 / CurrVelocity;
		double CurrDelay = delay
							+ CurrTrip - dWaveSamples
		;
		if (CurrDelay <= delay / 4)
		{
			TRACE("Too big dispersion!\n");
			//break;
			CurrDelay = delay / 4;
		}
		//Complex Z = exp(jw);
		//double phase = CurrDelay * imag(jw) / (2. * M_PI);
		//phase = -(phase - floor(phase)) * 2. * M_PI;;
		double phase = -CurrDelay * imag(jw);
		Complex value = exp(Complex(0., phase));
		InArray[i] = value;
		InArray[n - i] = conj(value);
	}
	InArray[0] = 1.;
	InArray[ n/2] = 0.5;
	for (i = 0; i < n; i++)
	{
		Complex sum = 0;
		for (int j = 0; j < n; j++)
		{
			sum += InArray[j] * exp(Complex(0., i * j * 2. * M_PI / n));
		}
		OutArray[i] = sum / n;
	}
	poly p;
	p.SetOrder(n - 1);
	for (i = 0; i < n; i++)
	{
		p[i] = OutArray[i];
	}
#else
	// build frequency responce array
	polyRoots freqs;
	polyRoots values;
	double CurrFreq = frequency;
	double CurrVelocity = PianoString.GetVelocity(CurrFreq);
	// build frequency grid. Assign at least n/4 samples to
	// area with variable phase speed
	// use logarithmic distribution
	//CArray<double, double> FreqArray;
	//FreqArray.SetSize(n / 2);
	for (i = 1; i < n / 2; i++)
	{
		CurrFreq = (i * dSamplingRate) / n;
		//CurrFreq = (i + 1) * 1000;
		//CurrFreq = frequency * exp(i * log(dSamplingRate / (2. * frequency)) / (n / 2));
		CurrVelocity = PianoString.GetVelocity(CurrFreq);
		if (CurrVelocity > MaxVelocity)
		{
			CurrVelocity = MaxVelocity;
		}
		//if (0) if (CurrFreq < frequency * 10.)
		//{
		//CurrVelocity = PianoString.GetVelocity(CurrFreq);
		//}
		Complex jw = Complex(0,
							CurrFreq * 2. * M_PI /
							dSamplingRate);
		if (CurrVelocity <= 0)
		{
			//break;
		}
		double CurrTrip = dWaveSamples * vel0 / CurrVelocity;
		double CurrDelay = delay + CurrTrip - dWaveSamples;
#if 1
		if (CurrDelay <= delay / 4)
		{
			TRACE("Too big dispersion!\n");
			//break;
			CurrDelay = delay / 4;
		}
#else
		if (CurrDelay <= 0)
		{
			TRACE("Too big dispersion!\n");
			//break;
		}
#endif
		Complex Z = exp(jw);
		double phase = -CurrDelay * imag(jw) /* / (2. * M_PI)*/;
		//phase = -(phase - floor(phase)) * 2. * M_PI;;
		Complex value = exp(Complex(0., phase));
		freqs += Z;
		freqs += conj(Z);
		values += value;
		values += conj(value);
	}
	if (0) for (; i--; )
		{
			freqs += conj(freqs[i]);
			values += conj(values[i]);
		}
//    if (n & 1)
	freqs += 1.;
	values += 1.;
	freqs += -1.;
	if (0)
		values += 0.;
	else     if (int(delay + 0.5) & 1)
	{
		values += -1;
	}
	else
	{
		values += 1;
	}
#if 0 && defined _DEBUG
	freqs.Dump();
	values.Dump();
#endif
#if 0
	poly p;
	p.FromPoints(polyRoots(freqs).array(), polyRoots(values).array(), freqs.count());
#else
	poly p;
	p.FromPoints(freqs.array(), values.array(), freqs.count());
#endif
	for (i = 0; i <= p.order(); i++)
	{
		p[i].imag() = 0.;
	}
#endif
	m_prCanonical = polyRatio(p, poly(0, Complex(1.)));
	InsertRatio(m_prCanonical);
	dwFlags |= FILTER_CANONICAL_KNOWN | FILTER_CREATED;
	return TRUE;
}
#endif

static Complex ipow(const Complex & x, int pow)
{
	Complex tmp;
	Complex acc(1.);
	unsigned upow;
	if (pow < 0)
	{
		tmp = 1. / x;
		upow = - pow;
	}
	else
	{
		tmp = x;
		upow = pow;
	}
	while (upow != 0)
	{
		if (upow & 1)
		{
			acc *= tmp;
		}
		tmp = tmp * tmp;
		upow >>= 1;
	}
	return acc;
}
BOOL CDigitalFilter::InsertRatio(const polyRatio& pr)
{
	m_aRatios.push_back(new polyRatio(pr));
	try
	{
		m_aDerivRatios.push_back(new polyRatio(pr.numer().deriv(),
												pr.denom().deriv()));
	}
	catch (std::bad_alloc&)
	{
		delete *m_aRatios.end();
		m_aRatios.pop_back();
		return FALSE;
	}

	dwFlags &= ~ (FILTER_ZEROS_KNOWN | FILTER_POLES_KNOWN
					| FILTER_CANONICAL_KNOWN);
	dwFlags |= FILTER_DERIV_KNOWN;
	return TRUE;
}

BOOL CDigitalFilter::RemoveRatio(int iIndex)
{
	if (iIndex >= m_aRatios.size())
		return FALSE;
	delete m_aRatios[iIndex];
	delete m_aDerivRatios[iIndex];
	m_aRatios.erase(m_aRatios.begin() + iIndex);
	m_aDerivRatios.erase(m_aDerivRatios.begin() + iIndex);
	dwFlags &= ~FILTER_CANONICAL_KNOWN;
	return TRUE;
}

Complex CDigitalFilter::FreqResponce(double f) const
{
	double w = f / dSamplingRate * 2 * M_PI;
	Complex z(cos(w), sin(w));
	Complex res = 0.;
	if (0 && Flags(FILTER_CANONICAL_KNOWN))
	{
		res = m_prCanonical(z)
			* ipow(z, DenomOrder() - NumerOrder());
	}
	else
	{
		if (m_aRatios.size() != 1)
		{
			for (auto ratio : m_aRatios)
			{
				res += ratio->eval(z)
						* ipow(z, ratio->DenomOrder()
								- ratio->NumerOrder());
			}
		}
		else
		{
			if (Flags(FILTER_FROM_ZEROS))
			{
				res = m_Zeros.eval(z)
					* m_prCanonical.numer()[0];
			}
			else
			{
				res = m_prCanonical.numer()(z);
			}
			if (Flags(FILTER_FROM_POLES))
			{
				res /= m_Poles.eval(z);
			}
			else
			{
				res /= m_prCanonical.denom()(z);
			}
			res *= ipow(z, DenomOrder() - NumerOrder());
		}
	}
	return res;
}

double CDigitalFilter::GroupDelay(double f) const
{
	double w = f / dSamplingRate * 2 * M_PI;
	Complex z(cos(w), sin(w));
	//      H(z)        d H(z) / d z
	Complex resp = 0., respderiv = 0.;
	double delay;
	for (unsigned i = 0; i < m_aRatios.size(); i++)
	{
		const polyRatio* ratio = m_aRatios[i];
		Complex Denom = ratio->denom().eval(z);
		Complex Numer = ratio->numer().eval(z);
		if (Denom != Complex(0., 0.))
		{
			resp += Numer / Denom;
			if (Flags(FILTER_DERIV_KNOWN))
			{
				const polyRatio* deriv_ratio = m_aDerivRatios[i];
				respderiv +=
					(deriv_ratio->numer().eval(z) * Denom
						- deriv_ratio->denom().eval(z) * Numer)
					/ (Denom * Denom);
			}
			else
			{
				respderiv += ratio->deriv().eval(z);
			}
		}
	}
	if (resp == 0.) return 0.;

	delay = -real(z * respderiv / resp);
	delay += NumerOrder() - DenomOrder();
	return delay;
}

// TrailLength - Find number of samples when the signal will fade to
// the specified amount of decibels
int CDigitalFilter::TrailLength(double dDecay) const
{
	if (!Flags(FILTER_IIR))
	{
		ASSERT(Flags(FILTER_CANONICAL_KNOWN));
		return m_prCanonical.numer().order() + 1;
	}
	// Find a pole closest to the unity circle.
	// It is assumed that all the poles are inside the circle
	ASSERT(Flags(FILTER_POLES_KNOWN));
	double dMaxPole = 0.;
	double tmp;
	VERIFY(m_Poles.count() > 0);
	for (int i = 0; i < m_Poles.count(); i++)
	{
		tmp = abs(m_Poles[i]);
		ASSERT (tmp < 1.);
		if (tmp > dMaxPole)
			dMaxPole = tmp;
	}
	return int(- 0.05 * fabs(dDecay) * log(10.) / log(dMaxPole));
}

polyRatio CDigitalFilter::GetCanonical()
{
	if (Flags(FILTER_CANONICAL_KNOWN))
		return m_prCanonical;
	polyRatio pr;
	for (auto ratio : m_aRatios)
	{
		pr += *ratio;
	}
	return pr;
}

void CDigitalFilter::MakeCanonical()
{
	if (!Flags(FILTER_CANONICAL_KNOWN))
	{
		m_prCanonical = GetCanonical();
		dwFlags |= FILTER_CANONICAL_KNOWN;
	}
}

// decompose the ratio to the elementary ratios
//void CDigitalFilter::Decompose()

BOOL CDigitalFilter::MakePoles()
{
	MakeCanonical();
	if (!Flags(FILTER_POLES_KNOWN))
	{
		if (!Flags(FILTER_COMPLEX))
		{
			m_prCanonical.denom().SetReal(TRUE);
		}
		m_Poles = m_prCanonical.denom().roots(-1.);
		dwFlags |= FILTER_POLES_KNOWN;
	}
	return TRUE;
}

BOOL CDigitalFilter::MakeZeros()
{
	MakeCanonical();
	if (!Flags(FILTER_ZEROS_KNOWN))
	{
		if (!Flags(FILTER_COMPLEX))
		{
			m_prCanonical.numer().SetReal(TRUE);
		}
		m_Zeros = m_prCanonical.numer().roots(-1.);
		dwFlags |= FILTER_ZEROS_KNOWN;
	}
	return TRUE;
}

int CDigitalFilter::TemporalResponce(Complex* dst,
									const Complex* src,
									int iCount)
{
	//memcpy(dst, src, iCount * sizeof(Complex));
	//return 1;
	MakeCanonical();
	const int nNumerOrder = m_prCanonical.numer().order();
	const int nNumerLength = 1 + nNumerOrder;
	const int nDenomOrder = m_prCanonical.denom().order();
	const int nDenomLength = nDenomOrder + 1;

	Complex* pPrevInArray = new Complex[2 * nNumerLength];
	Complex* pPrevOutArray = new Complex[2 * nDenomLength];
	for (int i = 0; i < 2 * nNumerLength; i++)
		pPrevInArray[i] = 0.;
	for (int i = 0; i < 2 * nDenomOrder; i++)
		pPrevOutArray[i] = 0.;
	// pPrevInArray and pPrevOutArray are used as
	// the history for the first few samples
	//
	const Complex* pPrevInSampl = pPrevInArray;
	const Complex* pPrevOutSampl = pPrevInArray;
	const Complex* pNumer = m_prCanonical.numer().array();
	const Complex* pDenom = m_prCanonical.denom().array();

	// if the filter was computed from exact poles and zeros,
	// use cascade scheme. Although it may be slightly
	// slower than the regular scheme, I expect it should
	// have better precision

	int nSinglePoles = 0;
	int nConjPoles = 0;
	int nSingleZeros = 0;
	int nConjZeros = 0;
	Complex cNormCoeff(1., 0.);

	if (Flags(FILTER_FROM_POLES))
	{
		// create coefficients array for single and
		// conjugated roots.
		for (int i = 0; i < nDenomOrder; i++)
		{
			Complex tmp = m_Poles.array()[i];
			if (i < nDenomOrder - 1
				&& tmp.imag() != 0.
				&& tmp == conj(m_Poles.array()[i + 1]))
			{
				// conjugated roots found
				nConjPoles += 2;
				pPrevOutArray[2 * (nDenomOrder - nConjPoles)] =
					-2. * tmp.real();
				pPrevOutArray[2 * (nDenomOrder - nConjPoles) + 2] =
					tmp.real() * tmp.real() + tmp.imag() * tmp.imag();
				i++;    // skip one extra pole
			}
			else
			{
				pPrevOutArray[nSinglePoles * 2] = tmp;
				nSinglePoles++;
			}
		}
		ASSERT(nConjPoles + nSinglePoles == nDenomOrder);
	}

	if (Flags(FILTER_FROM_ZEROS))
	{
		// create coefficients array for single and
		// conjugated zeros.
		cNormCoeff = m_prCanonical.numer()[0];
		for (int i = 0; i < nNumerOrder; i++)
		{
			Complex tmp = m_Zeros.array()[i];
			if (i < nNumerOrder - 1
				&& tmp.imag() != 0.
				&& tmp == conj(m_Zeros.array()[i + 1]))
			{
				// conjugated roots found
				nConjZeros += 2;
				pPrevInArray[2 * (nNumerOrder - nConjZeros)] =
					-2. * tmp.real();
				pPrevInArray[2 * (nNumerOrder - nConjZeros) + 2] =
					tmp.real() * tmp.real() + tmp.imag() * tmp.imag();
			}
			else
			{
				pPrevInArray[nSingleZeros * 2] = tmp;
				nSingleZeros++;
			}
		}
		ASSERT(nConjZeros + nSingleZeros == nNumerOrder);
	}

	pPrevOutSampl = pPrevOutArray + nDenomOrder;

	for (int iSample = 0; iSample < iCount; iSample++)
	{
		if ( ! Flags(FILTER_FROM_ZEROS))
		{
			if (iSample < nNumerLength)
			{
				// shift the information in pPrevInArray
				for (int i = 1; i < nNumerLength; i++)
				{
					pPrevInArray[i - 1] = pPrevInArray[i];
				}
				pPrevInArray[nNumerLength - 1] = src[iSample];
				pPrevInSampl = pPrevInArray + nNumerLength - 1;
			}
			else
			{
				pPrevInSampl = src + iSample;
			}
		}

		Complex tmp(0.);
		Complex tmp1;

		if (Flags(FILTER_COMPLEX))
		{
			// process FIR section
			if (Flags(FILTER_FROM_ZEROS))
			{
				tmp = src[iSample] * cNormCoeff;
				// process conjugated zeros
				int i;
				for (i = nNumerOrder; i > nSingleZeros; i -= 2)
				{
					// tmp - current input sample
					// pPrevInArray[i * 2 - 1] - input sample at T-1
					// pPrevInArray[i * 2 - 3] - sample at T-2
					tmp1 = tmp;
					tmp -= pPrevInArray[i * 2 - 3] * pPrevInArray[i * 2 - 4].real()
							+ pPrevInArray[i * 2 - 1] * pPrevInArray[i * 2 - 2].real();
					pPrevInArray[i * 2 - 3] = pPrevInArray[i * 2 - 1];
					pPrevInArray[i * 2 - 1] = tmp1;
				}
				// process single zeros
				for (; i > 0; i --)
				{
					// tmp - current input sample
					// pPrevInArray[i * 2 - 1] - input sample at T-1
					// pPrevInArray[i * 2 - 3] - sample at T-2
					tmp1 = tmp;
					tmp -= pPrevInArray[i * 2 - 1] * pPrevInArray[i * 2 - 2];
					pPrevInArray[i * 2 - 1] = tmp1;
				}
			}
			else
			{
				for (int i = 0; i < nNumerLength; i++)
				{
					tmp += pPrevInSampl[-i] * pNumer[i];
				}
			}

			// process IIR section
			if (Flags(FILTER_FROM_POLES))
			{
				pPrevOutArray[nDenomOrder * 2 + 1] = tmp;
				// process conjugated poles
				int i;
				for (i = nDenomOrder; i > nSinglePoles; i -= 2)
				{
					// pPrevOutArray[i * 2 + 1] - current input sample
					// pPrevOutArray[i * 2 - 1] - sample at T-1
					// pPrevOutArray[i * 2 - 3] - sample at T-2
					// output goes to pPrevOutArray[i * 2 - 3]
					tmp = pPrevOutArray[i * 2 - 3];
					pPrevOutArray[i * 2 - 3] = pPrevOutArray[i * 2 + 1]
												- pPrevOutArray[i * 2 - 3] * pPrevOutArray[i * 2 - 4].real()
												- pPrevOutArray[i * 2 - 1] * pPrevOutArray[i * 2 - 2].real();
					pPrevOutArray[i * 2 - 1] = tmp;
				}
				// process single poles
				for (; i > 0; i --)
				{
					// pPrevOutArray[i * 2 + 1] - current input sample
					// pPrevOutArray[i * 2 - 1] - sample at T-1
					// output goes to pPrevOutArray[i * 2 - 1]
					pPrevOutArray[i * 2 - 1] = pPrevOutArray[i * 2 + 1]
												+ pPrevOutArray[i * 2 - 1] * pPrevOutArray[i * 2 - 2];
				}
				ASSERT(i * 2 + 1 == 1);
				tmp = pPrevOutArray[1];
			}
			else
			{
				for (int i = 1; i < nDenomLength; i++)
				{
					tmp -= pPrevOutSampl[-i] * pDenom[i];
				}
			}
		}
		else
		{
			// process FIR section
			if (Flags(FILTER_FROM_ZEROS))
			{
				tmp = src[iSample] * cNormCoeff;
				// process conjugated zeros
				int i;
				for (i = nNumerOrder; i > nSingleZeros; i -= 2)
				{
					// tmp - current input sample
					// pPrevInArray[i * 2 - 1] - input sample at T-1
					// pPrevInArray[i * 2 - 3] - sample at T-2
					tmp1 = tmp;
					tmp -= pPrevInArray[i * 2 - 3] * pPrevInArray[i * 2 - 4].real()
							+ pPrevInArray[i * 2 - 1] * pPrevInArray[i * 2 - 2].real();
					pPrevInArray[i * 2 - 3] = pPrevInArray[i * 2 - 1];
					pPrevInArray[i * 2 - 1] = tmp1;
				}
				// process single zeros
				for (; i > 0; i --)
				{
					// tmp - current input sample
					// pPrevInArray[i * 2 - 1] - input sample at T-1
					// pPrevInArray[i * 2 - 3] - sample at T-2
					tmp1 = tmp;
					tmp -= pPrevInArray[i * 2 - 1] * pPrevInArray[i * 2 - 2].real();
					pPrevInArray[i * 2 - 1] = tmp1;
				}
			}
			else
			{
				for (int i = 0; i < nNumerLength; i++)
				{
					tmp += pPrevInSampl[-i] * pNumer[i].real();
				}
			}

			// process IIR section
			if (Flags(FILTER_FROM_POLES))
			{
				pPrevOutArray[nDenomOrder * 2 + 1] = tmp;
				// process conjugated poles
				int i;
				for (i = nDenomOrder; i > nSinglePoles; i -= 2)
				{
					// pPrevOutArray[i * 2 + 1] - current input sample
					// pPrevOutArray[i * 2 - 1] - sample at T-1
					// pPrevOutArray[i * 2 - 3] - sample at T-2
					// output goes to pPrevOutArray[i * 2 - 3]
					tmp = pPrevOutArray[i * 2 - 3];
					pPrevOutArray[i * 2 - 3] = pPrevOutArray[i * 2 + 1]
												- pPrevOutArray[i * 2 - 3] * pPrevOutArray[i * 2 - 4].real()
												- pPrevOutArray[i * 2 - 1] * pPrevOutArray[i * 2 - 2].real();
					pPrevOutArray[i * 2 - 1] = tmp;
				}
				// process single poles
				for (; i > 0; i --)
				{
					// pPrevOutArray[i * 2 + 1] - current input sample
					// pPrevOutArray[i * 2 - 1] - sample at T-1
					// output goes to pPrevOutArray[i * 2 - 1]
					pPrevOutArray[i * 2 - 1] = pPrevOutArray[i * 2 + 1]
												+ pPrevOutArray[i * 2 - 1] * pPrevOutArray[i * 2 - 2].real();
				}
				tmp = pPrevOutArray[1];
			}
			else
			{
				for (int i = 1; i < nDenomLength; i++)
				{
					tmp -= pPrevOutSampl[-i] * pDenom[i].real();
				}
			}
		}
		dst[iSample] = tmp;

		if ( ! Flags(FILTER_FROM_POLES))
		{
			if (iSample < nDenomOrder - 1)
			{
				// shift the information in pPrevOutArray
				for (int i = 1; i < nDenomOrder; i++)
				{
					pPrevOutArray[i - 1] = pPrevOutArray[i];
				}
				pPrevOutArray[nDenomOrder-1] = dst[iSample];
				pPrevOutSampl = pPrevOutArray + nDenomOrder;
			}
			else
			{
				pPrevOutSampl = dst + iSample + 1;
			}
		}
	}
	delete [] pPrevOutArray;
	delete [] pPrevInArray;
	return 1;
}

int CDigitalFilter::TemporalResponce(
									CSignalStoreIterator* dst,
									CSignalIterator * src,
									int iCount)
{
	ASSERT(src != NULL);
	ASSERT(dst != NULL);
	// temporal responce is calculated as a sum
	// of responces of all elementary filtercells (m_aRatios);
	// The same if FILTER_FROM_POLES or FILTER_FROM_ZEROS flag is set.
	// 1. Allocate a common buffer for filter coefficients (Complex, of exact size)
	// Allocate a common buffer for numerator/denominator orders (int[m_aRatios.GetSize()])
	//  Can be allocated on stack, size 256.
	// 2. Fill coeff buffer
	// 3. Allocate a common zero-initialized buffer for filter history (3*filter coeff size >= 128)
	// 4. Loop on input samples.
	// 5. Loop on filter cells.
	//      Put input signal to numerator history array.
	// 6. loop on numerator taps.
	//      put numerator output to denominator history array
	// 7. Loop on denominator taps.
	// 8. Add denominator result to the total

	// a common buffer for numerator/denominator orders
	unsigned nNumerCountsArray[128];
	unsigned nDenomCountsArray[128];

	ASSERT(m_aRatios.size() <= 128);
	// 1. Allocate a common buffer for filter coefficients (Complex, of exact size)
	unsigned nCoeffArraySize = 0;
	for (unsigned i = 0; i < m_aRatios.size(); i++)
	{
		nNumerCountsArray[i] = 1 + m_aRatios[i]->numer().order();
		nCoeffArraySize += nNumerCountsArray[i];
		nDenomCountsArray[i] = 1 + m_aRatios[i]->denom().order();
		nCoeffArraySize += nDenomCountsArray[i];
	}

	Complex* pCoeffsArray = new Complex[nCoeffArraySize];

	// 2. Fill coeff buffer
	Complex* pCoeff = pCoeffsArray;
	for (unsigned i = 0; i < m_aRatios.size(); i++)
	{
		for (unsigned j = 0; j < nNumerCountsArray[i]; j++)
		{
			*pCoeff++ = m_aRatios[i]->numer()[j];
		}
		for (unsigned j = 0; j < nDenomCountsArray[i]; j++)
		{
			*pCoeff++ = m_aRatios[i]->denom()[j];
		}
	}

	// 3. Allocate a common zero-initialized buffer for filter history (3*filter coeff size >= 128)
	unsigned nHistorySize = nCoeffArraySize * 3;
	if (nHistorySize < 128) nHistorySize = 128;

	// kludge to avoid unaligned allocation
	Complex* pHistoryArray = new Complex[nHistorySize];

	if (pHistoryArray != NULL)
	{
		for (unsigned i = 0; i < nHistorySize; i++)
		{
			pHistoryArray[i] = 0.;
		}
	}
	// 4. Loop on input samples.
	Complex * pHistoryPtr = pHistoryArray;
	// separate branches for Complex and real data
	if (Flags(FILTER_COMPLEX) || src->Flags(SIGNAL_COMPLEX))
	{
		ASSERT(dst->Flags(SIGNAL_COMPLEX));
		for (int iSample = 0; iSample < iCount; iSample++)
		{
			Complex InSample = src->GetNextComplex();
			Complex OutSample = 0.;
			pCoeff = pCoeffsArray;
			// 5. Loop on filter cells.
			for (unsigned nCell = 0; nCell < m_aRatios.size(); nCell++)
			{
				//      Put input signal to numerator history array.
				pHistoryPtr[0] = InSample;
				Complex tmp = 0.;
				// 6. loop on numerator taps.
				if(2 == nNumerCountsArray[nCell])
				{
					tmp += pCoeff[0] * pHistoryPtr[0]
							+ pCoeff[1] * pHistoryPtr[1];
					pHistoryPtr += 2;
					pCoeff += 2;
				}
				else if(3 == nNumerCountsArray[nCell])
				{
					tmp += pCoeff[0] * pHistoryPtr[0]
							+ pCoeff[1] * pHistoryPtr[1]
							+ pCoeff[2] * pHistoryPtr[2];
					pHistoryPtr += 3;
					pCoeff += 3;
				}
				else
				{
					for (unsigned j = 0; j < nNumerCountsArray[nCell]; j++)
					{
						tmp += pCoeff[j] * pHistoryPtr[j];
					}
					pHistoryPtr += nNumerCountsArray[nCell];
					pCoeff += nNumerCountsArray[nCell];
				}
				// 7. Loop on denominator taps.
				if(3 == nDenomCountsArray[nCell])
				{
					pHistoryPtr[0] -= pCoeff[2] * pHistoryPtr[2] + pCoeff[1] * pHistoryPtr[1];

					OutSample += pHistoryPtr[0];
					// put the output to denominator history array
					// 8. Add denominator result to the total
					//advance the pointers
					pHistoryPtr += 3;
					pCoeff += 3;
				}
				else
				{
					for (unsigned j = 1; j < nDenomCountsArray[nCell]; j++)
					{
						tmp -= pCoeff[j] * pHistoryPtr[j];
					}
					// put the output to denominator history array
					pHistoryPtr[0] = tmp;
					// 8. Add denominator result to the total
					OutSample += tmp;
					//advance the pointers
					pHistoryPtr += nDenomCountsArray[nCell];
					pCoeff += nDenomCountsArray[nCell];
				}
			}
			ASSERT (pCoeff == pCoeffsArray + nCoeffArraySize);
			dst->SetNextComplex(OutSample);
			pHistoryPtr -= nCoeffArraySize;

			// history emulates infinite buffer. When the pointer moves too low,
			// move all the data to the end of array
			if (pHistoryPtr <= pHistoryArray)
			{
				pHistoryPtr = pHistoryArray + nHistorySize - nCoeffArraySize;
				// the memory does not overlap, can use memcpy
				ASSERT(nHistorySize >= nCoeffArraySize * 2);
				memcpy(pHistoryPtr, pHistoryArray, nCoeffArraySize * sizeof * pHistoryPtr);
			}
			pHistoryPtr--;
		}
	}
	else
		for (int iSample = 0; iSample < iCount; iSample++)
		{
			double InSample = src->GetNextDouble();
			double OutSample = 0.;
			pCoeff = pCoeffsArray;
			// 5. Loop on filter cells.
			for (unsigned nCell = 0; nCell < m_aRatios.size(); nCell++)
			{
				//      Put input signal to numerator history array.
				pHistoryPtr[0] = InSample;
				double tmp = 0.;
				// 6. loop on numerator taps.
				if(2 == nNumerCountsArray[nCell])
				{
					tmp = tmp + pCoeff[0].real() * pHistoryPtr[0].real()
						+ pCoeff[1].real() * pHistoryPtr[1].real();
					pHistoryPtr += 2;
					pCoeff += 2;
				}
				else if(3 == nNumerCountsArray[nCell])
				{
					tmp = tmp + pCoeff[0].real() * pHistoryPtr[0].real()
						+ pCoeff[1].real() * pHistoryPtr[1].real()
						+ pCoeff[2].real() * pHistoryPtr[2].real();
					pHistoryPtr += 3;
					pCoeff += 3;
				}
				else
				{
					for (unsigned j = 0; j < nNumerCountsArray[nCell]; j++)
					{
						tmp += pCoeff[j].real() * pHistoryPtr[j].real();
					}
					pHistoryPtr += nNumerCountsArray[nCell];
					pCoeff += nNumerCountsArray[nCell];
				}
				// 7. Loop on denominator taps.
				if(3 == nDenomCountsArray[nCell])
				{
					pHistoryPtr[0].real(tmp - pCoeff[2].real() * pHistoryPtr[2].real()
										- pCoeff[1].real() * pHistoryPtr[1].real());
					OutSample += pHistoryPtr[0].real();
					// put the output to denominator history array
					// 8. Add denominator result to the total
					//advance the pointers
					pHistoryPtr += 3;
					pCoeff += 3;
				}
				else
				{
					for (unsigned j = 1; j < nDenomCountsArray[nCell]; j++)
					{
						tmp -= pCoeff[j].real() * pHistoryPtr[j].real();
					}
					// put the output to denominator history array
					pHistoryPtr[0] = tmp;
					// 8. Add denominator result to the total
					OutSample += tmp;
					//advance the pointers
					pHistoryPtr += nDenomCountsArray[nCell];
					pCoeff += nDenomCountsArray[nCell];
				}
			}
			ASSERT (pCoeff == pCoeffsArray + nCoeffArraySize);
			dst->SetNextDouble(OutSample);
			pHistoryPtr -= nCoeffArraySize;

			// history emulates infinite buffer. When the pointer moves too low,
			// move all the data to the end of array
			if (pHistoryPtr <= pHistoryArray)
			{
				pHistoryPtr = pHistoryArray + nHistorySize - nCoeffArraySize;
				// the memory does not overlap, can use memcpy
				ASSERT(nHistorySize >= nCoeffArraySize * 2);
				memcpy(pHistoryPtr, pHistoryArray, nCoeffArraySize * sizeof * pHistoryPtr);
			}
			pHistoryPtr--;
		}

	delete[] pHistoryArray;
	delete[] pCoeffsArray;

	return 1;
}

#ifdef _DEBUG
void CDigitalFilter::Dump(CDumpContext & dc)
{
	// write filter comments
	// filter type, coeff type, sampling rate,
	// frequencies, and so on.
	CString s;
	if (Flags(FILTER_IIR))
	{
		s = _T("Infinite impulse response filter\n");
	}
	else
	{
		s = _T("Finite impulse response filter\n");
	}
	s += _T("[Numerator coeffs]\n");
	MakeCanonical();
	dc << s;
	m_prCanonical.numer().Dump(dc);
	if (Flags(FILTER_ZEROS_KNOWN))
	{
		dc << "[Zeros]\n";
		Zeros().Dump(dc);
	}
	if (Flags(FILTER_IIR))
	{
		ASSERT(m_prCanonical.denom().order() > 0);
		dc << "[Denominator coeffs]\n";
		m_prCanonical.denom().Dump(dc);
		if (Flags(FILTER_POLES_KNOWN))
		{
			dc << "[Poles]\n";
			Poles().Dump(dc);
			double dMaxPole = 0.;
			double tmp;
			for (int i = 0; i < m_Poles.count(); i++)
			{
				tmp = abs(m_Poles[i]);
				ASSERT (tmp < 1.);
				if (tmp > dMaxPole)
					dMaxPole = tmp;
			}
			s.Format(_T("Max Pole Quality = %f\n"),
					1. / (1. - dMaxPole));
			dc << s;
			s.Format(_T("Trail Length = %d samples (for 90 dB decay)"),
					TrailLength());
			dc << s;
		}
	}
}
#endif