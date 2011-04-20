// DigitalSignalArray.h
#ifndef DIGITAL_SIGNAL_ARRAY__H__
#define DIGITAL_SIGNAL_ARRAY__H__
#pragma pack(push, 8)

#ifndef __AFX_H__
	#include <afx.h>
#endif

#if !defined(__COMPLEX_H)
#include <Complex.h>
#endif
typedef std::complex<float> fcomplex;

#define SIGNAL_COMPLEX  1
#define SIGNAL_STEREO   2
#define SIGNAL_DOUBLE   4   // otherwise float or int/short
#define SIGNAL_UNSIGNED8 8
#define SIGNAL_INT16    16

class CSignalIterator
{
protected:
	~CSignalIterator() {} // cannot delete explicitly
	CSignalIterator() {} // cannot create explicitly
	DWORD dwFlags;
public:
	DWORD Flags(DWORD mask = 0xFFFFFFFF)
	{
		return dwFlags & mask;
	}
	virtual void SetRetrieveIndex(int nIndex) = 0;
	virtual int GetRetrieveIndex() const = 0;
	virtual double GetDoubleAt(int nIndex) const = 0;
	virtual double GetNextDouble() = 0;
	virtual Complex GetComplexAt(int nIndex) const = 0;
	virtual Complex GetNextComplex() = 0;
protected:
	virtual void SetStoreIndex(int nIndex) = 0;
	virtual int GetStoreIndex() const = 0;
	virtual void SetDoubleAt(double value, int nIndex) = 0;
	virtual void SetNextDouble(double value) = 0;
	virtual void SetComplexAt(const Complex & value, int nIndex) = 0;
	virtual void SetNextComplex(const Complex & value) = 0;

};

class CSignalStoreIterator: public CSignalIterator
{
protected:
	~CSignalStoreIterator() {} // cannot delete explicitly
	CSignalStoreIterator() {} // cannot create explicitly
public:
	using CSignalIterator::Flags;
	using CSignalIterator::SetStoreIndex;
	using CSignalIterator::GetStoreIndex;
	using CSignalIterator::SetDoubleAt;
	using CSignalIterator::SetNextDouble;
	using CSignalIterator::SetComplexAt;
	using CSignalIterator::SetNextComplex;
protected:
	using CSignalIterator::SetRetrieveIndex;
	using CSignalIterator::GetRetrieveIndex;
	using CSignalIterator::GetDoubleAt;
	using CSignalIterator::GetNextDouble;
	using CSignalIterator::GetComplexAt;
	using CSignalIterator::GetNextComplex;
};

class CSignalArrayBase: public CSignalStoreIterator
{
protected:
	void * pArray;
	int nSize;
	size_t nItemSize;
	const void * pRetrieve;
	void * pStore;
public:
	CSignalArrayBase()
		:pArray(NULL), pRetrieve(NULL), pStore(NULL),
		nSize(0), nItemSize(0)
	{
		dwFlags = 0;
	}

	virtual ~CSignalArrayBase();
	using CSignalStoreIterator::SetRetrieveIndex;
	using CSignalStoreIterator::GetRetrieveIndex;
	using CSignalStoreIterator::GetDoubleAt;
	using CSignalStoreIterator::GetNextDouble;
	using CSignalStoreIterator::GetComplexAt;
	using CSignalStoreIterator::GetNextComplex;

	virtual BOOL Allocate(int Size);
	virtual void SetChannel(int) = 0;
	int GetSize() const { return nSize; }
	size_t ItemSize() { return nItemSize; }
};

template<class T, int CHANS=1>
class CSignalArray: public CSignalArrayBase
{
protected:
	int nChan;
public:
	CSignalArray();
	// functions are written as inline to allow inlining
	// with explicit class refinition
	virtual void SetRetrieveIndex(int nIndex)
	{
		ASSERT(pArray != NULL);
		ASSERT(nIndex >= 0 && nIndex < nSize);
		pRetrieve = nChan + nIndex * CHANS + (T *) pArray;
	}
	virtual void SetStoreIndex(int nIndex)
	{
		ASSERT(pArray != NULL);
		ASSERT(nIndex >= 0 && nIndex < nSize);
		pStore = nChan + nIndex * CHANS + (T *) pArray;
	}
	virtual int GetRetrieveIndex() const
	{
		ASSERT(pArray != NULL);
		return ((const T *) pRetrieve - nChan - (T *) pArray) / CHANS;
	}
	virtual int GetStoreIndex() const
	{
		ASSERT(pArray != NULL);
		return ((T *) pStore - (T *) pArray - nChan) / CHANS;
	}
	virtual double GetDoubleAt(int nIndex) const;
	virtual double GetNextDouble();

