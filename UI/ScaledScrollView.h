// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once
// ScaledScrollView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScaledScrollView view

class CScaledScrollView : public CView
{
protected:
	CScaledScrollView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CScaledScrollView)

// Attributes
public:
	virtual void UnsyncHorizontal();
	virtual BOOL SyncHorizontal(CScaledScrollView * pSView);
	virtual void UnsyncVertical();
	virtual BOOL SyncVertical(CScaledScrollView * pSView);
	BOOL IsSlaveHor() const
	{
		return m_pHorMaster != this;
	}
	BOOL IsSlaveVert() const
	{
		return m_pVertMaster != this;
	}
	double GetStartTrackingX() const
	{
		return m_dXStartTracking;
	}
	double GetEndTrackingX() const
	{
		return m_dXEndTracking;
	}
	double GetStartTrackingY() const
	{
		return m_dYStartTracking;
	}
	double GetEndTrackingY() const
	{
		return m_dYEndTracking;
	}

protected:
	virtual UINT GetFormatStringID() const;
	virtual CString GetString(double x, double y);
	virtual UINT GetPopupMenuID(CPoint point);
	virtual CMenu * ModifyPopupMenu(CMenu * pMenu);
	virtual CMenu * GetPopupMenu(CMenu * pMenu, CPoint point);
// Operations
	virtual void DoZoom(double dHorScale, double dVertScale, CPoint ptCenter);
public:
	virtual void Zoom(double dHorScale, double dVertScale,
					CPoint ptCenter = CPoint(INT_MAX, INT_MAX));
	// ptCenter - Client coordinates
	void ZoomIn(double dScale = 2.,
				CPoint ptCenter = CPoint(INT_MAX, INT_MAX))
	{
		Zoom(dScale, dScale, ptCenter);
	}
	void ZoomOut(double dScale = 2.,
				CPoint ptCenter = CPoint(INT_MAX, INT_MAX))
	{
		Zoom(1. / dScale, 1. / dScale, ptCenter);
	}
	void ZoomInSelection(DWORD flags);
	void ZoomOutSelection(DWORD flags);

	double GetXScale() const
	{
		return dScaleX;
	}
	double GetYScale() const
	{
		return dScaleY;
	}
	double GetXScaleDev() const
	{
		return dScaleX * dLogScaleX;
	}
	double GetYScaleDev() const
	{
		return dScaleY * dLogScaleX;
	}

	POINT DoubleToPointDev(double x, double y) const;
	void PointToDoubleDev(POINT pt, double & x, double & y) const;
	POINT DoubleToPoint(double x, double y) const;
	void PointToDouble(POINT pt, double & x, double & y) const;

	int WorldToWindowX(double x) const;
	int WorldToWindowY(double y) const;

	int WorldToWindowXrnd(double x) const;
	int WorldToWindowYrnd(double y) const;

	int WorldToWindowXceil(double x) const;
	int WorldToWindowYceil(double y) const;

	int WorldToWindowXfloor(double x) const;
	int WorldToWindowYfloor(double y) const;

	double WindowToWorldX(int x) const;
	double WindowToWorldY(int y) const;

	int WorldToLogX(double x) const;
	int WorldToLogY(double y) const;

	double LogToWorldX(int x) const;
	double LogToWorldY(int y) const;

	void GetExtents(double & left, double & right,
					double & bottom, double & top) const;
	virtual void SetExtents(double left, double right,
							double bottom, double top);
	virtual void NotifySlaveViews(DWORD flag);

	virtual BOOL ScrollBy(double dx, double dy, BOOL bDoScroll = TRUE);
	BOOL ScrollTo(double x, double y, BOOL bDoScroll = TRUE)
	{
		return ScrollBy(x - dOrgX, y - dOrgY, bDoScroll);
	}

	virtual BOOL MasterScrollBy(double dx, double dy, BOOL bDoScroll = TRUE)
	{
		return MasterScrollBy(dx, dy, bDoScroll, NULL, NULL);
	}
	// pScrollRect specifies the rectangle to scroll inside
	virtual BOOL MasterScrollBy(double dx, double dy, BOOL bDoScroll, RECT const * pScrollRect, RECT const * pClipRect = NULL);
	BOOL MasterScrollTo(double x, double y, BOOL bDoScroll = TRUE)
	{
		return MasterScrollBy(x - dOrgX, y - dOrgY, bDoScroll);
	}
	void SetMaxExtents(double left, double right,
						double bottom, double top);
	void SetMaxExtentsMaster(double left, double right,
							double bottom, double top);

	void KeepAspectRatio(bool flag)
	{
		bKeepAspectRatio = flag;
	}
	void KeepScaleOnResize(bool flag)
	{
		bKeepScaleOnResizeX = bKeepScaleOnResizeY = flag;
	}
	void KeepScaleOnResizeX(bool flag)
	{
		bKeepScaleOnResizeX = flag;
	}
	void KeepScaleOnResizeY(bool flag)
	{
		bKeepScaleOnResizeY = flag;
	}
	void KeepOrgOnResizeX(bool flag)
	{
		bKeepOrgOnResizeX = flag;
	}
	void KeepOrgOnResizeY(bool flag)
	{
		bKeepOrgOnResizeY = flag;
	}
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll, RECT const * pScrollRect, RECT const * pClipRect);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaledScrollView)
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual void OnDraw(CDC *);
	virtual void OnInitialUpdate();
