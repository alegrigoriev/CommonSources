// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the WNASPI32_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// WNASPI32_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef WNASPI32_EXPORTS
#define WNASPI32_API __declspec(dllexport)
#else
#define WNASPI32_API __declspec(dllimport)
#endif
#include "srbcmn.h"
#include "srb32.h"

typedef struct HaTargetLun
{
	UCHAR Lun;
	UCHAR TargetId;
	UCHAR HaId;
	UCHAR zero;
} HaTargetLun;


WNASPI32_API DWORD _cdecl GetASPI32SupportInfo(void);
WNASPI32_API DWORD _cdecl SendASPI32Command( void * );
WNASPI32_API BOOL _cdecl GetASPI32Buffer(PASPI32BUFF );
WNASPI32_API BOOL _cdecl FreeASPI32Buffer(PASPI32BUFF );
WNASPI32_API BOOL _cdecl TranslateASPI32Address( PDWORD, PDWORD );
WNASPI32_API UCHAR _cdecl GetASPI32DriveLetter(HaTargetLun);
WNASPI32_API HaTargetLun _cdecl GetASPI32HaTargetLun(UCHAR DriveLetter);