	virtual void SetDoubleAt(double value, int nIndex);
	virtual void SetNextDouble(double value);
	virtual Complex GetComplexAt(int nIndex) const;
	virtual Complex GetNextComplex();
	virtual void SetComplexAt(const Complex & value, int nIndex);
	virtual void SetNextComplex(const Complex & value);

	virtual void SetChannel(int Chan)  // 0-based channel
	{
		ASSERT(Chan >= 0 && Chan < CHANS);
		if (pRetrieve)
		{
			pRetrieve = Chan - nChan + (const T *) pRetrieve;
		}
		if (pStore)
		{
			pStore = Chan - nChan + (T *) pStore;
		}
		nChan = Chan;
	}
};

inline CSignalArray<__int16, 1>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (__int16) * CHANS;
	dwFlags |= SIGNAL_INT16;
}

inline CSignalArray<__int16, 2>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (__int16) * 2;
	dwFlags |= SIGNAL_INT16;
}

inline CSignalArray<unsigned __int8, 1>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (unsigned __int8) * 1;
	dwFlags |= SIGNAL_UNSIGNED8;
}

inline CSignalArray<unsigned __int8, 2>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (unsigned __int8) * 2;
	dwFlags |= SIGNAL_UNSIGNED8;
}

inline double CSignalArray<unsigned __int8, 1>::GetDoubleAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return (((signed __int8 *) pArray)[nIndex * 1 + nChan] + 128) << 8;
}

inline double CSignalArray<unsigned __int8, 2>::GetDoubleAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return (((signed __int8 *) pArray)[nIndex * 2 + nChan] + 128) << 8;
}

inline double CSignalArray<unsigned __int8, 1>::GetNextDouble()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < nSize + (unsigned __int8 *)pArray);
	return ((*((const signed __int8 * & ) pRetrieve)++) + 128) << 8;
}

inline double CSignalArray<unsigned __int8, 2>::GetNextDouble()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < nSize * 2 + (unsigned __int8 *)pArray);
	const signed __int8 * pTmp = (const signed __int8 *)pRetrieve;
	pRetrieve = pTmp + 2;
	return (*pTmp + 128) << 8;
}

inline Complex CSignalArray<unsigned __int8, 1>::GetComplexAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return (((signed __int8 *) pArray)
			[nIndex * 1 + nChan] + 128) << 8;
}

inline Complex CSignalArray<unsigned __int8, 2>::GetComplexAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return (((signed __int8 *) pArray)
			[nIndex * 2 + nChan] + 128) << 8;
}

inline Complex CSignalArray<unsigned __int8, 1>::GetNextComplex()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < nSize + (unsigned __int8 *)pArray);
	return ((*((const signed __int8 * & ) pRetrieve)++) + 128) << 8;
}

inline Complex CSignalArray<unsigned __int8, 2>::GetNextComplex()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < nSize * 2 + (unsigned __int8 *)pArray);
	const signed __int8 * pTmp = (const signed __int8 *)pRetrieve;
	pRetrieve = pTmp + 2;
	return (*pTmp + 128) << 8;
}

inline void CSignalArray<unsigned __int8, 1>::SetDoubleAt(double value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((unsigned __int8 *) pArray)[nIndex * 1 + nChan] = int(value + 128.5);
}

inline void CSignalArray<unsigned __int8, 2>::SetDoubleAt(double value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((unsigned __int8 *) pArray)[nIndex * 2 + nChan] = int(value + 128.5);
}

inline void CSignalArray<unsigned __int8, 1>::SetNextDouble(double value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 1 + (unsigned __int8 *)pArray);
	*((unsigned __int8 * & ) pStore) = int(value + 128.5);
	pStore = 1 + (unsigned __int8 *) pStore;
}

