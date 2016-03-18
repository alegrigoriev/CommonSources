// KInterlocked.h
#pragma once
#pragma warning(disable:4197)

template <typename T>
inline T InterlockedIncrementT(T * src)
{
	C_ASSERT(sizeof(LONG) == sizeof(T));
	return static_cast<T>(InterlockedIncrement(reinterpret_cast<LONG volatile*>(src)));
}

template <typename T>
inline T InterlockedIncrement64T(T * src)
{
	C_ASSERT(sizeof(__int64) == sizeof(T));
	return static_cast<T>(InterlockedIncrement64(reinterpret_cast<__int64 volatile*>(src)));
}

inline LONG InterlockedIncrementT(volatile LONG * src)
{
	return InterlockedIncrement(src);
}

inline __int64 InterlockedIncrement64T(volatile __int64 * src)
{
	return InterlockedIncrement64(src);
}

template <typename T>
inline T InterlockedDecrementT(T * src)
{
	C_ASSERT(sizeof(LONG) == sizeof(T));
	return static_cast<T>(InterlockedDecrement(reinterpret_cast<LONG volatile*>(src)));
}

template <typename T>
inline T InterlockedDecrement64T(T * src)
{
	C_ASSERT(sizeof(__int64) == sizeof(T));
	return static_cast<T>(InterlockedDecrement64(reinterpret_cast<__int64 volatile*>(src)));
}

inline LONG InterlockedDecrementT(volatile LONG * src)
{
	return InterlockedDecrement(src);
}

inline __int64 InterlockedDecrement64T(volatile __int64 * src)
{
	return InterlockedDecrement64(src);
}

template <typename T, typename T1>
inline T InterlockedExchangeT(T * src, T1 exchange)
{
	LONG tmp;
	return reinterpret_cast<T &>
			(tmp = InterlockedExchange(
										const_cast<LONG *>(& reinterpret_cast<LONG volatile &>(*src)),
										reinterpret_cast<LONG volatile &>(exchange)));
}

template <typename T>
inline T InterlockedExchangeAddT(T * src, LONG addend)
{
	return static_cast<T>
			(InterlockedExchangeAdd(
									const_cast<LONG *>(& reinterpret_cast<LONG volatile &>(*src)),
									addend));
}

template <typename T>
inline T InterlockedAddT(T * src, LONG addend)
{
	return static_cast<T>
			(addend + InterlockedExchangeAdd(
											const_cast<LONG *>(& reinterpret_cast<LONG volatile &>(*src)),
											addend));
}

template <typename T>
inline T InterlockedSubtractT(T * src, LONG addend)
{
	return static_cast<T>
			(InterlockedExchangeAdd(
									const_cast<LONG *>(& reinterpret_cast<LONG volatile &>(*src)),
									-addend) - addend);
}

template <typename T, typename T1, typename T2>
inline T InterlockedCompareExchangeT(T * src, T1 exchange, T2 compare)
{
	LONG tmp;
	return reinterpret_cast<T&>
			(tmp = InterlockedCompareExchange(
											const_cast<LONG *>(& reinterpret_cast<LONG volatile &>(*src)),
											reinterpret_cast<LONG volatile &>(exchange),
											reinterpret_cast<LONG volatile &>(compare)));
}

template <typename T, typename T1>
inline T InterlockedExchange64T(T * src, T1 exchange)
{
	__int64 tmp;
	return reinterpret_cast<T &>
			(tmp = InterlockedExchange(
										const_cast<__int64 *>(& reinterpret_cast<__int64 volatile &>(*src)),
										reinterpret_cast<__int64 volatile &>(exchange)));
}

template <typename T>
inline T InterlockedExchangeAdd64T(T * src, __int64 addend)
{
	return static_cast<T>
			(InterlockedExchangeAdd64(
									const_cast<__int64 *>(& reinterpret_cast<__int64 volatile &>(*src)),
									addend));
}

template <typename T>
inline T InterlockedAdd64T(T * src, __int64 addend)
{
	return static_cast<T>
			(addend + InterlockedExchangeAdd64(
												const_cast<__int64 *>(& reinterpret_cast<__int64 volatile &>(*src)),
												addend));
}

template <typename T>
inline T InterlockedSubtract64T(T * src, __int64 addend)
{
	return static_cast<T>
			(InterlockedExchangeAdd64(
									const_cast<__int64 *>(& reinterpret_cast<__int64 volatile &>(*src)),
									-addend) - addend);
}

template <typename T, typename T1, typename T2>
inline T InterlockedCompareExchange64T(T * src, T1 exchange, T2 compare)
{
	__int64 tmp;
	return reinterpret_cast<T&>
			(tmp = InterlockedCompareExchange64(
												const_cast<__int64 *>(& reinterpret_cast<__int64 volatile &>(*src)),
												reinterpret_cast<__int64 volatile &>(exchange),
												reinterpret_cast<__int64 volatile &>(compare)));
}

template <typename T>
inline T InterlockedCompareExchangePointerT(T * src, PVOID exchange, PVOID compare)
{
	return reinterpret_cast<T>
			(InterlockedCompareExchangePointer(& reinterpret_cast<PVOID volatile&>(*src),
												exchange,
												compare));
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedIncrementModulo(T * src, LONG modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, T(tmp + 1) % T(modulo), tmp));
	return tmp;
}

