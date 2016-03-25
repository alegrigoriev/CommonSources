// Copyright Alexander Grigoriev, 1997-2016, All Rights Reserved
// fft.h, copyright Alex Grigoriev, alegrigoriev@gmail.com

#include <complex>
#include "fft.inl"

template<class T>
void FastFourierTransform(const std::complex<T> * src, std::complex<T> * dst,
						int count)
{
	ASSERT(count >= 2 && count < 0x08000000
			&& count == (count & -count)
			&& src != NULL && dst != NULL);

	FastFourierTransformCore(src, dst, count, false);
}

template<class T>
inline void FastInverseFourierTransform(const std::complex<T> * src, std::complex<T> * dst,
										int count)
{
	ASSERT(count >= 2 && count < 0x08000000
			&& count == (count & -count)
			&& src != NULL && dst != NULL);

	FastFourierTransformCore(src, dst, count, true);
}

// FFT real -> complex.
// converts (count) real source samples to (count / 2 + 1)
// complex terms.
// IMPORTANT: dst array should be of size (count / 2 + 1);
//template<class T>
//void FastFourierTransform(const T * src, std::complex<T> * dst,
//						int count);
// IFFT complex -> real.
// converts (count / 2 + 1) complex source terms to
// (count) real samples.
// IMPORTANT: src array should be of size (count / 2 + 1);
//template<class T>
//void FastInverseFourierTransform(const std::complex<T> * src, T * dst,
//								int count);

