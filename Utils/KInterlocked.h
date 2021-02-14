// KInterlocked.h
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <intrin.h>
#include <type_traits>

namespace AG {
// Helper template class, to make up for lack of function template partial specialization in C++
template<typename T, typename T1=T, typename T2=T, int sizeofT = sizeof(T)> class Interlocked
{
public:
	static T Increment(volatile T* src) = delete;

	static T Decrement(volatile T* src) = delete;

	static T Exchange(volatile T* src, T1 exchange) = delete;

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand) = delete;

	static T ExchangeAdd(volatile T* src, T1 addend) = delete;

	static T Add(volatile T* src, T1 addend) = delete;

	static T Subtract(volatile T* src, T1 addend) = delete;

	static T Or(volatile T* src, T1 operand) = delete;

	static T Xor(volatile T* src, T1 operand) = delete;

	static T And(volatile T* src, T1 operand) = delete;
	static T ModuloAdd(volatile T* src, T1 operand) = delete;
};

// Template declarations
template <typename T, int sizeofT = sizeof(T)>
									inline T InterlockedIncrementT(volatile T* src) noexcept
{
	return Interlocked<T, T, T, sizeofT>::Increment(src);
}

template <typename T, int sizeofT = sizeof(T)>
									inline T InterlockedDecrementT(volatile T* src) noexcept
{
	return Interlocked<T, T, T, sizeofT>::Decrement(src);
}

template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedExchangeT(volatile T* src, T1 exchange) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::Exchange(src, exchange);
}

template <typename T, typename T1, typename T2, int sizeofT = sizeof(T)>
			inline T InterlockedCompareExchangeT(volatile T* src, T1 exchange, T2 comperand) noexcept
{
	return Interlocked<T, T1, T2, sizeofT>::CompareExchange(src, exchange, comperand);
}

template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedExchangeAddT(volatile T* src, T1 addend) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::ExchangeAdd(src, addend);
}

template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedAddT(volatile T* src, T1 addend) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::Add(src, addend);
}

template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedSubtractT(volatile T* src, T1 addend) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::Subtract(src, addend);
}

// the function returns previous contents of the memory location being modified
template <typename T, typename T1, typename T2, int sizeofT = sizeof(T)>
															inline T InterlockedAddModulo(volatile T* src, T1 addend, T2 modulo)
{
	return Interlocked<T, T1, T2, sizeofT>::AddModulo(src, addend, modulo);
}

// the function returns previous contents of the memory location being modified
template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedIncrementModulo(volatile T* src, T1 modulo)
{
	return Interlocked<T, T1, T1, sizeofT>::AddModulo(src, 1, modulo);
}

// the function returns previous contents of the memory location being modified
// ONLY IF THE PLATFORM DOESN'T PROVIDE AN INTRINSIC
template <typename T, typename T1, int sizeofT = sizeof(T)>
												inline T InterlockedOrT(volatile T* src, T1 operand) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::Or(src, operand);
}

// the function returns previous contents of the memory location being modified
template <typename T, typename T1, int sizeofT = sizeof(T)>
static T InterlockedAndT(volatile T* src, T1 operand) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::And(src, operand);
}

// the function returns previous contents of the memory location being modified
template <typename T, typename T1, int sizeofT = sizeof(T)>
static T InterlockedXorT(volatile T* src, T1 operand) noexcept
{
	return Interlocked<T, T1, T1, sizeofT>::Xor(src, operand);
}

// Partial specializations for 32 bit integer types (signed, unsigned)
template<typename T, typename T1, typename T2>
class Interlocked<T, T1, T2, 4>
{
	typedef long TT;
public:
	static T Increment(volatile T* src) noexcept
	{
		return static_cast<T>(::_InterlockedIncrement(
													reinterpret_cast<TT volatile*>(src)));
	}

	static T Decrement(volatile T* src) noexcept
	{
		return static_cast<T>(::_InterlockedDecrement(
													reinterpret_cast<TT volatile*>(src)));
	}

