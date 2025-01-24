// Implement fast windowed MDCT through FFT
#pragma once
#include "FFT.inl"

namespace FFT
{

// IMPORTANT: src array should be of size [count + 1];
// src can be an alias of dst array.
template<typename T>
void WMDCTPostProcess(const std::complex<T> *src, T* dst, unsigned count,
	const std::complex<double> *Roots, double multiplier)
{
	typedef std::complex<T> complexT;
	typedef std::complex<double> complex;

	ASSERT(count >= 2);
	ASSERT(count < 0x08000000);
	ASSERT(count == (count & (0 - count)));
	ASSERT(dst != NULL);

	complex prev = src[0];
	for (unsigned i = 0; i < count; i++)
	{
		Roots++;
		src++;
		complex next = complex(*src) * *Roots;
		prev = next - prev;

		double tmp;
		if (i & 1)
		{
			tmp = prev.real() - prev.imag();
		}
		else
		{
			tmp = prev.real() + prev.imag();
		}

		if (i & 2)
		{
			tmp = -tmp;
		}
		dst[i] = T(multiplier * tmp);
		prev = next;
	}
}

}

// Windowed modified discrete cosine transfer
// converts [count] real source samples to [count / 2] real terms.
// IMPORTANT: dst array should be of size [count + 2];
template<typename T>
void WindowedMDCT(const T* src, T* dst, unsigned count, unsigned options)
{
	using namespace FFT;
	typedef std::complex<T> complexT;

	ASSERT(count >= 4);
	ASSERT(count < 0x08000000);
	ASSERT(count == (count & (0 - count)));
	ASSERT(src != NULL);
	ASSERT(dst != NULL);

	complexT *cdst = reinterpret_cast<complexT*>(dst);

	complex* Roots = static_cast<complex*>(_alloca(count * sizeof(complex)));
	MakeComplexRootsOfUnity(Roots, count, false);

	FastFourierTransformCore(reinterpret_cast<const complexT*>(src), cdst, count / 2, Roots, count, 0);
	FFTPostProc(cdst, count / 2, Roots, count, options & ~FftOptions::FFT_NormalizeToUnity);

	WMDCTPostProcess(cdst, dst, count / 2, Roots,
		(options & FftOptions::FFT_NormalizeToUnity) ?
		(1.48982958696017290 / count) : 0.7071067811865475);
}
