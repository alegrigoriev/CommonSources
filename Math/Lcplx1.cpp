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

#include "LComplex.h"


ldouble  __stdcall abs(const lcomplex & z)
{
	return (sqrt(z.re*z.re + z.im*z.im));
}

ldouble  __stdcall norm(const lcomplex & z)
{
	return (z.re*z.re + z.im*z.im);
}

lcomplex &  lcomplex::operator*=(const lcomplex & z2)
{
	ldouble temp_re = re*z2.re - im*z2.im;
	ldouble temp_im = im*z2.re + re*z2.im;

	re = temp_re;
	im = temp_im;
	return *this;
}

lcomplex  __stdcall operator*(const lcomplex & z1, const lcomplex & z2)
{
	ldouble temp_re = z1.re*z2.re - z1.im*z2.im;
	ldouble temp_im = z1.im*z2.re + z1.re*z2.im;
	return lcomplex(temp_re, temp_im);
}

lcomplex & lcomplex::operator/=(const lcomplex & z2)
{
	ldouble sum_sqrs = norm(z2);
	*this *= conj(z2);
	re /= sum_sqrs;
	im /= sum_sqrs;
	return *this;
}

lcomplex __stdcall operator/(const lcomplex & z1, const lcomplex & z2)
{
	return z1 * conj(z2) / norm(z2);
}

lcomplex __stdcall operator/(ldouble re_val1, const lcomplex & z2)
{
	return re_val1 * conj(z2) / norm(z2);
}