	static T Exchange(volatile T* src, T1 exchange) noexcept
	{
		return static_cast<T>(::_InterlockedExchange(
													reinterpret_cast<TT volatile*>(src),
													static_cast<TT>(static_cast<T>(exchange))));
	}

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand) noexcept
	{
		return static_cast<T>(::_InterlockedCompareExchange(
															reinterpret_cast<TT volatile*>(src),
															static_cast<TT>(static_cast<T>(exchange)),
															static_cast<TT>(static_cast<T>(comperand))));
	}

	static T ExchangeAdd(volatile T* src, T1 addend) noexcept
	{
		return (T)(::_InterlockedExchangeAdd(
											reinterpret_cast<TT volatile*>(src),
											static_cast<TT>(addend)));
	}

	static T Add(volatile T* src, T1 addend) noexcept
	{
		return (T)(::_InterlockedExchangeAdd(
											reinterpret_cast<TT volatile*>(src),
											static_cast<TT>(addend)) + addend);
	}

	static T Subtract(volatile T* src, T1 addend) noexcept
	{
		return (T)(::_InterlockedExchangeAdd(
											reinterpret_cast<TT volatile*>(src),
											0 - static_cast<TT>(addend)) - addend);
	}

	static T Or(volatile T* src, T1 operand) noexcept
	{
		return static_cast<T>(::_InterlockedOr(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T Xor(volatile T* src, T1 operand) noexcept
	{
		return static_cast<T>(::_InterlockedXor(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T And(volatile T* src, T1 operand) noexcept
	{
		return static_cast<T>(::_InterlockedAnd(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T AddModulo(volatile T* src, T1 Addend, T2 modulo)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, T(tmp + Addend) % modulo, tmp));
		return tmp;
	}
};

// Partial specializations for 16 bit integer types (signed, unsigned)
template<typename T, typename T1, typename T2>
class Interlocked<T, T1, T2, 2>
{
	typedef short TT;
public:
	static T Increment(volatile T* src)
	{
		return static_cast<T>(::_InterlockedIncrement16(
														reinterpret_cast<TT volatile*>(src)));
	}

	static T Decrement(volatile T* src)
	{
		return static_cast<T>(::_InterlockedDecrement16(
														reinterpret_cast<TT volatile*>(src)));
	}

	static T Exchange(volatile T* src, T1 exchange)
	{
		return static_cast<T>(::_InterlockedExchange16(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(exchange))));
	}

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand)
	{
		return static_cast<T>(::_InterlockedCompareExchange16(
															reinterpret_cast<TT volatile*>(src),
															static_cast<TT>(static_cast<T>(exchange)),
															static_cast<TT>(static_cast<T>(comperand))));
	}

	static T ExchangeAdd(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd16(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(addend))));
	}

	static T Add(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd16(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(addend))) + addend);
	}

	static T Subtract(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd16(
														reinterpret_cast<TT volatile*>(src),
														0 - static_cast<TT>(addend)) - addend);
	}

	static T Or(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedOr16(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T Xor(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedXor16(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T And(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedAnd16(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T AddModulo(volatile T* src, T1 Addend, T2 modulo)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, T(tmp + Addend) % modulo, tmp));
		return tmp;
	}
};

// Partial specializations for 8 bit integer types (signed, unsigned)
template<typename T, typename T1, typename T2>
class Interlocked<T, T1, T2, 1>
{
	typedef char TT;
public:
	static T Increment(volatile T* src)
	{
		return Add(src, 1);
	}

	static T Decrement(volatile T* src)
	{
		return Subtract(src, 1);
	}

	static T Exchange(volatile T* src, T1 exchange)
	{
		return static_cast<T>(::_InterlockedExchange8(
													reinterpret_cast<TT volatile*>(src),
													static_cast<TT>(static_cast<T>(exchange))));
	}

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand)
	{
		return static_cast<T>(::_InterlockedCompareExchange8(
															reinterpret_cast<TT volatile*>(src),
															static_cast<TT>(static_cast<T>(exchange)),
															static_cast<TT>(static_cast<T>(comperand))));
	}

	static T ExchangeAdd(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd8(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(addend))));
	}

	static T Add(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd8(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(addend))) + addend);
	}

	static T Subtract(volatile T* src, T1 addend)
	{
		return static_cast<T>(::_InterlockedExchangeAdd8(
														reinterpret_cast<TT volatile*>(src),
														0 - static_cast<TT>(addend)) - addend);
	}

	static T Or(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedOr8(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T Xor(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedXor8(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T And(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedAnd8(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T AddModulo(volatile T* src, T1 Addend, T2 modulo)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, T(tmp + Addend) % modulo, tmp));
		return tmp;
	}
};

#if defined(_M_ARM) || defined(_M_X64) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
// Partial specializations for 64 bit integer types (signed, unsigned)
// for machines with extensive 64 bit support
template<typename T, typename T1, typename T2>
class Interlocked<T, T1, T2, 8>
{
	typedef __int64 TT;
public:
	static T Increment(volatile T* src)
	{
		return static_cast<T>(::_InterlockedIncrement64(
														reinterpret_cast<TT volatile*>(src)));
	}

	static T Decrement(volatile T* src)
	{
		return static_cast<T>(::_InterlockedDecrement64(
														reinterpret_cast<TT volatile*>(src)));
	}

	static T Exchange(volatile T* src, T1 exchange)
	{
		return static_cast<T>(::_InterlockedExchange64(
														reinterpret_cast<TT volatile*>(src),
														static_cast<TT>(static_cast<T>(exchange))));
	}

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand)
	{
		return static_cast<T>(::_InterlockedCompareExchange64(
															reinterpret_cast<TT volatile*>(src),
															static_cast<TT>(static_cast<T>(exchange)),
															static_cast<TT>(static_cast<T>(comperand))));
	}

	static T ExchangeAdd(volatile T* src, T1 addend)
	{
		return (T)(::_InterlockedExchangeAdd64(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(addend)));
	}

	static T Add(volatile T* src, T1 addend)
	{
		return (T)(::_InterlockedExchangeAdd64(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(addend)) + addend);
	}

	static T Subtract(volatile T* src, T1 addend)
	{
		return (T)(::_InterlockedExchangeAdd64(
												reinterpret_cast<TT volatile*>(src),
												0 - static_cast<TT>(addend)) - addend);
	}

	static T Or(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedOr64(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T Xor(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedXor64(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T And(volatile T* src, T1 operand)
	{
		return static_cast<T>(::_InterlockedAnd64(
												reinterpret_cast<TT volatile*>(src),
												static_cast<TT>(static_cast<T>(operand))));
	}

	static T AddModulo(volatile T* src, T1 Addend, T2 modulo)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, T(tmp + Addend) % modulo, tmp));
		return tmp;
	}
};
#else
// Partial specializations for 64 bit integer types (signed, unsigned)
// for machines with limited 64 bit support,
// Only _InterlockedCompareExchange64 is available, not even InterlockedExchange64
template<typename T, typename T1, typename T2>
class Interlocked<T, T1, T2, 8>
{
	typedef __int64 TT;
public:
	static T Increment(volatile T* src)
	{
		return Add(src, 1);
	}

	static T Decrement(volatile T* src)
	{
		return Subtract(src, 1);
	}

	static T Exchange(volatile T* src, T1 exchange)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, exchange, tmp));
		return tmp;
	}

	static T CompareExchange(volatile T* src, T1 exchange, T2 comperand)
	{
		return (T)(::_InterlockedCompareExchange64(
													reinterpret_cast<TT volatile*>(src),
													static_cast<TT>(exchange),
													static_cast<TT>(comperand)));
	}

	static T ExchangeAdd(volatile T* src, T1 addend)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, tmp + addend, tmp));
		return tmp;
	}

	static T Add(volatile T* src, T1 addend)
	{
		return static_cast<T>(ExchangeAdd(src, addend) + addend);
	}

	static T Subtract(volatile T* src, T1 addend)
	{
		return static_cast<T>(ExchangeAdd(src, (TT)0-(TT)addend) - addend);
	}

	static T Or(volatile T* src, T1 operand)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, tmp | operand, tmp));
		return tmp;
	}

	static T Xor(volatile T* src, T1 operand)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, tmp ^ operand, tmp));
		return tmp;
	}

	static T And(volatile T* src, T1 operand)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, tmp & operand, tmp));
		return tmp;
	}

	static T AddModulo(volatile T* src, T1 Addend, T2 modulo)
	{
		T tmp;
		do
		{
			tmp = *src;
		} while (tmp != CompareExchange(src, T(tmp + Addend) % modulo, tmp));
		return tmp;
	}
};
#endif

