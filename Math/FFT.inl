// Copyright Alexander Grigoriev, 1997-2016, All Rights Reserved
// File FFT.inl

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef ASSERT
#define ASSERT(x)
#endif
#include <intrin.h>

namespace FFT
{

// Conversion of 'count' complex FFT result terms to 'count'
// terms as if they were obtained from real data. Used in real->complex FFT
template<class T>
void FFTPostProc(std::complex<T> * x, const unsigned count)
{
	ASSERT(count > 0 && count % 2 == 0);
	double angle = M_PI / count;
	std::complex<double> rot2(cos(2*angle), -sin(2*angle));
	std::complex<double> u0(-sin(angle), -cos(angle));
	std::complex<double> u1(-sin(2*angle), -cos(2*angle));

	x[count] = std::complex<T>(T(0.), T(x[0].real() - x[0].imag()));
	x[0] = std::complex<T>(T(x[0].real() + x[0].imag()), T(0.));

	// at i==count/2 u reaches -unity, and x[k] stays unchanged
	for (unsigned i = 0, k = count; ; )
	{
		k--;
		i++;
		std::complex<double> xk = x[k];
		std::complex<double> xi = conj(x[i]);
		std::complex<double> tmp = xk + xi;
		std::complex<double> tmp2 = u0 * (xk - xi);
		u0 *= rot2;

		x[i] = conj(0.5 * (tmp + tmp2));
		x[k] = 0.5 * (tmp - tmp2);
		k--;
		i++;
		if (i == k)
		{
			break;
		}

		xk = x[k];
		xi = conj(x[i]);
		tmp = xk + xi;
		tmp2 = u1 * (xk - xi);
		u1 *= rot2;

		x[i] = conj(0.5 * (tmp + tmp2));
		x[k] = 0.5 * (tmp - tmp2);

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
	std::complex<double> rot(cos(angle), -sin(angle));
	std::complex<double> u(0., -1.);

	dst[0] = T(0.5) * (src[0] + conj(src[count]) + std::complex<T>(0., 1.) * (conj(src[count]) + src[0]));

	for (unsigned i = 1, k = count - 1; i <= count / 2; i++, k--)
	{
		u *= rot;
		std::complex<T> tmp = conj(src[k]) + src[i];
		std::complex<T> tmp2 = std::complex<T>(u * std::complex<double>(conj(src[k]) - src[i]));

		dst[i] = T(0.5) * (tmp + tmp2);
		dst[k] = T(0.5) * conj(tmp - tmp2);
	}
}

// FFT implementation accelerated with Intel SSE2
static void FastFourierTransformCoreSSE2(const std::complex<double> * complex_src, std::complex<double> * complex_dst,
										const unsigned count,
										const bool reverse_fft)
{
	const __m128d * src = reinterpret_cast<const __m128d *>(complex_src);
	__m128d * dst = reinterpret_cast<__m128d *>(complex_dst);
	// The source and destination arrays are float/double pairs (complex numbers),
	// total 2^order_power complex numbers, or 2*2^order_power of float/double items
	for (unsigned f = count / 2; f > 1; f /= 2, src = dst)
	{
		// f iterates from 2^order_power to 4

		double z = M_PI / f;
		if (reverse_fft) z = -z;

		unsigned e = f * 2;
		__m128d cs = { cos(z), sin(z) };

		__m128d uv = { 1., 0. };
		__m128d cs_swap = {cs.m128d_f64[1], cs.m128d_f64[0]};

		for (unsigned j = 0; j < f; j++)
		{
			// on the first pass, j goes from 0 to half array size, for each complex number
			// on the second pass, j goes from 0 to quarter array size, for each complex number
			// etc
			__m128d uv_re = _mm_shuffle_pd(uv, uv, 0);	// duplicated real
			__m128d uv_im = _mm_shuffle_pd(uv, uv, 3);	// duplicated imag

			for(unsigned i = j; i < count; i += e)
			{
				const __m128d *srci = src + i;
				__m128d *dsti = dst + i;
				// on the first j pass, i goes from j for each complex number
				register __m128d rt = _mm_sub_pd(srci[0], srci[f]);
				dsti[0] = _mm_add_pd(srci[0], srci[f]);
				dsti[f] = _mm_addsub_pd(_mm_mul_pd(rt, uv_re), _mm_mul_pd(_mm_shuffle_pd(rt, rt, 1), uv_im));
			} // i-loop
			uv = _mm_addsub_pd(_mm_mul_pd(cs, uv_re), _mm_mul_pd(cs_swap, uv_im));
		}  // j - loop
	} // f - loop

	// last pass, each couple of complex numbers goes through add/subtract
	if (reverse_fft)
	{
		__m128d a = { 1.0 / count,  1.0 / count};

		for (unsigned i = 0; i <= count - 2; i += 2)
		{
			register __m128d tmp = _mm_mul_pd(a, _mm_add_pd(src[i], src[i + 1]));
			dst[i + 1] = _mm_mul_pd(a, _mm_sub_pd(src[i], src[i + 1]));
			dst[i] = tmp;
		}
	}
	else
	{
		for (unsigned i = 0; i <= count - 2; i += 2)
		{
			register __m128d tmp = _mm_add_pd(src[i], src[i + 1]);
			dst[i + 1] = _mm_sub_pd(src[i], src[i + 1]);
			dst[i] = tmp;
		}
	}
	unsigned m = count / 2;	// iterate to the half of the array size
	__m128d * dst_m = dst + m;	// half of the array, corresponds to the most significant bit of i, j
	for (unsigned i = 0, j = 0; i < m; i +=2)
	{
		// j is a bit reverse of i
		register __m128d tmp;
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
			tmp = dst_m[j + 1];
			dst_m[j + 1] = dst_m[i + 1];
			dst_m[i + 1] = tmp;
		}
		// n = 2*number of complex numbers
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

static void FastFourierTransformCoreSSE2(const std::complex<float> * complex_src, std::complex<float> * complex_dst,
										const unsigned count,
										const bool reverse_fft)
{
	const __m128 * src = reinterpret_cast<const __m128 *>(complex_src);
	__m128 * dst = reinterpret_cast<__m128 *>(complex_dst);
	// The source and destination arrays are float/double pairs (complex numbers),
	// total 2^order_power complex numbers, or 2*2^order_power of float/double items
	const unsigned half_count = count / 2;

	__m128d cs2 = { cos(M_PI / half_count), sin(M_PI / half_count) };
	if (reverse_fft) cs2.m128d_f64[1] = -cs2.m128d_f64[1];

	for (unsigned f = count / 4; f >= 1; f /= 2, src = dst)
	{
		// f iterates from 2^order_power to 4

		double z = M_PI / f;
		if (reverse_fft) z = -z;

		unsigned e = f * 2;

		__m128d uv0 = { 1., 0. };
		__m128d uv1 = cs2; // from previous step
		cs2.m128d_f64[0] = cos(z);
		cs2.m128d_f64[1] = sin(z);
		__m128d cs2_swap = {cs2.m128d_f64[1], cs2.m128d_f64[0]};

		for (unsigned j = 0; j < f; j++)
		{
			// on the first pass, j goes from 0 to half array size, for each complex number
			// on the second pass, j goes from 0 to quarter array size, for each complex number
			// etc
			__m128d uv0_re = _mm_shuffle_pd(uv0, uv0, 0);	// duplicated real
			__m128d uv0_im = _mm_shuffle_pd(uv0, uv0, 3);	// duplicated imag
			__m128d uv1_re = _mm_shuffle_pd(uv1, uv1, 0);	// duplicated real
			__m128d uv1_im = _mm_shuffle_pd(uv1, uv1, 3);	// duplicated imag

			for(unsigned i = j; i < half_count; i += e)
			{
				const __m128 *srci = src + i;
				__m128 *dsti = dst + i;
				// on the first j pass, i goes from j for each complex number
				register __m128 rt = _mm_sub_ps(srci[0], srci[f]);
				register __m128d rt_d0 = _mm_cvtps_pd(rt);
				dsti[0] = _mm_add_ps(srci[0], srci[f]);	// two complex numbers at once
				rt_d0 = _mm_addsub_pd(_mm_mul_pd(rt_d0, uv0_re), _mm_mul_pd(_mm_shuffle_pd(rt_d0, rt_d0, 1), uv0_im));
				rt = _mm_shuffle_ps(rt, rt, (2 << 0) | (3 << 2) | (2 << 4) | (3 << 6));
				register __m128d rt_d1 = _mm_cvtps_pd(rt);
				rt_d1 = _mm_addsub_pd(_mm_mul_pd(rt_d1, uv1_re), _mm_mul_pd(_mm_shuffle_pd(rt_d1, rt_d1, 1), uv1_im));
				rt = _mm_cvtpd_ps(rt_d0);
				rt = _mm_shuffle_ps(rt, _mm_cvtpd_ps(rt_d1), (0 << 0) | (1 << 2) | (0 << 4) | (1 << 6));
				dsti[f] = rt;
			} // i-loop
			uv0 = _mm_addsub_pd(_mm_mul_pd(cs2, uv0_re), _mm_mul_pd(cs2_swap, uv0_im));
			uv1 = _mm_addsub_pd(_mm_mul_pd(cs2, uv1_re), _mm_mul_pd(cs2_swap, uv1_im));
		}  // j - loop
	} // f - loop

	// last pass, each couple of complex numbers goes through add/subtract
	if (reverse_fft)
	{
		__m128 a = { 1.0f / count,  1.0f / count,  1.0f / count,  1.0f / count};

		for (unsigned i = 0; i < half_count; i ++)
		{
			register __m128 tmp1 = _mm_mul_ps(a, src[i]);
			register __m128 tmp2 = tmp1;
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, (0 << 0) | (0 << 2) | (1 << 4) | (1 << 6));
			tmp2 = _mm_shuffle_ps(tmp2, tmp2, (2 << 0) | (2 << 2) | (3 << 4) | (3 << 6));
			tmp1 = _mm_addsub_ps(tmp1, tmp2);
			dst[i] = _mm_shuffle_ps(tmp1, tmp1, (1 << 0) | (3 << 2) | (0 << 4) | (2 << 6));
		}
	}
	else
	{
		for (unsigned i = 0; i < half_count; i ++)
		{
			register __m128 tmp1 = src[i];
			register __m128 tmp2 = tmp1;
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, (0 << 0) | (0 << 2) | (1 << 4) | (1 << 6));
			tmp2 = _mm_shuffle_ps(tmp2, tmp2, (2 << 0) | (2 << 2) | (3 << 4) | (3 << 6));
			tmp1 = _mm_addsub_ps(tmp1, tmp2);
			dst[i] = _mm_shuffle_ps(tmp1, tmp1, (1 << 0) | (3 << 2) | (0 << 4) | (2 << 6));
		}
	}
	unsigned m = count / 2;	// iterate to the half of the array size
	__int64 * dst64_0 = (__int64 *)dst;
	__int64 * dst64_1 = dst64_0 + m;	// half of the array, corresponds to the most significant bit of i, j
	for (unsigned i = 0, j = 0; i < m; i +=2)
	{
		// j is a bit reverse of i
		register __int64 tmp;
		// exchange 0..i..1 and 1..j..0
		tmp = dst64_1[j];
		dst64_1[j] = dst64_0[i + 1];
		dst64_0[i + 1] = tmp;

		if (i > j)
		{
			// exchange 0..i..0 and 0..j..0
			tmp = dst64_0[j];
			dst64_0[j] = dst64_0[i];
			dst64_0[i] = tmp;
			// exchange 1..i..1 and 1..j..1
			tmp = dst64_1[j + 1];
			dst64_1[j + 1] = dst64_1[i + 1];
			dst64_1[i + 1] = tmp;
		}
		// n = 2*number of complex numbers
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

//inline implementation of a generic complex->complex fft function
template<class T>
void FastFourierTransformCore(const std::complex<T> * src, std::complex<T> * dst,
							const unsigned count,
							const bool reverse_fft)
{
	static bool CanUseSSE2 = true;
	if (CanUseSSE2
		&& 0 == (uintptr_t(src) & 15)
		&& 0 == (uintptr_t(dst) & 15))
	{
		FastFourierTransformCoreSSE2(src, dst, count, reverse_fft);
		return;
	}
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
		T a = T(1.0 / count);
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

} // namespace FFT

template<class T> void FastFourierTransform(std::complex<T> * x, unsigned count)
{
	using namespace FFT;

	ASSERT(count >= 2 && count < 0x08000000
			&& count == (count & (0-count))
			&& x != NULL);
	FastFourierTransformCore(x, x, count, false);
}

template<class T> void FastInverseFourierTransform(std::complex<T> * x, unsigned count)
{
	using namespace FFT;

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
	using namespace FFT;

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
	using namespace FFT;

	ASSERT(count >= 4 && count < 0x08000000
			&& count == (count & (0-count))
			&& src != NULL && dst != NULL);

	IFFTPreProc(src, reinterpret_cast<std::complex<T> *>(dst), count / 2);

	FastFourierTransformCore(reinterpret_cast<std::complex<T> *>(dst), reinterpret_cast<std::complex<T> *>(dst), count / 2, true);
}
