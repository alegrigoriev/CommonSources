/*-----------------------------------------------------------------------*
 * filename - cplx2.cpp
 * C++ Complex Library Routines
 *-----------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 1.5
 *
 *      Copyright (c) 1990, 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#include "Complex.h"

static const Complex complex_zero(0, 0);
inline double sqr(double x) { return x*x;}

Complex __stdcall acos(const Complex & z)
{
	// Complex i(0,1);
	// return -i*log(z+i*sqrt(1-z*z));
	Complex temp1(1 - sqr(z.re) + sqr(z.im), -2*z.re*z.im);
	double phi = arg(temp1)/2;
	double rp = sqrt(abs(temp1));
	Complex temp2(z.re - rp*sin(phi), z.im + rp*cos(phi));
	return Complex(arg(temp2), -log(abs(temp2)));
}

double __stdcall arg(const Complex & z)
{
	return z == complex_zero ? 0.0 : atan2(z.im, z.re);
}

Complex __stdcall asin(const Complex & z)
{
	// Complex i(0,1);
	// return -i*log(i*z+sqrt(1-z*z));

	Complex temp1(1 - sqr(z.re) + sqr(z.im), -2*z.re*z.im);
	double phi = arg(temp1) / 2;
	double rp = sqrt(abs(temp1));
	Complex temp2(-z.im + rp*cos(phi), z.re + rp*sin(phi));
	return Complex(arg(temp2), -log(abs(temp2)));
}

Complex __stdcall atan(const Complex & z)
{
	// Complex i(0, 1);
	// return -0.5*i * log((1+i*z)/(1-i*z));
	// if z=i then a floating-point exception may occur

	double opb = 1 + z.im;
	double a2 = sqr(z.re);
	double den = opb*opb + a2;
	Complex temp(((1-z.im)*opb - a2)/den, 2*z.re/den);
	return Complex(arg(temp)/2, -log(abs(temp))/2);
}

Complex __stdcall cos(const Complex & z)
{
	// Complex i(0, 1);
	// return (exp(i*z) + exp(-i*z))/2;

	long double eb  = exp(z.im);
	long double emb = 1 / eb;
	return Complex(cos(z.re)*(emb+eb)/2, sin(z.re)*(emb-eb)/2);
}

Complex __stdcall cosh(const Complex & z)
{
	// return (exp(z) + exp(-z))/2;
	long double ea = exp(z.re);
	long double eainv = 1 / ea;
	return Complex(cos(z.im)*(ea + eainv)/2, sin(z.im)*(ea - eainv)/2);
}

Complex __stdcall exp(const Complex & z)
{
	return polar(exp(z.re),z.im);
}

Complex __stdcall log(const Complex & z)
{
	// exception if z = 0
	return Complex(log(abs(z)), arg(z));
}

Complex __stdcall log10(const Complex & z)
{
	// return log10e*log(z);
	return log(z) * M_LOG10E;
}

Complex __stdcall pow(const Complex & base, double expon)
{
	// return exp(expon*log(base));

	if (base == complex_zero && expon > 0) return complex_zero;
	return polar(pow(abs(base), expon), expon*arg(base));
}

Complex __stdcall pow(double base, const Complex & expon)
{
	// return exp(expon*log(base));
	if (base == 0 && real(expon) > 0) return complex_zero;
	double lnx = log(fabs(base));
	if (base > 0.0)
		return exp(expon * lnx);
	return exp(expon * Complex(lnx,M_PI));
}

Complex __stdcall pow(const Complex & base, const Complex & expon)
{
	if (base == complex_zero && real(expon) > 0) return complex_zero;
	return exp(expon * log(base));
}

Complex __stdcall sin(const Complex & z)
{
	// Complex i(0,1);
	// return (exp(i*z) - exp(-i*z))/(2*i);

	long double eb  = exp(z.im);
	long double emb = 1 / eb;
	return Complex(sin(z.re)*(emb+eb)/2, -0.5*cos(z.re)*(emb-eb));
}

Complex __stdcall sinh(const Complex & z)
{
	// return (exp(z) - exp(-z))/2
	long double ea = exp(z.re);
	long double eainv = 1 / ea;
	return Complex(cos(z.im)*(ea - eainv)/2, sin(z.im)*(ea + eainv)/2);
}

Complex __stdcall sqrt(const Complex & z)
{
	// return pow(z, 0.5);
//    return polar(sqrt(abs(z)), arg(z) / 2);
	double az=abs(z);
	double re = sqrt((az + z.re)/2.);
	double im = sqrt((az - z.re)/2.);
	if (z.im < 0) im = -im;
	return Complex(re, im);
}

Complex __stdcall tan(const Complex & z)
{
	// return sin(z)/cos(z)
	double sina = sin(z.re);
	double cosa = cos(z.re);
	long double eb = exp(z.im);
	long double emb = 1 / eb;
	double emin = emb - eb;
	double eplus = emb + eb;
	return Complex(4*sina*cosa, -emin*eplus)
			/ (sqr(cosa*eplus) + sqr(sina*emin));
}

Complex __stdcall tanh(const Complex & z)
{
	// return sinh(z)/cosh(z);
	double sinb = sin(z.im);
	double cosb = cos(z.im);
	long double ea = exp(z.re);
	long double eainv = 1 / ea;
	double eamin = ea - eainv;
	double eapls = ea + eainv;
	long double div = (sqr(cosb*eapls) + sqr(sinb*eamin));

	return Complex((eamin/div)*eapls, (4*sinb*cosb)/div);
}


// Stream I/O function definitions

ostream & __stdcall operator<<(ostream & s, const Complex & z)
{
	return s << "(" << real(z) << ", " << imag(z) << ")";
}

istream & __stdcall operator>>(istream & s, Complex & z)
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
	if (s) z = Complex(re_val, im_val);
	return s;
}


