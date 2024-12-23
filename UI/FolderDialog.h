// Copyright Alexander Grigoriev, 1997-2002, All Rights Reserved
#pragma once
// FolderDialog.h : header file
//
#include <shlobj.h>
/////////////////////////////////////////////////////////////////////////////
// CFolderDialog dialog

class CFolderDialog : public CCommonDialog
{
	DECLARE_DYNAMIC(CFolderDialog)

public:
	CFolderDialog(LPCTSTR lpszTitle = NULL,
				LPCTSTR lpszStartingDirectory = NULL,
				bool EnableCreateFolder = false,
				DWORD dwFlags = BIF_RETURNFSANCESTORS |
								BIF_RETURNONLYFSDIRS |
								BIF_NEWDIALOGSTYLE,
				CWnd* pParentWnd = NULL,
				class CStringHistory * pHistory = NULL);

	CFolderDialog(UINT TitleId,
				LPCTSTR lpszStartingDirectory = NULL,
				bool EnableCreateFolder = false,
				DWORD dwFlags = BIF_RETURNFSANCESTORS |
								BIF_RETURNONLYFSDIRS |
								BIF_NEWDIALOGSTYLE,
				CWnd* pParentWnd = NULL,
				class CStringHistory * pHistory = NULL);

	BROWSEINFO m_bi;
	TCHAR szBuffer[MAX_PATH + 1];
	bool m_bEnableCreateDir;
	CString szStartupDir;
	CString szDisplayName;
	CString szPath;
	CString m_sTitle;
	CComboBox m_HistoryCombo;
// Operations
	virtual INT_PTR DoModal();

	CString GetFolderDisplayName() const;
	CString GetFolderPath() const;

// Overridable callbacks
	virtual int OnInitDone();
	virtual int OnFolderChange(LPITEMIDLIST lpItem);
	virtual int OnValidateFailed(LPCTSTR ErrorName);
	virtual void OnIUnknown(IUnknown * pUnknown);

	void LoadHistoryCb();

	void EnableOK(BOOL Enable = TRUE);
	void SetExpanded(LPCWSTR Path);
	void SetExpanded(LPITEMIDLIST Path);
	void SetOkText(LPCWSTR Text);
	void SetSelection(LPCTSTR Path);
	void SetSelection(LPITEMIDLIST Path);
	void SetStatusText(LPCTSTR Text);

protected:
	static int CALLBACK BrowseCallbackProc(HWND hwnd,
											UINT uMsg, LPARAM lParam, LPARAM lpData);

	class CStringHistory * m_pStringHistory;
// Implementation
#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CFolderDialog)
	// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnComboSelendOK();
	//}}AFX_MSG
	//virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

