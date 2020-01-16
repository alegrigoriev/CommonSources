#pragma once

struct LocaleParameters
{
private:
	static const int BufSize = 16;
	TCHAR m_TimeSeparator[BufSize] = {':'};
	TCHAR m_DecimalPoint[BufSize] = {'.'};
	TCHAR m_ThousandSeparator[BufSize] = {','};

public:
	LocaleParameters()
	{
	}
	void LoadLocaleParameters()
	{
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, m_DecimalPoint, BufSize);
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, m_TimeSeparator, BufSize);
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, m_ThousandSeparator, BufSize);
	}

	static void Load()
	{
		singleton.LoadLocaleParameters();
	}

	static const TCHAR* TimeSeparator()
	{
		return singleton.m_TimeSeparator;
	}

	static const TCHAR* DecimalPoint()
	{
		return singleton.m_DecimalPoint;
	}

	static const TCHAR* ThousandSeparator()
	{
		return singleton.m_ThousandSeparator;
	}
private:
	static LocaleParameters singleton;
};

CString LtoaCS(LONGLONG num);
