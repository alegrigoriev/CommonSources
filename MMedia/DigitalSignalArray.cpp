// DigitalSignalArray.cpp

#include "DigitalSignalArray.h"

CSignalArrayBase::~CSignalArrayBase()
{
	delete pArray;
	pArray = NULL;
	pStore = NULL;
	pRetrieve = NULL;
	nSize = 0;
}

BOOL CSignalArrayBase::Allocate(int Size)
{
	ASSERT (Size > 0);
	if (Size <= nSize && Size > (nSize / 4) * 3)
	{
		// don't reallocate the array
		return TRUE;
	}
	delete pArray;
	pArray = NULL;
	pStore = NULL;
	pRetrieve = NULL;
	pArray = new char[Size * ItemSize()];
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
#if 0
// template compilation test
CSignalArray<__int16,1> a1;
CSignalArray<__int16,2> a2;
CSignalArray<double,1> a3;
CSignalArray<double,2> a4;
CSignalArray<unsigned __int8,1> a5;
CSignalArray<unsigned __int8,1> a6;
CSignalArray<Complex,1> a7;
CSignalArray<Complex,2> a8;
CMappedSignalArray<__int16,1> ma1(NULL);
CMappedSignalArray<__int16,2> ma2(NULL);
CMappedSignalArray<double,1> ma3(NULL);
CMappedSignalArray<double,2> ma4(NULL);
CMappedSignalArray<unsigned __int8,1> ma5(NULL);
CMappedSignalArray<unsigned __int8,1> ma6(NULL);
CMappedSignalArray<Complex,1> ma7(NULL);
CMappedSignalArray<Complex,2> ma8(NULL);
#endif
