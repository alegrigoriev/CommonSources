#pragma once

inline unsigned MultiSzLen(LPCSTR src)
{
	size_t len = 0;
	size_t len1;
	while (0 != (len1 = strlen(src + len)))
	{
		len += len1 + 1;
	}
	return (unsigned)len;
}

inline unsigned MultiSzLen(LPCWSTR src)
{
	size_t len = 0;
	size_t len1;
	while (0 != (len1 = wcslen(src + len)))
	{
		len += len1 + 1;
	}
	return (unsigned)len;
}

inline void AssignMultiSz(CStringW & dst, LPCSTR src)
{
	dst = CStringW(src, MultiSzLen(src));
}

inline void AssignMultiSz(CStringW & dst, LPCWSTR src)
{
	dst.SetString(src, MultiSzLen(src));
}

inline void AssignMultiSz(CStringA & dst, LPCSTR src)
{
	dst.SetString(src, MultiSzLen(src));
}

inline void AssignMultiSz(CStringA & dst, LPCWSTR src)
{
	dst = CStringA(src, MultiSzLen(src));
}