inline void CSignalArray<unsigned __int8, 2>::SetNextDouble(double value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 2 + (unsigned __int8 *)pArray);
	*((unsigned __int8 * & ) pStore) = int(value + 128.5);
	pStore = 2 + (unsigned __int8 *) pStore;
}

inline void CSignalArray<__int16, 1>::SetDoubleAt(double value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	if (value >= 0)
	{
		((__int16 *) pArray)[nIndex * 1 + nChan] = __int16(value + 0.5);
	}
	else
	{
		((__int16 *) pArray)[nIndex * 1 + nChan] = __int16(value - 0.5);
	}
}

inline void CSignalArray<__int16, 2>::SetDoubleAt(double value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	if (value >= 0)
	{
		((__int16 *) pArray)[nIndex * 2 + nChan] = __int16(value + 0.5);
	}
	else
	{
		((__int16 *) pArray)[nIndex * 2 + nChan] = __int16(value - 0.5);
	}
}

inline void CSignalArray<__int16, 1>::SetNextDouble(double value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 1 + (__int16 *)pArray);
	if (value >= 0)
	{
		*((__int16 * & ) pStore)++ = __int16(value + 0.5);
	}
	else
	{
		*((__int16 * & ) pStore)++ = __int16(value - 0.5);
	}
	pStore = 1 + (__int16 *) pStore;
}

inline void CSignalArray<__int16, 2>::SetNextDouble(double value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 2 + (__int16 *)pArray);
	if (value >= 0)
	{
		*((__int16 * & ) pStore)++ = __int16(value + 0.5);
	}
	else
	{
		*((__int16 * & ) pStore)++ = __int16(value - 0.5);
	}
	pStore = 2 + (__int16 *) pStore;
}

inline CSignalArray<class Complex, 1>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (Complex) * 1;
	dwFlags |= SIGNAL_COMPLEX | SIGNAL_DOUBLE;
}

inline CSignalArray<class Complex, 2>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (Complex) * 2;
	dwFlags |= SIGNAL_COMPLEX | SIGNAL_DOUBLE;
}

inline CSignalArray<class fcomplex, 1>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (fcomplex) * 1;
	dwFlags |= SIGNAL_COMPLEX;
}

inline CSignalArray<class fcomplex, 2>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (fcomplex) * 2;
	dwFlags |= SIGNAL_COMPLEX;
}

inline double CSignalArray<class Complex, 1>::GetDoubleAt(int nIndex) const
{
	ASSERT(FALSE);
	return ((Complex *) pArray)[nIndex].re;
}

inline double CSignalArray<Complex, 1>::GetNextDouble()
{
	ASSERT(FALSE);
	return ((const Complex * & ) pRetrieve)++->re;
}

inline void CSignalArray<Complex, 1>::SetComplexAt(const Complex & value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((Complex *) pArray)[nIndex] = value;
}

inline void CSignalArray<Complex, 1>::SetNextComplex(const Complex & value)
{
	ASSERT(pStore >= pArray && pStore < nSize + (Complex*)pArray);
	*((Complex * & ) pStore)++ = value;
}

inline double CSignalArray<class Complex, 2>::GetDoubleAt(int nIndex) const
{
	ASSERT(FALSE);
	return ((Complex *) pArray)[nIndex * 2].re;
}

inline double CSignalArray<Complex, 2>::GetNextDouble()
{
	ASSERT(FALSE);
	const Complex * pTmp = (const Complex *) pRetrieve;
	pRetrieve = pTmp + 2;
	return pTmp->re;
}

inline void CSignalArray<Complex, 2>::SetComplexAt(const Complex & value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((Complex *) pArray)[nIndex * 2 + nChan] = value;
}

inline void CSignalArray<Complex, 2>::SetNextComplex(const Complex & value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 2 + (Complex*)pArray);
	*((Complex * ) pStore) = value;
	((Complex * & ) pStore) += 2;
}
//
inline double CSignalArray<class fcomplex, 1>::GetDoubleAt(int nIndex) const
{
	ASSERT(FALSE);
	return ((fcomplex *) pArray)[nIndex].re;
}

inline double CSignalArray<fcomplex, 1>::GetNextDouble()
{
	ASSERT(FALSE);
	return ((const fcomplex * & ) pRetrieve)++->re;
}