template <typename T>
static T InterlockedIncrement64Modulo(T * src, __int64 modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchange64T<T>(src, T(tmp + 1) % T(modulo), tmp));
	return tmp;
}
// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedAddModulo(T * src, LONG Addend, LONG modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, T(tmp + Addend) % T(modulo), tmp));
	return tmp;
}

template <typename T>
static T InterlockedAdd64Modulo(T * src, __int64 Addend, __int64 modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchange64T<T>(src, T(tmp + Addend) % T(modulo), tmp));
	return tmp;
}
// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedOrT(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp | operand, tmp));
	return tmp;
}

template <typename T>
static T InterlockedOr64T(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchange64T<T>(src, tmp | operand, tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedAndT(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp & operand, tmp));
	return tmp;
}

template <typename T>
static T InterlockedAnd64T(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchange64T<T>(src, tmp & operand, tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedXorT(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp ^ operand, tmp));
	return tmp;
}

template <typename T>
static T InterlockedXor64T(T * src, T operand)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchange64T<T>(src, tmp ^ operand, tmp));
	return tmp;
}

template<typename T, int sizeofT = sizeof(T)>
									struct NUM_volatile
{
	T volatile num;

	NUM_volatile(T init=0)
		: num(init)
	{
	}
	NUM_volatile & operator =(T src)
	{
		num = src;
		return *this;
	}
	operator T() const
	{
		return num;
	}

	T operator ++()
	{
		return InterlockedIncrementT<typename T volatile>( & num);
	}

	T operator ++(int)
	{
		return InterlockedIncrementT<typename T volatile>( & num) - 1;
	}

	T operator --()
	{
		return InterlockedDecrementT<typename T volatile>( & num);
	}

	T operator --(int)
	{
		return InterlockedDecrementT<typename T volatile>( & num) + 1;
	}

	T operator +=(T op)
	{
		return InterlockedAddT<typename T volatile>(& num, op);
	}
	T operator -=(T op)
	{
		return InterlockedSubtractT<typename T volatile>(& num, op);
	}

	T operator |=(T op)
	{
		return InterlockedOrT<typename T volatile>(& num, op) | op;
	}

	T Exchange_Or(T op)
	{
		return InterlockedOrT<typename T volatile>(& num, op);
	}

	T operator &=(T op)
	{
		return InterlockedAndT<typename T volatile>(& num, op) & op;
	}

	T Exchange_And(T op)
	{
		return InterlockedAndT<typename T volatile>(& num, op);
	}

	T operator ^=(T op)
	{
		return InterlockedXorT<typename T volatile>(& num, op) ^ op;
	}

	T Exchange(T src)
	{
		return InterlockedExchangeT<typename T volatile>( & num, src);
	}
	T ExchangeAdd(T src)
	{
		return InterlockedExchangeAddT<typename T volatile>( & num, src);
	}
	T CompareExchange(T src, T Comperand)
	{
		return InterlockedCompareExchangeT<typename T volatile>( & num, src, Comperand);
	}

	T IncrementModulo(T modulo)
	{
		return InterlockedIncrementModulo<typename T volatile>( & num, modulo);
	}

	T AddModulo(T Addend, T modulo)
	{
		return InterlockedAddModulo<typename T volatile>( & num, Addend, modulo);
	}
};

template<typename T>
struct NUM_volatile<T, 8>
{
	T volatile num;

	NUM_volatile(T init=0)
		: num(init)
	{
	}
	NUM_volatile & operator =(T src)
	{
		num = src;
		return *this;
	}
	operator T() const
	{
		return num;
	}

	T operator ++()
	{
		return InterlockedIncrement64T<typename T volatile>( & num);
	}

	T operator ++(int)
	{
		return InterlockedIncrement64T<typename T volatile>( & num) - 1;
	}

	T operator --()
	{
		return InterlockedDecrement64T<typename T volatile>( & num);
	}

	T operator --(int)
	{
		return InterlockedDecrement64T<typename T volatile>( & num) + 1;
	}

	T operator +=(T op)
	{
		return InterlockedAdd64T<typename T volatile>(& num, op);
	}
	T operator -=(T op)
	{
		return InterlockedSubtract64T<typename T volatile>(& num, op);
	}

	T operator |=(T op)
	{
		return InterlockedOr64T<typename T volatile>(& num, op) | op;
	}

	T Exchange_Or(T op)
	{
		return InterlockedOr64T<typename T volatile>(& num, op);
	}

	T operator &=(T op)
	{
		return InterlockedAnd64T<typename T volatile>(& num, op) & op;
	}

	T Exchange_And(T op)
	{
		return InterlockedAnd64T<typename T volatile>(& num, op);
	}

	T operator ^=(T op)
	{
		return InterlockedXor64T<typename T volatile>(& num, op) ^ op;
	}

	T Exchange(T src)
	{
		return InterlockedExchange64T<typename T volatile>( & num, src);
	}
	T ExchangeAdd(T src)
	{
		return InterlockedExchangeAdd64T<typename T volatile>( & num, src);
	}
	T CompareExchange(T src, T Comperand)
	{
		return InterlockedCompareExchange64T<typename T volatile>( & num, src, Comperand);
	}

	T IncrementModulo(T modulo)
	{
		return InterlockedIncrement64Modulo<typename T volatile>( & num, modulo);
	}

	T AddModulo(T Addend, T modulo)
	{
		return InterlockedAdd64Modulo<typename T volatile>( & num, Addend, modulo);
	}
};

typedef NUM_volatile<LONG> LONG_volatile;
typedef NUM_volatile<ULONG> ULONG_volatile;

