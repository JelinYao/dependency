// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atlframe.h>
#include "resource.h"
#include "AboutDlg.h"
#include <map>
#include <list>
#include "utils/PEHelper.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
	public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		NOTIFY_HANDLER(IDC_TREE1, TVN_SELCHANGED, OnTvnSelchangedTree1)
		NOTIFY_HANDLER(IDC_TREE1, NM_RCLICK, OnNMRclickTree)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
private:
	class TREEITEM_DATA {
	public:
		BOOL read_flag;
		std::wstring item_text;
		std::list<IMAGE_EXPORT_FUNCTION> export_function_list;
		std::list<IMAGE_IMPORT_DLL> import_dll_list;
	};

	HWND tree_view_;
	HTREEITEM tree_root_item_;
	bool is_x64_archite_;
	std::wstring current_pe_dir_;
	std::map<HTREEITEM, TREEITEM_DATA> tree_item_map_;

	HWND list_view_;

protected:

	BOOL OpenFile(const std::wstring& pe_path);
	void ClearTreeView();
	void ExpendItem(HTREEITEM item);

	void ClearListView();
	
	HTREEITEM AddTreeItem(HTREEITEM hParent, HTREEITEM hPrev, LPCTSTR pszCaption);
	void AddListViewItem(int item_index, int column_index, LPCTSTR text);

	void AddExportFunctions(const std::list<IMAGE_EXPORT_FUNCTION>& function_list);

	void OnFileOpen();
	void OnFileClose();
	void OnExit();

private:

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}
	
public:
	LRESULT OnTvnSelchangedTree1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnNMRclickTree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};