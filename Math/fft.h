// Copyright Alexander Grigoriev, 1997-2025, All Rights Reserved
// fft.h, copyright Alex Grigoriev, alegrigoriev@gmail.com
#pragma once
#include <complex>

namespace FFT
{

enum FftOptions : unsigned
{
	None = 0,
	Inverse = 1,
	/* Force use of FPU for calculations */
	DontUseSSE2 = 2,
	/* FFT_NormalizeToUnity changes scale of FFT terms.
	   By default, AC result of a sine/cosine wave of unity amplitude produces a term with magnitude
	   equal half the length of of the source array,
	   and DC magnitude 1 will produce zeroth term equal the length of of the source array.
	   If FFT_NormalizeToUnity bit set in 'options',
	   unity AC inputs will produce outputs with magnitude 0.5, and unity DC will produce unity zeroth term.

	   IFFT_NormalizedToUnity used with IFFT, changes the source scale to properly accept results
	   of FFT with FFT_NormalizeToUnity option.

	   If you do FFT with FFT_ScalePowerToUnity, and then IFFT with IFFT_PowerScaledToUnity,
	   it produces the original source array back.
	   */
	FFT_NormalizeToUnity = 4,
	IFFT_NormalizedToUnity = FFT_NormalizeToUnity,
};

}

template<class T>
extern void FastFourierTransform(std::complex<T>* x,
								unsigned count,
								unsigned options = 0);

template<class T>
extern void FastInverseFourierTransform(std::complex<T>* x,
										unsigned count,
										unsigned options = 0);

// FFT real -> complex.
// converts [count] real source samples to [count / 2 + 1]
// complex terms.
// IMPORTANT: dst array should be of size [count / 2 + 1];
template<class T>
extern void FastFourierTransform(const T* src,
								std::complex<T>* dst,
								unsigned count,
								unsigned options = 0);

// IFFT complex -> real.
// converts [count / 2 + 1] complex source terms to
// [count] real samples.
// IMPORTANT: src array should be of size [count / 2 + 1];
template<class T>
extern void FastInverseFourierTransform(const std::complex<T>* src,
										T* dst,
										unsigned count,
										unsigned options = 0);

template<class T>
extern void FastFourierTransform(const std::complex<T>* src, std::complex<T>* dst,
								unsigned count,
								unsigned options = 0);

template<class T>
extern void FastInverseFourierTransform(const std::complex<T>* src,
										std::complex<T>* dst,
										unsigned count,
										unsigned options = 0);
