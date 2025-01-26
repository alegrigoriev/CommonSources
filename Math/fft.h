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
};

}

template<class T>
extern void FastFourierTransform(std::complex<T>* x,
								unsigned count);

template<class T>
extern void FastInverseFourierTransform(std::complex<T>* x,
										unsigned count);

// FFT real -> complex.
// converts [count] real source samples to [count / 2 + 1]
// complex terms.
// IMPORTANT: dst array should be of size [count / 2 + 1];
template<class T>
extern void FastFourierTransform(const T* src,
								std::complex<T>* dst,
								unsigned count);

// IFFT complex -> real.
// converts [count / 2 + 1] complex source terms to
// [count] real samples.
// IMPORTANT: src array should be of size [count / 2 + 1];
template<class T>
extern void FastInverseFourierTransform(const std::complex<T>* src,
										T* dst,
										unsigned count);

template<class T>
extern void FastFourierTransform(const std::complex<T>* src, std::complex<T>* dst,
								unsigned count);

template<class T>
extern void FastInverseFourierTransform(const std::complex<T>* src,
										std::complex<T>* dst,
										unsigned count);
