#define _USE_MATH_DEFINES
#include <math.h>

#include "FFT.inl"

namespace FFT
{
// Instantiate templates
void __InstantiateFunctions()
{
	float xf[2] = {0};
	std::complex<float> zf[2];
	FastFourierTransform(zf, 1, 0);
	FastFourierTransform(zf, zf, 1, 0);
	FastFourierTransform(xf, zf, 1, 0);
	FastInverseFourierTransform(zf, 1, 0);
	FastInverseFourierTransform(zf, zf, 1, 0);
	FastInverseFourierTransform(zf, xf, 1, 0);
	double xd[2] = { 0 };
	std::complex<double> zd[2];
	FastFourierTransform(zd, 1, 0);
	FastFourierTransform(zd, zd, 1, 0);
	FastFourierTransform(xd, zd, 1, 0);
	FastInverseFourierTransform(zd, 1, 0);
	FastInverseFourierTransform(zd, zd, 1, 0);
	FastInverseFourierTransform(zd, xd, 1, 0);
}
}
