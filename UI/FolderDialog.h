#if !defined(AFX_FOLDERDIALOG_H__D77B1122_F648_11D1_B1C9_00A0244CBB12__INCLUDED_)
#define AFX_FOLDERDIALOG_H__D77B1122_F648_11D1_B1C9_00A0244CBB12__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FolderDialog.h : header file
//
//#ifndef _SHLOBJ_H_
#include <shlobj.h>
//#endif
/////////////////////////////////////////////////////////////////////////////
// CFolderDialog dialog

class CFolderDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFolderDialog)

public:
	CFolderDialog(LPCTSTR lpszTitle = NULL,
				LPCTSTR lpszStartingDirectory = NULL,
				DWORD dwFlags = BIF_RETURNONLYFSDIRS,
				CWnd* pParentWnd = NULL);

	BROWSEINFO m_bi;
	TCHAR szBuffer[MAX_PATH + 1];
	CString szStartupDir;
	CString szDisplayName;
	CString szPath;
// Operations
	virtual int DoModal();

	CString GetFolderDisplayName() const;
	CString GetFolderPath() const;

// Overridable callbacks
	virtual int OnInitDone(HWND hwnd);
	virtual int OnFolderChange(HWND hwnd, LPITEMIDLIST lpItem);
	virtual int OnValidateFailed(HWND hwnd, LPCTSTR ErrorName);

protected:
	static int CALLBACK BrowseCallbackProc(HWND hwnd,
											UINT uMsg, LPARAM lParam, LPARAM lpData);


// Implementation
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CFolderDialog)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOLDERDIALOG_H__D77B1122_F648_11D1_B1C9_00A0244CBB12__INCLUDED_)