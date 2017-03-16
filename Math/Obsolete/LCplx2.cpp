/*-----------------------------------------------------------------------*
 * filename - lcplx2.cpp
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

static const lcomplex lcomplex_zero(0, 0);
inline ldouble sqr(ldouble x) { return x*x;}

lcomplex __stdcall acos(const lcomplex & z)
{
	// Complex i(0,1);
	// return -i*log(z+i*sqrt(1-z*z));
	lcomplex temp1(ldouble(1.) - sqr(z.re) + sqr(z.im), ldouble(-2.)*z.re*z.im);
	ldouble phi = arg(temp1)/ldouble(2.);
	ldouble rp = sqrt(abs(temp1));
	lcomplex temp2(z.re - rp*ldouble(sin(phi)), z.im + rp*ldouble(cos(phi)));
	return lcomplex(arg(temp2), -log(abs(temp2)));
}

ldouble __stdcall arg(const lcomplex & z)
{
	return z == lcomplex_zero ? 0.0 : atan2(z.im, z.re);
}

lcomplex __stdcall asin(const lcomplex & z)
{
	// Complex i(0,1);
	// return -i*log(i*z+sqrt(1-z*z));

	lcomplex temp1(ldouble(1.) - sqr(z.re) + sqr(z.im), ldouble(-2.)*z.re*z.im);
	ldouble phi = arg(temp1) / ldouble(2.);
	ldouble rp = sqrt(abs(temp1));
	lcomplex temp2(-z.im + rp*ldouble(cos(phi)), z.re + rp*ldouble(sin(phi)));
	return lcomplex(arg(temp2), -log(abs(temp2)));
}

lcomplex __stdcall atan(const lcomplex & z)
{
	// Complex i(0, 1);
	// return -0.5*i * log((1+i*z)/(1-i*z));
	// if z=i then a floating-point exception may occur

	ldouble opb = ldouble(1.) + z.im;
	ldouble a2 = sqr(z.re);
	ldouble den = opb*opb + a2;
	lcomplex temp(((ldouble(1.)-z.im)*opb - a2)/den, ldouble(2.)*z.re/den);
	return lcomplex(arg(temp)/ldouble(2.), -ldouble(log(abs(temp)))/ldouble(2.));
}

lcomplex __stdcall cos(const lcomplex & z)
{
	// Complex i(0, 1);
	// return (exp(i*z) + exp(-i*z))/2;

	ldouble eb  = exp(z.im);
	ldouble emb = ldouble(1.) / eb;
	return lcomplex(ldouble(cos(z.re))*(emb+eb)/ldouble(2.),
					ldouble(sin(z.re))*(emb-eb)/ldouble(2.));
}

lcomplex __stdcall cosh(const lcomplex & z)
{
	// return (exp(z) + exp(-z))/2;
	ldouble ea = exp(z.re);
	ldouble eainv = ldouble(1.) / ea;
	return lcomplex(ldouble(cos(z.im))*(ea + eainv)/ldouble(2.),
					ldouble(sin(z.im))*(ea - eainv)/ldouble(2.));
}

lcomplex __stdcall exp(const lcomplex & z)
{
	return polar(ldouble(exp(z.re)),z.im);
}

lcomplex __stdcall log(const lcomplex & z)
{
	// exception if z = 0
	return lcomplex(log(abs(z)), arg(z));
}

lcomplex __stdcall log10(const lcomplex & z)
{
	// return log10e*log(z);
	return log(z) * ldouble(M_LOG10E);
}

lcomplex __stdcall pow(const lcomplex & base, ldouble expon)
{
	// return exp(expon*log(base));

	if (base == lcomplex_zero && expon > 0.) return lcomplex_zero;
	return polar(ldouble(pow(double(abs(base)), double(expon))), expon*arg(base));
}

lcomplex __stdcall pow(ldouble base, const lcomplex & expon)
{
	// return exp(expon*log(base));
	if (base == ldouble(0.) && real(expon) > 0.) return lcomplex_zero;
	ldouble lnx = log(fabs(base));
	if (base > 0.0)
		return exp(expon * lnx);
	return exp(expon * lcomplex(lnx,M_PI));
}

lcomplex __stdcall pow(const lcomplex & base, const lcomplex & expon)
{
	if (base == lcomplex_zero && real(expon) > 0.) return lcomplex_zero;
	return exp(expon * log(base));
}

lcomplex __stdcall sin(const lcomplex & z)
{
	// Complex i(0,1);
	// return (exp(i*z) - exp(-i*z))/(2*i);

	ldouble eb  = exp(z.im);
	ldouble emb = ldouble(1.) / eb;
	return lcomplex(ldouble(sin(z.re))*(emb+eb)/ldouble(2.),
					ldouble(-0.5*cos(z.re))*(emb-eb));
}

lcomplex __stdcall sinh(const lcomplex & z)
{
	// return (exp(z) - exp(-z))/2
	ldouble ea = exp(z.re);
	ldouble eainv = ldouble(1.) / ea;
	return lcomplex(ldouble(cos(z.im)/2.)*(ea - eainv),
					ldouble(sin(z.im)/2.)*(ea + eainv));
}

lcomplex __stdcall sqrt(const lcomplex & z)
{
	// return pow(z, 0.5);
//    return polar(ldouble(sqrt(abs(z))), arg(z) / ldouble(2.));
	ldouble az=abs(z);
	ldouble re = sqrt((az + z.re)/ldouble(2.));
	ldouble im = sqrt((az - z.re)/ldouble(2.));
	if (z.im < 0.)
		im = -im;
	return lcomplex(re, im);
}

lcomplex __stdcall tan(const lcomplex & z)
{
	// return sin(z)/cos(z)
	ldouble sina = sin(z.re);
	ldouble cosa = cos(z.re);
	ldouble eb = exp(z.im);
	ldouble emb = ldouble(1.) / eb;
	ldouble emin = emb - eb;
	ldouble eplus = emb + eb;
	return lcomplex(ldouble(4.)*sina*cosa, -emin*eplus)
			/ (sqr(cosa*eplus) + sqr(sina*emin));
}

lcomplex __stdcall tanh(const lcomplex & z)
{
	// return sinh(z)/cosh(z);
	ldouble sinb = sin(z.im);
	ldouble cosb = cos(z.im);
	ldouble ea = exp(z.re);
	ldouble eainv = ldouble(1.) / ea;
	ldouble eamin = ea - eainv;
	ldouble eapls = ea + eainv;
	ldouble div = (sqr(cosb*eapls) + sqr(sinb*eamin));

	return lcomplex((eamin/div)*eapls, (ldouble(4.)*sinb*cosb)/div);
}


// Stream I/O function definitions

ostream & __stdcall operator<<(ostream & s, const lcomplex & z)
{
	return s << "(" << double(real(z)) << ", " << double(imag(z)) << ")";
}

istream & __stdcall operator>>(istream & s, lcomplex & z)
{
	double re_val = 0.0, im_val = 0.0;
	char c = 0;

	s >> c;
	if (c == '(') {
		s >> re_val >> c;
		if (c == ',') s >> im_val >> c;
		if (c != ')') s.clear(ios::badbit | s.rdstate());
	}
	else {
		s.putback(c);
		s >> re_val;
	}
	if (s) z = lcomplex(re_val, im_val);
	return s;
}