// Specialization for operations with pointers
template<typename T>
class Interlocked<T*, T*, T*, sizeof (void*)>
{
	typedef T* P;
	typedef typename std::remove_cv<T>::type *PP;
	typedef uintptr_t TT;
public:
	// Note that arithmetics cannot be performed on void*, only on typed pointers
	static P Increment(P volatile* src) noexcept
	{
		return Add(src, 1);
	}

	static P Decrement(P volatile* src) noexcept
	{
		return Subtract(src, 1);
	}

	static P ExchangeAdd(P volatile* src, ptrdiff_t addend)
	{
		return reinterpret_cast<P>(InterlockedExchangeAddT<TT, ptrdiff_t>(
										reinterpret_cast<TT volatile*>(src),
										addend * sizeof(T)));
	}

	static P Add(P volatile* src, ptrdiff_t addend)
	{
		return reinterpret_cast<P>(InterlockedExchangeAddT<TT, ptrdiff_t>(
										reinterpret_cast<TT volatile*>(src),
										addend*ptrdiff_t(sizeof (T)))) + addend;
	}

	static P Subtract(P volatile* src, size_t addend)
	{
		return reinterpret_cast<P>(InterlockedExchangeAddT<TT, ptrdiff_t>(
										reinterpret_cast<TT volatile*>(src),
										- addend * ptrdiff_t(sizeof(T)))) - addend;
	}

	static P Exchange(P volatile* src, P exchange)
	{
		return reinterpret_cast<P>
				(::_InterlockedExchangePointer(static_cast<void *volatile*>(static_cast<volatile void*>(src)),
												const_cast<PP>(exchange)));
	}

