// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window
#pragma once

#include <float.h>

class CNumEdit : public CEdit
{
// Construction
public:
	CNumEdit();
	DECLARE_DYNAMIC(CNumEdit)

// Attributes
public:
	virtual BOOL IsValid();
// Operations
public:

	static BOOL SimpleFloatParse(LPCTSTR lpszText, double & d);
	void ExchangeData(CDataExchange* pDX, double & num,
					LPCTSTR szDataName = NULL, LPCTSTR szUnits = NULL,
					double dLowLimit = 0., double dHighLimit = 0.);
	void ExchangeData(CDataExchange* pDX, double & num,
					UINT uIdDataName, UINT uIdUnits,
					double dLowLimit = 0., double dHighLimit = 0.);
	virtual BOOL GetData(CDataExchange * pDX, double & num,
						LPCTSTR szDataName = NULL, LPCTSTR szUnits = NULL,
						double dLowLimit = 0., double dHighLimit = 0.);
	virtual void SetData(double num);

	void SetPrecision(int FractionDigits)
	{
		m_Precision = FractionDigits;
		if (m_Precision > 10)
		{
			m_Precision = 10;
		}
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNumEdit();

protected:
	CString sLastValid;
	int m_Precision;
	//int iSelStart, iSelEnd;
	// Generated message map functions
	//{{AFX_MSG(CNumEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
static double DblMin = DBL_MIN;
#define IS_MIN_DOUBLE(x) ((x) == DblMin)

void DDX_Number(CDataExchange * pDX, UINT nID, double & num,
				LPCTSTR szDataName, LPCTSTR szUnits,
				double dLowLimit, double dHighLimit);

void DDX_Number(CDataExchange* pDX, UINT nID, double & num,
				UINT uIdDataName, UINT uIdUnits,
				double dLowLimit, double dHighLimit);

/////////////////////////////////////////////////////////////////////////////
