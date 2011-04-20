// SplitterWndEx.cpp
#include "stdafx.h"
#include "SplitterWndEx.h"

IMPLEMENT_DYNAMIC(CSplitterWndEx, CSplitterWnd)

IMPLEMENT_DYNCREATE(CEmptyPane, CView)

BEGIN_MESSAGE_MAP(CSplitterWndEx, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitterWndEx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSplitterWndEx::CSplitterWndEx()
{
}

// DeleteRow and DeleteColumn are provided only to
// avoid ASSERT(GetStyle() & SPLS_DYNAMIC_SPLIT);
void CSplitterWndEx::DeleteRow(int rowDelete)
{
#ifdef _DEBUG
	DWORD dwOldStyle = GetStyle();
	ModifyStyle(0, SPLS_DYNAMIC_SPLIT, 0);
#endif
	CSplitterWnd::DeleteRow(rowDelete);
#ifdef _DEBUG
	if ((dwOldStyle & SPLS_DYNAMIC_SPLIT) == 0)
	{
		ModifyStyle(SPLS_DYNAMIC_SPLIT, 0, 0);
	}
#endif
}

void CSplitterWndEx::DeleteColumn(int colDelete)
{
#ifdef _DEBUG
	DWORD dwOldStyle = GetStyle();
	ModifyStyle(0, SPLS_DYNAMIC_SPLIT, 0);
#endif
	CSplitterWnd::DeleteColumn(colDelete);
#ifdef _DEBUG
	if ((dwOldStyle & SPLS_DYNAMIC_SPLIT) == 0)
	{
		ModifyStyle(SPLS_DYNAMIC_SPLIT, 0, 0);
	}
#endif
}

void CSplitterWndEx::InsertRow(int rowInsert)
{
	ASSERT_VALID(this);
	ASSERT(m_nRows < m_nMaxRows);

	// Change views' ID's to make one empty row
	// and move row info (m_pRowInfo)
	m_nRows++; // set new count

	for (int row = m_nRows - 2; row >= rowInsert; row--)
	{
		m_pRowInfo[row + 1] = m_pRowInfo[row];
		for (int col = 0; col < m_nCols; col++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			// increment pane row number
			pPane->SetDlgCtrlID(IdFromRowCol(row + 1, col));
		}
		if (m_bHasVScroll)
		{
			CWnd* pScroll = GetDlgItem(AFX_IDW_VSCROLL_FIRST+row);
			if (pScroll != NULL)
				pScroll->SetDlgCtrlID(AFX_IDW_VSCROLL_FIRST+row+1);
		}
	}

	// create the scroll bar
	if (m_bHasVScroll &&
		!CreateScrollBarCtrl(SBS_VERT, AFX_IDW_VSCROLL_FIRST + rowInsert))
	{
		TRACE0("Warning: InsertRow failed to create scroll bar.\n");
		return;
	}

	return;
	// the application should create new views and call
	// RecalcLayout();
}

void CSplitterWndEx::InsertColumn(int colInsert)
{
	ASSERT_VALID(this);
	ASSERT(m_nCols < m_nMaxCols);

	// Change views' ID's to make one empty column
	// and move column info (m_pColInfo)
	m_nCols++; // set new count

	for (int col = m_nCols - 2; col >= colInsert; col--)
	{
		m_pColInfo[col + 1] = m_pColInfo[col];
		for (int row = 0; row < m_nRows; row++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			// increment pane column number
			pPane->SetDlgCtrlID(IdFromRowCol(row, col + 1));
		}
		if (m_bHasHScroll)
		{
			CWnd* pScroll = GetDlgItem(AFX_IDW_HSCROLL_FIRST+col);
			if (pScroll != NULL)
				pScroll->SetDlgCtrlID(AFX_IDW_HSCROLL_FIRST+col+1);
		}
	}

	// create the scroll bar
	if (m_bHasHScroll &&
		!CreateScrollBarCtrl(SBS_HORZ, AFX_IDW_HSCROLL_FIRST + colInsert))
	{
		TRACE0("Warning: InsertColumn failed to create scroll bar.\n");
		return;
	}

	return;
	// the application should create new views and call
	// RecalcLayout();
}

BOOL CSplitterWndEx::CreateStaticEx(CWnd* pParentWnd,
									int nMaxRows, int nMaxCols,
									int nRows, int nCols,
									DWORD dwStyle,
									UINT nID)
{
	ASSERT(pParentWnd != NULL);
	ASSERT(nMaxRows >= 1 && nMaxRows <= 16);
	ASSERT(nMaxCols >= 1 && nMaxCols <= 16);
	ASSERT(nMaxCols > 1 || nMaxRows > 1);     // 1x1 is not permitted
	ASSERT(nCols <= nMaxCols);
	ASSERT(nRows <= nMaxRows);
	ASSERT(dwStyle & WS_CHILD);
	ASSERT(!(dwStyle & SPLS_DYNAMIC_SPLIT)); // can't have dynamic split

	ASSERT(m_nRows == 0 && m_nCols == 0);       // none yet
	m_nRows = nRows;
	m_nCols = nCols;
	m_nMaxRows = nMaxRows;
	m_nMaxCols = nMaxCols;

	// create with zero minimum pane size
	if (!CreateCommon(pParentWnd, CSize(0, 0), dwStyle, nID))
		return FALSE;

	// all panes must be created with explicit calls to CreateView
	return TRUE;
}

void CSplitterWndEx::FillEmptyPanes()
{
	for (int row = 0; row < GetRowCount(); row++)
	{
		for (int col = 0; col < GetColumnCount(); col++)
		{
			if (NULL == GetDlgItem(IdFromRowCol(row, col)))
			{
				int nHeight, nMaxHeight;
				GetRowInfo(row, nHeight, nMaxHeight);

				int nWidth, nMaxWidth;
				GetColumnInfo(col, nWidth, nMaxWidth);
				CreateView(row, col, RUNTIME_CLASS(CEmptyPane),
							CSize(nWidth, nHeight), NULL);
			}
		}
	}

}
CView * CSplitterWndEx::ReplacePane(CView * pView,
									int row, int col)
{
	CWnd * pWnd = GetPane(row, col);
	CView * pOldView = DYNAMIC_DOWNCAST(CView, pWnd);
	if (pView == pOldView)
	{
		return pOldView;
	}
	if (pOldView)
	{
		pOldView->SetDlgCtrlID(-1);
	}
	if (pView)
	{
		pView->SetDlgCtrlID(IdFromRowCol(row, col));
	}
	return pOldView;
}
