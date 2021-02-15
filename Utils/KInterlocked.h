// KInterlocked.h
#pragma once
#include <intrin.h>

template <typename T>
inline T InterlockedIncrementT(volatile T* src) noexcept
{
	static_assert(sizeof(long) == sizeof(T), "InterlockedIncrementT: Wrong operand size");
	return static_cast<T>(_InterlockedIncrement(reinterpret_cast<long volatile*>(src)));
}

template <typename T>
inline T InterlockedDecrementT(volatile T* src) noexcept
{
	static_assert(sizeof(long) == sizeof(T), "InterlockedDecrementT: Wrong operand size");
	return static_cast<T>(_InterlockedDecrement(reinterpret_cast<long volatile*>(src)));
}

template <typename T, typename T1>
inline T InterlockedExchangeT(volatile T* src, T1 exchange) noexcept
{
	long tmp;
	return reinterpret_cast<T&>
			(tmp = _InterlockedExchange(
										const_cast<long*>(&reinterpret_cast<long volatile&>(*src)),
										reinterpret_cast<long volatile&>(exchange)));
}

template <typename T>
inline T InterlockedExchangeAddT(volatile T* src, long addend) noexcept
{
	return static_cast<T>
			(_InterlockedExchangeAdd(
									const_cast<long*>(&reinterpret_cast<long volatile&>(*src)),
									addend));
}

template <typename T>
inline T InterlockedAddT(volatile T* src, long addend) noexcept
{
	return static_cast<T>
			(addend + _InterlockedExchangeAdd(
											const_cast<long*>(&reinterpret_cast<long volatile&>(*src)),
											addend));
}

template <typename T>
inline T InterlockedSubtractT(volatile T* src, long addend) noexcept
{
	return static_cast<T>
			(_InterlockedExchangeAdd(
									const_cast<long*>(&reinterpret_cast<long volatile&>(*src)),
									-addend) - addend);
}

template <typename T, typename T1, typename T2>
inline T InterlockedCompareExchangeT(volatile T* src, T1 exchange, T2 compare) noexcept
{
	long tmp;
	return reinterpret_cast<T&>
			(tmp = _InterlockedCompareExchange(
												const_cast<long*>(&reinterpret_cast<long volatile&>(*src)),
												reinterpret_cast<long volatile&>(exchange),
												reinterpret_cast<long volatile&>(compare)));
}

template <typename T>
inline T InterlockedCompareExchangePointerT(volatile T* src, void* exchange, void* compare) noexcept
{
	return reinterpret_cast<T>
			(_InterlockedCompareExchangePointer(&reinterpret_cast<void* volatile&>(*src),
												exchange,
												compare));
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedIncrementModulo(volatile T* src, long modulo) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, T(tmp + 1) % T(modulo), tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedAddModulo(volatile T* src, long Addend, long modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, T(tmp + Addend) % T(modulo), tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedOrT(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp | operand, tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedAndT(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp & operand, tmp));
	return tmp;
}

// the function returns previous contents of the memory location being modified
template <typename T>
static T InterlockedXorT(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	}
	while (tmp != InterlockedCompareExchangeT<T>(src, tmp ^ operand, tmp));
	return tmp;
}

template<typename T, int sizeofT = sizeof(T)>
									struct NUM_volatile
{
	T volatile num;

	NUM_volatile(T init = 0) noexcept
		: num(init)
	{
	}
	NUM_volatile& operator =(T src) noexcept
	{
		num = src;
		return *this;
	}
	operator T() const noexcept
	{
		return num;
	}

	T operator ++() noexcept
	{
		return InterlockedIncrementT<T>(&num);
	}

	T operator ++(int) noexcept
	{
		return InterlockedIncrementT<T>(&num) - 1;
	}

	T operator --() noexcept
	{
		return InterlockedDecrementT<T>(&num);
	}

	T operator --(int) noexcept
	{
		return InterlockedDecrementT<T>(&num) + 1;
	}

	T operator +=(T op) noexcept
	{
		return InterlockedAddT<T>(&num, op);
	}
	T operator -=(T op) noexcept
	{
		return InterlockedSubtractT<T>(&num, op);
	}

	T operator |=(T op) noexcept
	{
		return InterlockedOrT<T>(&num, op) | op;
	}

