// PolyMath.h
#pragma once

 #include "assert.h"
#if defined(_WINDOWS) || defined(WIN32)

 #include "afx.h"
#endif
#include <complex>

namespace PolynomialMath
{
typedef std::complex<double> Complex;
class poly;
/*
complexArray class is introduced to reduce array copy overhead
When 'poly' class is copied, we just copy its complexArray pointer
and increment its reference count to 1. When we need to modify
the array we allocate new copy only if the reference count is
greater than 1.
*/

class complexArray
{
	friend class poly;
	friend class polyRoots;
public:
	complexArray()
	{
		iRefCount=1;
		pArray = NULL;
		iAllocatedSize = 0;
	}
	complexArray(const complexArray&);
	operator Complex*() { return pArray; }
	operator const Complex*() const { return pArray; }
	friend void SafeDelete(complexArray *);
	void Allocate(int);
	int AllocatedSize() const { return iAllocatedSize; }
	void IncRefCount() { ++iRefCount; }
	void DecRefCount() { --iRefCount; }

protected:
	~complexArray();
	complexArray * MakeUnique();
	int iRefCount;
	Complex *pArray;
	int iAllocatedSize;
};

class poly
{
public:
	poly(const poly&);
	explicit poly(int iInitOrder=0, Complex first=0., int iMaxOrder=256);
	poly(const Complex *roots, int Order=0, Complex first=1., int iMaxOrder=256);
	poly(const polyRoots & roots, Complex first=1., int iMaxOrder = 256);
	// Binary Operator Functions
	friend poly operator+(const poly &, const poly &);
	friend poly operator+(double, const poly &);
	friend poly operator+(const poly &, double);
	friend poly operator+(const poly &, const Complex&);
	friend poly operator+(const Complex&, const poly &);
	friend poly operator-(const poly &, const poly &);
	friend poly operator-(double, const poly &);
	friend poly operator-(const poly &, double);
	friend poly operator-(const poly &, const Complex&);
	friend poly operator-(const Complex&, const poly &);
	friend poly operator*(const poly &, const poly &);
	friend poly operator*(double, const poly &);
	friend poly operator*(const poly &, double);
	friend poly operator*(const poly &, const Complex&);
	friend poly operator*(const Complex&, const poly &);
	friend poly operator/(const poly &, const poly &);
	friend poly operator%(const poly &, const poly &);
	friend poly operator/(const poly &, double);
	friend poly operator/(const poly &, const Complex&);
	friend poly operator<<(const poly &, int);
	friend poly operator>>(const poly &, int);
	friend int operator==(const poly &, const poly &);
	friend int operator!=(const poly &, const poly &);
	friend void PolyDiv(poly *quot, poly *rem, const poly &numer, const poly &denom);
	Complex eval(Complex arg) const;
	Complex operator ()(Complex x) const { return eval(x); }
	poly deriv(void) const;
	poly orig(void) const;
	void ScaleRoots(const Complex & scale);

	polyRoots roots(Complex start=0., int iIter=100) const;
	void FromRoots(const polyRoots & roots, Complex first=1.);
	void FromPoints(const Complex * pArguments,
					const Complex * pValues, int nCount);
	friend void SafeDelete(complexArray *);
	//	friend
	poly & operator+=(const poly &);
	poly & operator+=(const Complex &);
	poly & operator+=(double);
	poly & operator-=(const poly &);
	poly & operator-=(const Complex &);
	poly & operator-=(double);
	poly & operator*=(const poly &);
	poly & operator*=(Complex );
	poly & operator*=(double);
	poly & operator/=(const poly &);
	poly & operator/=(const Complex &);
	poly & operator/=(double);
	poly & operator%=(const poly &);
	poly & operator<<=(int);
	poly & operator>>=(int);
	poly & operator = (const poly &);
	Complex & operator[](int i)
	{
		ASSERT (i >= 0);
		ASSERT (i <= iOrder);
		return cmArray->pArray[i]; }

	const Complex & operator[](int i) const
	{
		ASSERT (i >= 0);
		ASSERT (i <= iOrder);
		return cmArray->pArray[i]; }

	poly operator+() const;
	poly operator-() const;
	int order() const {return iOrder; }
	int IsReal() const {return isReal; }
	void SetReal(BOOL set) { isReal = set; }
	Complex *array() {return cmArray->pArray; }
	const Complex *array() const {return (const Complex*) cmArray->pArray; }
	void Allocate(int ord)
	{
		cmArray->Allocate(ord+1);
	}
	void SetOrder(int);
	~poly();
	void MakeUnique() { cmArray = cmArray->MakeUnique(); }
	BOOL IsZero() const;
	void Normalize();

#ifdef _DEBUG
	void Dump(CDumpContext & = afxDump);
#endif

protected:
	complexArray *cmArray;
	int iOrder; // polynom order (# of coeffs-1)
	int iMaxOrder;  // maximum allowed array size
	int isReal; // all coefficients are real
};

class polyRoots
{
	friend class poly;
public:
	polyRoots(const polyRoots&);
	polyRoots(int iMaxOrder=256);
	polyRoots(const Complex *roots, int Count=0, int iMaxOrder=256);
	~polyRoots();
	// Binary Operator Functions
	friend polyRoots operator+(const polyRoots &, const polyRoots &);
	friend polyRoots operator+(double, const polyRoots &);
	friend polyRoots operator+(const polyRoots &, double);
	friend polyRoots operator+(const polyRoots &, const Complex&);
	friend polyRoots operator+(const Complex&, const polyRoots &);
	friend int operator==(const polyRoots &, const polyRoots &);
	friend int operator!=(const polyRoots &, const polyRoots &);
	Complex eval(Complex arg) const;
	friend void SafeDelete(complexArray *);

	polyRoots & operator+=(const polyRoots &);
	polyRoots & operator+=(const Complex &);
	polyRoots & operator+=(double);
	polyRoots & operator = (const polyRoots &);

	Complex & operator[](int i)
	{
		ASSERT (i >= 0);
		ASSERT (i < iCount);
		return cmArray->pArray[i]; }

	const Complex & operator[](int i) const
	{
		ASSERT (i >= 0);
		ASSERT (i < iCount);
		return cmArray->pArray[i]; }

	int count() const {return iCount; }
	Complex *array() {return cmArray->pArray; }
	const Complex *array() const {return (const Complex*) cmArray->pArray; }
	void Allocate(int ord)
	{
		cmArray->Allocate(ord);
	}
	void SetCount(int);
	void MakeUnique() { cmArray = cmArray->MakeUnique(); }

#ifdef _DEBUG
	void Dump(CDumpContext & = afxDump);
#endif

protected:
	complexArray *cmArray;
	int iCount; // polynom order (# of coeffs-1)
	int iMaxCount;  // maximum allowed array size
};

std::ostream & operator<<(std::ostream &, const poly &);
//std::istream & operator>>(std::istream &, poly &);

} // namespace PolynomialMath
