/*-----------------------------------------------------------------------*
 * filename - cplx1.cpp
 * C++ Complex Library Routines
 *-----------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 1.5
 *
 *      Copyright (c) 1990, 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#include "complex.h"


inline double  __stdcall abs(const complex & z)
{
	return (sqrt(z.re*z.re + z.im*z.im));
}

inline double  __stdcall norm(const complex & z)
{
	return (z.re*z.re + z.im*z.im);
}

inline complex &  complex::operator*=(const complex & z2)
{
	double temp_re = re*z2.re - im*z2.im;
	im = im*z2.re + re*z2.im;
	re = temp_re;
	return *this;
}

inline complex  __stdcall operator*(const complex & z1, const complex & z2)
{
	return complex(z1.re*z2.re - z1.im*z2.im,
					z1.im*z2.re + z1.re*z2.im);
}

inline complex & complex::operator/=(const complex & z2)
{
	double sum_sqrs = norm(z2);
	*this = complex((z1.re*z2.re + z1.im*z2.im) / sum_sqrs,
					(z1.im*z2.re - z1.re*z2.im) / sum_sqrs);
	return *this;
}

inline complex __stdcall operator/(const complex & z1, const complex & z2)
{
	double sum_sqrs = norm(z2);
	return complex((z1.re*z2.re + z1.im*z2.im) / sum_sqrs,
					(z1.im*z2.re - z1.re*z2.im) / sum_sqrs);
}

inline complex __stdcall operator/(double re_val1, const complex & z2)
{
	double sum_sqrs = norm(z2);
	return complex((re_val1 * z2.re / sum_sqrs,
						- re_val1 * z2.im / sum_sqrs);
}

