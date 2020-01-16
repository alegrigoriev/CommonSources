#include "StdAfx.h"
#include "TimeToStr.h"
#include "LocaleUtilities.h"

CString TimeToHhMmSs(ULONGLONG TimeMs, int Flags)
{
	unsigned hh = (unsigned)(TimeMs / 3600000);
	TimeMs -= hh * 3600000;
	unsigned mm = (unsigned)TimeMs / 60000;
	TimeMs -= mm * 60000;
	unsigned ss = (unsigned)TimeMs / 1000;
	TimeMs -= ss * 1000;
	unsigned ms = (unsigned)TimeMs;
	CString s;
	LPCTSTR TimeSeparator = LocaleParameters::TimeSeparator();
	LPCTSTR DecimalPoint = LocaleParameters::DecimalPoint();

	TCHAR StrMs[16];

	if (Flags & TimeToHhMmSs_Frames75)
	{
		_stprintf_s(StrMs, countof(StrMs), _T("%02uf"), ms);
	}
	else
	{
		_stprintf_s(StrMs, countof(StrMs), _T("%03u"), ms);
	}

	if (Flags & (TimeToHhMmSs_NeedsMs | TimeToHhMmSs_Frames75))
	{
		if (hh != 0 || (Flags & TimeToHhMmSs_NeedsHhMm))
		{
			s.Format(_T("%u%s%02u%s%02u%s%s"),
					hh, TimeSeparator,
					mm, TimeSeparator,
					ss, DecimalPoint,
					StrMs);
		}
		else if (mm != 0 || (Flags & TimeToHhMmSs_NeedsMm))
		{
			s.Format(_T("%u%s%02u%s%s"),
					mm, TimeSeparator,
					ss, DecimalPoint,
					StrMs);
		}
		else
		{
			s.Format(_T("%u%s%s"),
					ss, DecimalPoint,
					StrMs);
		}
	}
	else
	{
		if (hh != 0
			|| 0 != (Flags & TimeToHhMmSs_NeedsHhMm))
		{
			s.Format(_T("%u%s%02u%s%02u"),
					hh, TimeSeparator,
					mm, TimeSeparator,
					ss);
		}
		else
		{
			s.Format(_T("%u%s%02u"),
					mm, TimeSeparator,
					ss);
		}
	}
	return s;
}

CString SampleToString(ULONGLONG Sample, unsigned nSamplesPerSec, unsigned Flags)
{
	switch (Flags & SampleToString_Mask)
	{
	case SampleToString_Sample:
		return LtoaCS(Sample);
		break;
	case SampleToString_Seconds:
	{
		CString s;
		ULONGLONG ms = ULONGLONG(Sample * 1000. / nSamplesPerSec);
		unsigned sec = unsigned(ms / 1000);
		ms = ms % 1000;
		TCHAR * pFormat = _T("%s%s0");
		if (Flags & TimeToHhMmSs_NeedsMs)
		{
			pFormat = _T("%s%s%03u");
		}
		s.Format(pFormat, static_cast<LPCTSTR>(LtoaCS(sec)), LocaleParameters::DecimalPoint(), (unsigned)ms);
		return s;
	}
		break;

	default:
	case SampleToString_HhMmSs:
		return TimeToHhMmSs(unsigned(Sample * 1000. / nSamplesPerSec), Flags);
		break;

	case SampleToString_HhMmSsFf:
	{
		unsigned Seconds = unsigned(Sample / nSamplesPerSec);
		unsigned Frames = (Sample % nSamplesPerSec) * 75 / nSamplesPerSec;
		return TimeToHhMmSs(Seconds * 1000 + Frames, Flags | TimeToHhMmSs_Frames75);
	}
		break;
	}
}

