// DigitalSignalArray.h
#ifndef DIGITAL_SIGNAL_ARRAY__H__
#define DIGITAL_SIGNAL_ARRAY__H__
#pragma pack(push, 8)

#ifndef __AFX_H__
	#include <afx.h>
#endif

#include <complex>
#include <vector>

typedef std::complex<float> fcomplex;
typedef std::complex<double> Complex;

#define SIGNAL_COMPLEX  1
#define SIGNAL_STEREO   2
#define SIGNAL_DOUBLE   4   // otherwise float or int/short
#define SIGNAL_UNSIGNED8 8
#define SIGNAL_INT16    16

class CSignalIterator
{
protected:
	CSignalIterator() {} // cannot create explicitly
	~CSignalIterator() {} // cannot delete explicitly
public:
	virtual DWORD Flags(DWORD mask = 0xFFFFFFFF) const = 0;
	virtual void SetRetrieveIndex(int nIndex) const = 0;
	virtual int GetRetrieveIndex() const = 0;
	virtual double GetDoubleAt(int nIndex) const = 0;
	virtual double GetNextDouble() = 0;
	virtual Complex GetComplexAt(int nIndex) const = 0;
	virtual Complex GetNextComplex() = 0;

private:
	// assignment guard
	CSignalIterator(const CSignalIterator &);
	CSignalIterator & operator =(const CSignalIterator &);
};

class CSignalStoreIterator
{
protected:
	CSignalStoreIterator() {} // cannot create explicitly
	~CSignalStoreIterator() {} // cannot delete explicitly
public:
	virtual DWORD Flags(DWORD mask = 0xFFFFFFFF) const = 0;
	virtual void SetStoreIndex(int nIndex) = 0;
	virtual int GetStoreIndex() const = 0;
	virtual void SetDoubleAt(double value, int nIndex) = 0;
	virtual void SetNextDouble(double value) = 0;
	virtual void SetComplexAt(const Complex & value, int nIndex) = 0;
	virtual void SetNextComplex(const Complex & value) = 0;

private:
	// assignment guard
	CSignalStoreIterator(const CSignalStoreIterator &);
	CSignalStoreIterator & operator =(const CSignalStoreIterator &);
};

class CSignalArrayBase: public CSignalStoreIterator, public CSignalIterator
{
protected:
	DWORD dwFlags;
	int nSize;          // number of samples (before multiplying on number of channels)
	int StoreIndex;
	mutable int RetrieveIndex;

	int data_size() const { return nSize; }

public:
	CSignalArrayBase()
		:dwFlags(0)
		, StoreIndex(0)
		, RetrieveIndex(0)
		, nSize(0)
		, nChan(0)
	{
	}

	virtual ~CSignalArrayBase() {}

	virtual DWORD Flags(DWORD mask = 0xFFFFFFFF) const
	{
		return dwFlags & mask;
	}
	virtual BOOL Allocate(int Size) = 0;
	virtual void SetChannel(int) = 0;

	virtual void SetRetrieveIndex(int nIndex) const
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex <= data_size());

		RetrieveIndex = (unsigned) nIndex;
	}

	virtual void SetStoreIndex(int nIndex)
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex <= data_size());

		StoreIndex = (unsigned) nIndex;
	}
	virtual int GetRetrieveIndex() const
	{
		return RetrieveIndex;
	}
	virtual int GetStoreIndex() const
	{
		return StoreIndex;
	}

public:

	int nChan;
};

template<class T, int CHANS>
class CSignalArrayVector: public CSignalArrayBase
{
protected:
	typedef std::vector<T> vector_t;

	vector_t data;
public:
	BOOL Allocate(int Size)
	{
		ASSERT (Size > 0);
		if (Size <= nSize && Size > (nSize / 2))
		{
			// don't reallocate the array
			return TRUE;
		}
		data.resize(Size*CHANS);
		nSize = Size;
		RetrieveIndex = 0;
		StoreIndex = 0;
		return TRUE;
	}
};