	static P CompareExchange(P volatile* src, P exchange, P compare)
	{
		return reinterpret_cast<P>
				(::_InterlockedCompareExchangePointer(
													static_cast<void* volatile*>(static_cast<volatile void*>(src)),
													const_cast<PP>(exchange), const_cast<PP>(compare)));
	}
};

template <typename T>
inline T* InterlockedExchangePointerT(T* volatile* src, T* exchange) noexcept
{
	return Interlocked<T*,T*,T*>::Exchange(src, exchange);
}

template <typename T, typename T1, typename T2>
T InterlockedCompareExchangePointerT(T* volatile* src, T1 exchange, T2 compare) noexcept
{
	return Interlocked<T*,T1,T2>::CompareExchange(src, exchange, compare);
}


template<typename T>
struct Atomic
{
	T volatile num;

	Atomic(T init = 0) noexcept
		: num(init)
	{
	}
	Atomic& operator =(T src) noexcept
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
		return InterlockedIncrementT(&num);
	}

	T operator ++(int) noexcept
	{
		return InterlockedIncrementT(&num) - 1;
	}

	T operator --() noexcept
	{
		return InterlockedDecrementT(&num);
	}

	T operator --(int) noexcept
	{
		return InterlockedDecrementT(&num) + 1;
	}

	T operator +=(T op) noexcept
	{
		return InterlockedAddT(&num, op);
	}
	T operator -=(T op) noexcept
	{
		return InterlockedSubtractT(&num, op);
	}

	T operator |=(T op) noexcept
	{
		return InterlockedOrT(&num, op) | op;
	}

	T Exchange_Or(T op) noexcept
	{
		return InterlockedOrT(&num, op);
	}

	T operator &=(T op) noexcept
	{
		return InterlockedAndT(&num, op) & op;
	}

	T Exchange_And(T op) noexcept
	{
		return InterlockedAndT(&num, op);
	}

	T operator ^=(T op) noexcept
	{
		return InterlockedXorT(&num, op) ^ op;
	}

	T Exchange_Xor(T op) noexcept
	{
		return InterlockedXorT(&num, op);
	}

	T Exchange(T src) noexcept
	{
		return InterlockedExchangeT(&num, src);
	}
	T ExchangeAdd(T src) noexcept
	{
		return InterlockedExchangeAddT(&num, src);
	}
	T CompareExchange(T src, T Comperand) noexcept
	{
		return InterlockedCompareExchangeT(&num, src, Comperand);
	}

	T IncrementModulo(T modulo) noexcept
	{
		return InterlockedIncrementModulo(&num, modulo);
	}

	T AddModulo(T Addend, T modulo)
	{
		return InterlockedAddModulo(&num, Addend, modulo);
	}
};

// A specialization for pointer argument
template<typename T>
struct Atomic<T*>
{
	typedef T* P;
	P volatile ptr;

	Atomic(P init = 0)
		: ptr(init)
	{
	}
	Atomic& operator =(P src)
	{
		ptr = src;
		return *this;
	}
	operator P() const
	{
		return ptr;
	}

	P operator ++()
	{
		return operator +=(1);
	}

	P operator ++(int)
	{
		return ExchangeAdd(1);
	}

	P operator --()
	{
		return operator -=(1);
	}

	P operator --(int)
	{
		return ExchangeAdd(-1);
	}

	P operator +=(ptrdiff_t increment)
	{
		return ExchangeAdd(increment) + increment;
	}

	P operator -=(ptrdiff_t decrement)
	{
		return ExchangeAdd(-decrement) - decrement;
	}

	P Exchange(P src)
	{
		return InterlockedExchangeT<P>(&ptr, src);
	}
	P CompareExchange(P src, P comperand)
	{
		return InterlockedCompareExchangeT<P>(&ptr, src, comperand);
	}
	P ExchangeAdd(ptrdiff_t increment)
	{
		return InterlockedExchangeAddT<P, ptrdiff_t>(&ptr, increment * ptrdiff_t(sizeof(T)));
	}

	P RingIncrement(P ring_begin, size_t ring_size, ptrdiff_t increment)
	{
		const P ring_end = ring_begin + ring_size;
		P old_ptr, new_ptr;
		do
		{
			old_ptr = ptr;
			new_ptr = old_ptr + increment;
			if (ring_size == 0)
			{
				// skip
			}
			else if (increment > 0)
			{
				while (new_ptr >= ring_end)
				{
					new_ptr -= ring_size;
				}
			}
			else if (increment < 0)
			{
				while (new_ptr < ring_begin)
				{
					new_ptr += ring_size;
				}
			}
		} while (old_ptr != CompareExchange(new_ptr, old_ptr));
		return new_ptr;
	}
};

}

typedef AG::Atomic<signed long> LONG_volatile;
typedef AG::Atomic<unsigned long> ULONG_volatile;
typedef AG::Atomic<unsigned long long> ULONGLONG_volatile;

