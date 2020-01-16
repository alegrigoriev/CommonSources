// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once
// Ruler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHorizontalRuler view

class CHorizontalRuler : public CView
{
	typedef CView BaseClass;
protected:
	CHorizontalRuler();           // protected constructor used by dynamic creation
	DECLARE_DYNAMIC(CHorizontalRuler)

// Attributes
public:
	static int CalculateHeight();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHorizontalRuler)
protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	// Pixels >0 - picture moves to the right, pixels <0 - picture moves to the left
	virtual void HorizontalScrollByPixels(int Pixels) = 0;
	virtual void BeginMouseTracking();

// Implementation
protected:
	virtual ~CHorizontalRuler();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	int PrevMouseX;
	int ButtonPressed;
	bool m_bIsTrackingSelection;

	// Generated message map functions
protected:
	//{{AFX_MSG(CHorizontalRuler)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CVerticalRuler view

class CVerticalRuler : public CView
{
	typedef CView BaseClass;
protected:
	CVerticalRuler();           // protected constructor used by dynamic creation
	DECLARE_DYNAMIC(CVerticalRuler)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVerticalRuler)
protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	virtual void VerticalScrollByPixels(int Pixels) = 0;
	virtual void BeginMouseTracking();
// Implementation
protected:
	virtual ~CVerticalRuler();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	int PrevMouseY;
	int ButtonPressed;
	bool m_bIsTrackingSelection;

	// Generated message map functions
protected:
	//{{AFX_MSG(CVerticalRuler)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

