// Copyright Alexander Grigoriev, 1997-2016, All Rights Reserved
// File FFT.inl

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef ASSERT
#define ASSERT(x)
#endif
#include <intrin.h>

// Conversion of 'count' complex FFT result terms to 'count'
// terms as if they were obtained from real data. Used in real->complex FFT
template<class T>
void FFTPostProc(std::complex<T> * x, const unsigned count)
{
	ASSERT(count > 0 && count % 2 == 0);
	double angle = M_PI / count;
	std::complex<double> rot(cos(angle), -sin(angle));
	std::complex<double> u(0., -1.);

	x[count] = x[0];

	for (unsigned i = 0, k = count; i <= count / 2; i++, k--)
	{
		std::complex<double> tmp = x[k] + conj(x[i]);
		std::complex<double> tmp2 = x[k] - conj(x[i]);

		tmp2 = tmp2 * u;
		x[i] = conj(0.5 * (tmp + tmp2));
		x[k] = 0.5 * (tmp - tmp2);
		u *= rot;
	}
}

// Conversion of 'count+1' complex FFT result terms to 'count+1'
// terms as if they were obtained from complex data.
// Used in complex->real IFFT
template<class T>
void IFFTPreProc(const std::complex<T> * src, std::complex<T> * dst, const unsigned count)
{
	ASSERT(count > 0 && count % 2 == 0);
	double angle = M_PI / count;
	std::complex<double> rot(cos(angle), sin(angle));
	std::complex<double> u(0., -1.);

	dst[0] = T(0.5) * (src[0] + conj(src[count]) + std::complex<T>(0., -1.) * (conj(src[count]) - src[0]));

	for (unsigned i = 1, k = count - 1; i <= count / 2; i++, k--)
	{
		u *= rot;
		std::complex<T> tmp = conj(src[k]) + src[i];
		std::complex<T> tmp2 = u * std::complex<double>(conj(src[k]) - src[i]);

		dst[i] = T(0.5) * (tmp + tmp2);
		dst[k] = T(0.5) * conj(tmp - tmp2);
	}
}

//inline implementation of a generic complex->complex fft function
template<class T>
void FastFourierTransformCore(const std::complex<T> * src, std::complex<T> * dst,
							const unsigned count,
							const bool reverse_fft)
{
	std::complex<double> cs2(cos(M_PI / (count/2)), sin(M_PI / (count/2)));
	if (reverse_fft) cs2 = conj(cs2);

	for (unsigned f = count / 2; f > 1; f /= 2, src = dst)
	{
		// f iterates from 2^order_power to 4

		double z = 2.*M_PI / f;		// double of the standard rotation
		if (reverse_fft) z = -z;

		int e = f * 2;

		std::complex<double> uv0(1., 0.);
		std::complex<double> uv1 = cs2;		// from previous step
		cs2.real(cos(z));
		cs2.imag(sin(z));

		for (unsigned j = 0; j < f; j+=2)
		{
			// on the first pass, j goes from 0 to half array size, for each complex number
			// on the second pass, j goes from 0 to quarter array size, for each complex number
			// etc
			for(unsigned i = j; i < count; i += e)
			{
				// on the first j pass, i goes from j for each complex number
				const std::complex<T> * srci = src + i;
				std::complex<T> * dsti = dst + i;
				std::complex<double> s0 = srci[0], s1 = srci[f];
				dsti[0] = s0 + s1;
				dsti[f] = uv0 * (s0 - s1);

				s0 = srci[1];
				s1 = srci[f + 1];
				dsti[1] = s0 + s1;
				dsti[f+1] = uv1 * (s0 - s1);
			} // i-loop
			uv0 *= cs2;
			uv1 *= cs2;
		}  // j - loop
	} // L - loop

	// last pass, each couple of complex numbers goes through add/subtract
	if (reverse_fft)
	{
		T a = T(2.0 / count);
		for (unsigned i = 0; i < count; i += 2)
		{
			std::complex<T> s0 = src[i], s1 = src[i + 1];
			dst[i] = a * (s0 + s1);
			dst[i + 1] = a * (s0 - s1);
		} // i-loop
	}
	else
	{
		for (unsigned i = 0; i < count; i += 2)
		{
			std::complex<T> s0 = src[i], s1 = src[i + 1];
			dst[i] = (s0 + s1);
			dst[i + 1] = (s0 - s1);
		} // i-loop
	}

	unsigned m = count / 2;	// iterate to the half of the array size
	std::complex<T> * dst_m = dst + m;	// half of the array, corresponds to the most significant bit of i, j
	for (unsigned i = 0, j = 0; i < m; i +=2)
	{
		// j is a bit reverse of i
		std::complex<T> tmp;
		// exchange 0..i..1 and 1..j..0
		tmp = dst_m[j];
		dst_m[j] = dst[i + 1];
		dst[i + 1] = tmp;

		if (i > j)
		{
			// exchange 0..i..0 and 0..j..0
			tmp = dst[j];
			dst[j] = dst[i];
			dst[i] = tmp;
			// exchange 1..i..1 and 1..j..1
			tmp = dst_m[j+1];
			dst_m[j+1] = dst_m[i+1];
			dst_m[i+1] = tmp;
		}
		// n = number of complex numbers
		unsigned k = count / 4;
		// simulate increment on reverse bit pattern by propagating the carry from most to least significant bit
		while(k & j)
		{
			j ^= k;
			k >>= 1;
		}
		j += k;
	}
}

template<class T> void FastFourierTransform(std::complex<T> * x, unsigned count)
{
	ASSERT(count >= 2 && count < 0x08000000
			&& count == (count & (0-count))
			&& x != NULL);
	FastFourierTransformCore(x, x, count, false);
}

template<class T> void FastInverseFourierTransform(std::complex<T> * x, unsigned count)
{
	ASSERT(count >= 2 && count < 0x08000000
			&& count == (count & (0-count))
			&& x != NULL);
	FastFourierTransformCore(x, x, count, true);
}

// FFT real -> complex.
// converts (count) real source samples to (count / 2 + 1)
// complex terms.
// IMPORTANT: dst array should be of size (count / 2 + 1);
template<class T>
void FastFourierTransform(const T * src, std::complex<T> * dst,
						unsigned count)
{
	ASSERT(count >= 4 && count < 0x08000000
			&& count == (count & (0-count))
			&& src != NULL && dst != NULL);

	//T * tmp = reinterpret_cast<T * >(dst);
	FastFourierTransformCore(reinterpret_cast<const std::complex<T> *>(src), dst, count / 2, false);
	FFTPostProc(dst, count / 2);
}

// IFFT complex -> real.
// converts (count / 2 + 1) complex source terms to
// (count) real samples.
// IMPORTANT: src array should be of size (count / 2 + 1);
template<class T>
void FastInverseFourierTransform(const std::complex<T> * src, T * dst,
								unsigned count)
{
	ASSERT(count >= 4 && count < 0x08000000
			&& count == (count & (0-count))
			&& src != NULL && dst != NULL);

	IFFTPreProc(src, reinterpret_cast<std::complex<T> *>(dst), count / 2);

	FastFourierTransformCore(reinterpret_cast<std::complex<T> *>(dst), reinterpret_cast<std::complex<T> *>(dst), count / 2, true);
}
