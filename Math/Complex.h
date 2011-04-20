/* Complex.h

 Complex Number Library - Include File
 class Complex: declarations for Complex numbers.

All function names, member names, and operators have been borrowed
from AT&T C++, except for the addition of:

 friend Complex acos(Complex &);
 friend Complex asin(Complex &);
 friend Complex atan(Complex &);
 friend Complex log10(Complex &);
 friend Complex tan(Complex &);
 friend Complex tanh(Complex &);
 Complex operator+();
 Complex operator-();
*/

/*
 *  C/C++ Run Time Library - Version 6.5
 *
 *  Copyright (c) 1990, 1994 by Borland International
 *  All Rights Reserved.
 *
 */

#pragma once
// Complex stream I/O
#include <complex>
typedef std::complex<double> Complex;