inline void CSignalArray<fcomplex, 1>::SetComplexAt(const Complex & value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((fcomplex *) pArray)[nIndex] = value;
}

inline void CSignalArray<fcomplex, 1>::SetNextComplex(const Complex & value)
{
	ASSERT(pStore >= pArray && pStore < nSize + (fcomplex*)pArray);
	*((fcomplex * & ) pStore)++ = value;
}

inline double CSignalArray<class fcomplex, 2>::GetDoubleAt(int nIndex) const
{
	ASSERT(FALSE);
	return ((fcomplex *) pArray)[nIndex * 2].re;
}

inline double CSignalArray<fcomplex, 2>::GetNextDouble()
{
	ASSERT(FALSE);
	const fcomplex * pTmp = (const fcomplex *) pRetrieve;
	pRetrieve = pTmp + 2;
	return pTmp->re;
}

inline void CSignalArray<fcomplex, 2>::SetComplexAt(const Complex & value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((fcomplex *) pArray)[nIndex * 2 + nChan] = value;
}

inline void CSignalArray<fcomplex, 2>::SetNextComplex(const Complex & value)
{
	ASSERT(pStore >= pArray && pStore < nSize * 2 + (fcomplex*)pArray);
	*((fcomplex * ) pStore) = value;
	((fcomplex * & ) pStore) += 2;
}

template<class T, int CHANS>
inline CSignalArray<T,CHANS>::CSignalArray()
	:nChan(0)
{
	nItemSize = sizeof (T) * CHANS;
	ASSERT(CHANS == 1 || CHANS == 2);
}

template<class T, int CHANS>
inline void CSignalArray<T,CHANS>::SetDoubleAt(double value, int nIndex)
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	((T *) pArray)[nIndex * CHANS + nChan] = T(value);
}

template<class T, int CHANS>
inline void CSignalArray<T,CHANS>::SetNextDouble(double value)
{
	ASSERT(pStore >= pArray && pStore < nSize * CHANS + (T *)pArray);
	*(T *) pStore = T(value);
	pStore = (T *) pStore + CHANS;
}

template<class T, int CHANS>
inline Complex CSignalArray<T,CHANS>::GetComplexAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return ((T *) pArray)[nIndex * CHANS + nChan];
}

template<class T, int CHANS>
inline Complex CSignalArray<T,CHANS>::GetNextComplex()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < nSize * CHANS + (T *)pArray);
	const T * pTmp = (const T *) pRetrieve;
	pRetrieve = pTmp + CHANS;
	return *pTmp;
}

template<class T, int CHANS>
inline double CSignalArray<T,CHANS>::GetDoubleAt(int nIndex) const
{
	ASSERT(pArray != NULL);
	ASSERT(nIndex >= 0 && nIndex < nSize);
	return ((T *) pArray)[nIndex * CHANS + nChan];
}

template<class T, int CHANS>
inline double CSignalArray<T,CHANS>::GetNextDouble()
{
	ASSERT(pRetrieve >= pArray && pRetrieve < (T*)pArray + nSize * CHANS);
	const T * pTmp = (const T *) pRetrieve;
	pRetrieve = pTmp + CHANS;
	return *pTmp;
}

template<class T, int CHANS>
inline void CSignalArray<T,CHANS>::SetComplexAt(const Complex &, int )
{
	ASSERT(FALSE);
}

template<class T, int CHANS>
inline void CSignalArray<T,CHANS>::SetNextComplex(const Complex & )
{
	ASSERT(FALSE);
}

template <class T, int CHANS=1>
class CMappedSignalArray : public CSignalArray<T, CHANS>
{
public:
	virtual BOOL Allocate(int Size)
	{
		if (pArray != NULL)
		{
			nSize = Size;
			return TRUE;
		}
		else
		{
			nSize = 0;
			return FALSE;
		}
	}
	virtual ~CMappedSignalArray()
	{
		pArray = NULL;  // to prevent deleting mapped memory
	}
	CMappedSignalArray(void * pBuf)
	{
		ASSERT(pBuf != NULL);
		pArray = pBuf;
	}
};

#pragma pack(pop)
#endif  //#ifndef DIGITAL_SIGNAL_ARRAY__H__
