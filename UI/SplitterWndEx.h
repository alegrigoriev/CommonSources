// SplitterWndEx.h
#ifndef SPLITTER_WND_EX_H__
#define SPLITTER_WND_EX_H__
// The class allows dynamic creation of additional
// views of different type for the static splitter window

class CSplitterWndEx : public CSplitterWnd
{
	DECLARE_DYNAMIC(CSplitterWndEx)
// Construction
public:
	void FillEmptyPanes();
	CSplitterWndEx();

	// Create a multiple view type splitter with static layout
	BOOL CreateStaticEx(CWnd* pParentWnd,
						int nMaxRows, int nMaxCols,
						int nRows = 1, int nCols = 1,
						DWORD dwStyle = WS_CHILD | WS_VISIBLE,
						UINT nID = AFX_IDW_PANE_FIRST);

// DeleteRow and DeleteColumn are provided only to
// avoid ASSERT(GetStyle() & SPLS_DYNAMIC_SPLIT);
	virtual void DeleteRow(int rowDelete);
	virtual void DeleteColumn(int colDelete);

	virtual void InsertRow(int rowInsert);
	virtual void InsertColumn(int colInsert);
	CView * ReplacePane(CView * pView, int row, int col);
	//{{AFX_MSG(CSplitterWndEx)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	// view type to fill empty panes
	class CEmptyPane: public CView
	{
	protected: // create from serialization only
		CEmptyPane() {}
		DECLARE_DYNCREATE(CEmptyPane);
	public:
		virtual void OnDraw(CDC* pDC) {}  // draw nothing
		virtual ~CEmptyPane() {}
	};
};

#endif //#ifndef SPLITTER_WND_EX_H__