	T Exchange_Or(T op) noexcept
	{
		return InterlockedOrT<T>(&num, op);
	}

	T operator &=(T op) noexcept
	{
		return InterlockedAndT<T>(&num, op) & op;
	}

	T Exchange_And(T op) noexcept
	{
		return InterlockedAndT<T>(&num, op);
	}

	T operator ^=(T op) noexcept
	{
		return InterlockedXorT<T>(&num, op) ^ op;
	}

	T Exchange_Xor(T op) noexcept
	{
		return InterlockedXorT<T>(&num, op);
	}

	T Exchange(T src) noexcept
	{
		return InterlockedExchangeT<T>(&num, src);
	}
	T ExchangeAdd(T src) noexcept
	{
		return InterlockedExchangeAddT<T>(&num, src);
	}
	T CompareExchange(T src, T Comperand) noexcept
	{
		return InterlockedCompareExchangeT<T>(&num, src, Comperand);
	}

	T IncrementModulo(T modulo)
	{
		return InterlockedIncrementModulo<T>(&num, modulo);
	}

	T AddModulo(T Addend, T modulo) noexcept
	{
		return InterlockedAddModulo<T>(&num, Addend, modulo);
	}
};

#if _WIN32_WINNT >= _WIN32_WINNT_WIN7
template <typename T, typename T1, typename T2>
inline T InterlockedCompareExchange64T(volatile T* src, T1 exchange, T2 compare) noexcept
{
	__int64 tmp;
	return reinterpret_cast<T&>
			(tmp = _InterlockedCompareExchange64(
												const_cast<__int64*>(&reinterpret_cast<__int64 volatile&>(*src)),
												reinterpret_cast<__int64 volatile&>(exchange),
												reinterpret_cast<__int64 volatile&>(compare)));
}

#if defined(_M_ARM) || defined(_M_X64) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
template <typename T>
inline T InterlockedIncrement64T(volatile T* src) noexcept
{
	static_assert(sizeof(__int64) == sizeof(T), "InterlockedIncrement64T: Wrong operand size");
	return static_cast<T>(InterlockedIncrement64(reinterpret_cast<__int64 volatile*>(src)));
}

template <typename T>
inline T InterlockedDecrement64T(volatile T* src) noexcept
{
	static_assert(sizeof(__int64) == sizeof(T), "InterlockedDecrement64T: Wrong operand size");
	return static_cast<T>(_InterlockedDecrement64(reinterpret_cast<__int64 volatile*>(src)));
}

template <typename T, typename T1>
inline T InterlockedExchange64T(volatile T* src, T1 exchange) noexcept
{
	__int64 tmp;
	return reinterpret_cast<T&>
			(tmp = _InterlockedExchange64(
										const_cast<__int64*>(&reinterpret_cast<__int64 volatile&>(*src)),
										reinterpret_cast<__int64 volatile&>(exchange)));
}

template <typename T>
inline T InterlockedExchangeAdd64T(volatile T* src, __int64 addend) noexcept
{
	return static_cast<T>
			(_InterlockedExchangeAdd64(
										const_cast<__int64*>(&reinterpret_cast<__int64 volatile&>(*src)),
										addend));
}

template <typename T>
inline T InterlockedAdd64T(volatile T* src, __int64 addend) noexcept
{
	return static_cast<T>
			(addend + _InterlockedExchangeAdd64(
												const_cast<__int64*>(&reinterpret_cast<__int64 volatile&>(*src)),
												addend));
}

template <typename T>
inline T InterlockedSubtract64T(volatile T* src, __int64 addend) noexcept
{
	return static_cast<T>
			(_InterlockedExchangeAdd64(
										const_cast<__int64*>(&reinterpret_cast<__int64 volatile&>(*src)),
										-addend) - addend);
}
#else
template <typename T, typename T1>
inline T InterlockedExchange64T(volatile T* src, T1 exchange) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, exchange, tmp));
	return tmp;
}

template <typename T>
inline T InterlockedExchangeAdd64T(volatile T* src, __int64 addend) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, tmp + addend, tmp));
	return tmp;
}
template <typename T>
inline T InterlockedIncrement64T(volatile T* src) noexcept
{
	static_assert(sizeof(__int64) == sizeof(T), "InterlockedIncrement64T: Wrong operand size");
	return InterlockedExchangeAdd64T(src, 1) + 1;
}