template<class T, int CHANS,class Base>
class CSignalArrayMain: public Base
{
public:
	CSignalArrayMain()
	{
		ASSERT(CHANS == 1 || CHANS == 2);
	}
	~CSignalArrayMain() {}
	// functions are written as inline to allow inlining
	// with explicit class refinition

	virtual void SetDoubleAt(double value, int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		data[nIndex * CHANS + nChan] = T(value);
	}

	virtual void SetNextDouble(double value)
	{
		ASSERT(StoreIndex < data_size());
		data[StoreIndex++*CHANS + nChan] = T(value);
	}

	virtual Complex GetComplexAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan];
	}

	virtual Complex GetNextComplex()
	{
		ASSERT(RetrieveIndex < data_size());

		return data[RetrieveIndex++*CHANS + nChan];
	}

	virtual double GetDoubleAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan];
	}

	virtual double GetNextDouble()
	{
		ASSERT(RetrieveIndex < data_size());

		return data[RetrieveIndex++*CHANS + nChan];
	}

	virtual void SetComplexAt(const Complex &, int )
	{
		ASSERT(FALSE);
	}

	virtual void SetNextComplex(const Complex & )
	{
		ASSERT(FALSE);
	}
	virtual void SetChannel(int Chan)  // 0-based channel
	{
		ASSERT(Chan >= 0 && Chan < CHANS);
		nChan = Chan;
	}
};

// partial specialization for __int16
template<int CHANS, typename Base>
class CSignalArrayMain<__int16, CHANS, Base> : public Base
{
public:
	CSignalArrayMain()
	{
		ASSERT(CHANS == 1 || CHANS == 2);
		dwFlags |= SIGNAL_INT16;
	}

	~CSignalArrayMain() {}
	// functions are written as inline to allow inlining
	// with explicit class refinition

	void SetDoubleAt(double value, int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		data[nIndex * CHANS + nChan] = (value >= 0) ? __int16(value + 0.5) : __int16(value - 0.5);
	}

	void SetNextDouble(double value)
	{
		ASSERT(StoreIndex < data_size());
		data[StoreIndex++*CHANS+nChan] = (value >= 0) ? __int16(value + 0.5) : __int16(value - 0.5);
	}

	virtual Complex GetComplexAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan];
	}

	virtual Complex GetNextComplex()
	{
		ASSERT(RetrieveIndex < data_size());

		return data[RetrieveIndex++*CHANS + nChan];
	}

	virtual double GetDoubleAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan];
	}

	virtual double GetNextDouble()
	{
		ASSERT(RetrieveIndex < data_size());

		return data[RetrieveIndex++*CHANS + nChan];
	}

	virtual void SetComplexAt(const Complex &, int )
	{
		ASSERT(FALSE);
	}

	virtual void SetNextComplex(const Complex & )
	{
		ASSERT(FALSE);
	}
	virtual void SetChannel(int Chan)  // 0-based channel
	{
		ASSERT(Chan >= 0 && Chan < CHANS);
		nChan = Chan;
	}
};

// partial specialization for unsigned __int8
template<int CHANS, typename Base>
class CSignalArrayMain<unsigned __int8, CHANS, Base> : public Base
{
public:
	CSignalArrayMain()
	{
		ASSERT(CHANS == 1 || CHANS == 2);
		dwFlags |= SIGNAL_UNSIGNED8;
	}

