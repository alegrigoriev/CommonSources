#define _USE_MATH_DEFINES
#include <math.h>
#include "MDCT.h"
#include "MDCT.inl"

namespace FFT
{
	// Instantiate templates
	void __InstantiateMdctFunctions()
	{
		float xf[2] = { 0 };
		float yf[2] = { 0 };
		WindowedMDCT(xf, yf, 1, 0);
		double xd[2] = { 0 };
		double yd[2] = { 0 };
		WindowedMDCT(xd, yd, 1, 0);
	}
}