template <typename T>
inline T InterlockedDecrement64T(volatile T* src) noexcept
{
	static_assert(sizeof(__int64) == sizeof(T), "InterlockedDecrement64T: Wrong operand size");
	return InterlockedExchangeAdd64T(src, -1) - 1;
}

template <typename T>
inline T InterlockedAdd64T(volatile T* src, __int64 addend) noexcept
{
	return InterlockedExchangeAdd64T(src, addend) + addend;
}

template <typename T>
inline T InterlockedSubtract64T(volatile T* src, __int64 addend) noexcept
{
	return InterlockedExchangeAdd64T(src, -addend) - addend;
}
#endif

template <typename T>
static T InterlockedIncrement64Modulo(volatile T* src, __int64 modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, T(tmp + 1) % T(modulo), tmp));
	return tmp;
}

template <typename T>
static T InterlockedAdd64Modulo(volatile T* src, __int64 Addend, __int64 modulo)
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, T(tmp + Addend) % T(modulo), tmp));
	return tmp;
}
template <typename T>
static T InterlockedOr64T(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, tmp | operand, tmp));
	return tmp;
}

template <typename T>
static T InterlockedAnd64T(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, tmp & operand, tmp));
	return tmp;
}

template <typename T>
static T InterlockedXor64T(volatile T* src, T operand) noexcept
{
	T tmp;
	do
	{
		tmp = *src;
	} while (tmp != InterlockedCompareExchange64T<T>(src, tmp ^ operand, tmp));
	return tmp;
}

template<typename T>
struct NUM_volatile<T, 8>
{
	T volatile num;

	NUM_volatile(T init = 0) noexcept
		: num(init)
	{
	}
	NUM_volatile& operator =(T src) noexcept
	{
		num = src;
		return *this;
	}
	operator T() const noexcept
	{
		return num;
	}

	T operator ++() noexcept
	{
		return InterlockedIncrement64T<T>(&num);
	}

	T operator ++(int) noexcept
	{
		return InterlockedIncrement64T<T>(&num) - 1;
	}

	T operator --() noexcept
	{
		return InterlockedDecrement64T<T>(&num);
	}

	T operator --(int) noexcept
	{
		return InterlockedDecrement64T<T>(&num) + 1;
	}

	T operator +=(T op) noexcept
	{
		return InterlockedAdd64T<T>(&num, op);
	}
	T operator -=(T op) noexcept
	{
		return InterlockedSubtract64T<T>(&num, op);
	}

	T operator |=(T op) noexcept
	{
		return InterlockedOr64T<T>(&num, op) | op;
	}

	T Exchange_Or(T op) noexcept
	{
		return InterlockedOr64T<T>(&num, op);
	}

	T operator &=(T op) noexcept
	{
		return InterlockedAnd64T<T>(&num, op) & op;
	}

	T Exchange_And(T op) noexcept
	{
		return InterlockedAnd64T<T>(&num, op);
	}

	T operator ^=(T op) noexcept
	{
		return InterlockedXor64T<T>(&num, op) ^ op;
	}

	T Exchange_Xor(T op) noexcept
	{
		return InterlockedXor64T<T>(&num, op);
	}

	T Exchange(T src) noexcept
	{
		return InterlockedExchange64T<T>(&num, src);
	}
	T ExchangeAdd(T src) noexcept
	{
		return InterlockedExchangeAdd64T<T>(&num, src);
	}
	T CompareExchange(T src, T Comperand) noexcept
	{
		return InterlockedCompareExchange64T<T>(&num, src, Comperand);
	}

	T IncrementModulo(T modulo)
	{
		return InterlockedIncrement64Modulo<T>(&num, modulo);
	}

	T AddModulo(T Addend, T modulo)
	{
		return InterlockedAdd64Modulo<T>(&num, Addend, modulo);
	}
};
#endif
template<typename T>
struct NUM_volatile<T*, sizeof (void*)>
{

};

typedef NUM_volatile<long> LONG_volatile;
typedef NUM_volatile<unsigned long> ULONG_volatile;
typedef NUM_volatile<unsigned long long> ULONGLONG_volatile;