	~CSignalArrayMain() {}
	// functions are written as inline to allow inlining
	// with explicit class refinition
	virtual double GetDoubleAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return (int(data[nIndex * CHANS + nChan]) - 128) << 8;
	}

	virtual double GetNextDouble()
	{
		ASSERT(RetrieveIndex < data_size());
		return (int(data[nChan + CHANS * RetrieveIndex++]) - 128) << 8;
	}

	virtual Complex GetComplexAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return float((int(data[nIndex * CHANS + nChan]) - 128) << 8);
	}

	virtual Complex GetNextComplex()
	{
		ASSERT(RetrieveIndex < data_size());

		float tmp =(int(data[RetrieveIndex++*CHANS + nChan]) - 128) * 256.f;
		return tmp;
	}

	virtual void SetDoubleAt(double value, int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		data[nIndex * CHANS + nChan] = int(value + 128.5);
	}

	virtual void SetNextDouble(double value)
	{
		ASSERT(StoreIndex < data_size());
		data[StoreIndex++*CHANS+nChan] = int(value + 128.5);
	}

	virtual void SetComplexAt(const Complex &, int )
	{
		ASSERT(FALSE);
	}

	virtual void SetNextComplex(const Complex & )
	{
		ASSERT(FALSE);
	}
	virtual void SetChannel(int Chan)  // 0-based channel
	{
		ASSERT(Chan >= 0 && Chan < CHANS);
		nChan = Chan;
	}
};

template<int CHANS,class Base, typename T>
class CSignalArrayMain<std::complex<T>, CHANS, Base>: public Base
{
public:
	CSignalArrayMain()
	{
		ASSERT(CHANS == 1 || CHANS == 2);
		dwFlags |= SIGNAL_COMPLEX | SIGNAL_DOUBLE;
	}
	~CSignalArrayMain() {}
	// functions are written as inline to allow inlining
	// with explicit class refinition

	virtual void SetDoubleAt(double value, int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		data[nIndex * CHANS + nChan] = Complex((T)value);
	}

	virtual void SetNextDouble(double value)
	{
		ASSERT(StoreIndex < data_size());
		data[StoreIndex++*CHANS + nChan] = (T)value;
	}

	virtual Complex GetComplexAt(int nIndex) const
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan];
	}

	virtual Complex GetNextComplex()
	{
		ASSERT(RetrieveIndex < data_size());

		return data[RetrieveIndex++*CHANS + nChan];
	}

	virtual double GetDoubleAt(int nIndex) const
	{
		ASSERT(FALSE);
		ASSERT(nIndex >= 0 && nIndex < data_size());
		return data[nIndex * CHANS + nChan].real();
	}

	virtual double GetNextDouble()
	{
		ASSERT(FALSE);
		ASSERT(RetrieveIndex < data_size());
		return data[RetrieveIndex++*CHANS + nChan].real();
	}

	virtual void SetComplexAt(const Complex & value, int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < data_size());
		data[nIndex * CHANS + nChan] = value;
	}

	virtual void SetNextComplex(const Complex & value)
	{
		ASSERT(StoreIndex < data_size());
		data[StoreIndex++*CHANS + nChan] = value;
	}

	virtual void SetChannel(int Chan)  // 0-based channel
	{
		ASSERT(Chan >= 0 && Chan < CHANS);
		nChan = Chan;
	}
};

template<class T, int CHANS=1>
class CSignalArray: public CSignalArrayMain<T, CHANS, CSignalArrayVector<T, CHANS> >
{
public:
	CSignalArray() {}
	~CSignalArray() {}
};

template<typename T, int CHANS>
class CMappedSignalArrayBase : public CSignalArrayBase
{
protected:
	CMappedSignalArrayBase()
		: data(NULL)
	{
	}

	T * data;
};

template <typename T, int CHANS>
class CMappedSignalArray : public CSignalArrayMain<T, CHANS, CMappedSignalArrayBase<T, CHANS> >
{
public:
	virtual BOOL Allocate(int Size)
	{
		if (data != NULL)
		{
			nSize = Size;
			RetrieveIndex = 0;
			StoreIndex = 0;
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
	}
	CMappedSignalArray(T * pBuf, int Size)
	{
		ASSERT(pBuf != NULL);
		data = pBuf;
		nSize = Size;
	}
};

#pragma pack(pop)
#endif  //#ifndef DIGITAL_SIGNAL_ARRAY__H__