protected:
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL
	virtual BOOL OnNeedText( UINT_PTR id, NMHDR * pNotifyStruct, LRESULT * result );
	virtual INT_PTR OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

// Implementation
protected:
	double dOrgX, dOrgY; // left top corner
	double dScaleX, dScaleY;    // real to logical translation coeff
	double dLogScaleX, dLogScaleY; // logical to device translation coeff
	double dSizeX, dSizeY;  // fabs of right-left, top-bottom
	double dExtX, dExtY; // signed right-left, top-bottom
	// maximum space extents: (for scrolling)
	double dMinLeft, dMaxRight, dMinBottom, dMaxTop;

	CScaledScrollView * m_pHorMaster;
	CScaledScrollView * m_pVertMaster;

	enum {ScrollMin = -16384, ScrollMax = 16383};

	bool bKeepAspectRatio;
	bool bKeepScaleOnResizeX;
	bool bKeepScaleOnResizeY;
	bool bKeepOrgOnResizeX;
	bool bKeepOrgOnResizeY;

	// variables and functions for selection rectangle support
	double m_dXStartTracking;
	double m_dYStartTracking;
	double m_dXEndTracking;
	double m_dYEndTracking;
	double m_dXSelectionBegin;
	double m_dYSelectionBegin;
	double m_dXSelectionEnd;
	double m_dYSelectionEnd;

	bool bIsTrackingSelection;
	bool bHasSelection;
	bool bSelRectDrawn;
	int nKeyPressed;
	// invert selection rectangle, regardless whether it is visible or not
	virtual void DrawSelectionRect(CDC * , double left,
									double right, double bottom, double top);
	// hide selection rectangle, if it is drawn
	virtual void RemoveSelectionRect();
	virtual void RestoreSelectionRect();
	// set or change selection rectangle
	virtual void ChangeSelection(double left, double right,
								double bottom, double top);

	virtual void CancelSelection();
	virtual void AdjustNewScale(double OldScaleX, double OldScaleY,
								double & NewScaleX, double & NewScaleY);
	virtual void AdjustNewOrigin(double & NewOrgX, double & NewOrgY);

	void UpdateScrollbars(BOOL bRedraw = TRUE);
	virtual void UpdateCaretPosition() {}

	int GetMappingInfo();
	void ArrangeMaxExtents();
	virtual void OnChangeOrgExt(double left, double width,
								double top, double height, DWORD flag);
	virtual void OnMasterChangeOrgExt(double left, double width,
									double top, double height, DWORD flag);
	virtual POINT GetZoomCenter();
	// flag bits:
	enum {CHANGE_WIDTH = 0x1, CHANGE_HOR_ORIGIN = 0x2,
		CHANGE_HEIGHT = 0x4, CHANGE_VERT_ORIGIN = 0x8,
		CHANGE_MAX_HOR_EXTENTS = 0x10,
		CHANGE_MAX_VERT_EXTENTS = 0x20,
		CHANGE_HOR_EXTENTS = CHANGE_WIDTH | CHANGE_HOR_ORIGIN,
		CHANGE_VERT_EXTENTS = CHANGE_HEIGHT | CHANGE_VERT_ORIGIN};

	void RecalcScale();
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

	virtual ~CScaledScrollView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CScaledScrollView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewZoomOut2();
	afx_msg void OnViewZoomin2();
	afx_msg void OnViewZoomOut();
	afx_msg void OnViewZoomin();
	afx_msg void OnUpdateIndicatorMousePosition(CCmdUI* pCmdUI);
	afx_msg void OnViewZoominHor();
	afx_msg void OnViewZoomInVert();
	afx_msg void OnViewZoomOutHor();
	afx_msg void OnViewZoomOutVert();
	afx_msg void OnViewZoominHor2();
	afx_msg void OnViewZoomInVert2();
	afx_msg void OnViewZoomOutHor2();
	afx_msg void OnViewZoomOutVert2();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged( CWnd* pWnd );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
