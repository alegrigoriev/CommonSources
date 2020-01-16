#include "StdAfx.h"
#include "LocaleUtilities.h"

LocaleParameters LocaleParameters::singleton;

// long to string, thousands separated by commas
CString LtoaCS(LONGLONG num)
{
	TCHAR s[32];
	CString s1;
	TCHAR * p = s;
	LPCTSTR ThousandSeparator = LocaleParameters::ThousandSeparator();
	_i64tot_s(num, s, 20, 10);
	if (0 == ThousandSeparator[0])
	{
		return s;
	}
	if ('-' == p[0])
	{
		s1 += '-';
		p++;
	}
	unsigned len = (unsigned)_tcslen(p);
	unsigned first = len % 3;

	if (0 == first && len > 0)
	{
		first = 3;
	}
	s1.Append(p, first);
	p += first;

	len -= first;
	while (p[0])
	{
		s1 += ThousandSeparator;
		s1.Append(p, 3);
		p += 3;
	}

	return s1;
}

