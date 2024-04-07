// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atlframe.h>
#include <atlctrls.h>
#include "resource.h"
#include "AboutDlg.h"
#include "utils/PEHelper.h"
#include "ResourceDelegate.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
    public CMessageFilter, public CIdleHandler, public ResourceDelegate
{
public:
    CMainDlg();
    ~CMainDlg();

    enum { IDD = IDD_MAINDLG };

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
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
        COMMAND_ID_HANDLER(ID_LISTCTRL_SELECTALL, OnSelectAll)
        NOTIFY_HANDLER(IDC_TREE1, TVN_SELCHANGED, OnTvnSelchangedTree1)
        NOTIFY_HANDLER(IDC_LIST1, NM_RCLICK, OnNMRclickList1)
    END_MSG_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
    class TREEITEM_DATA {
    public:
        BOOL read_flag = FALSE;
        std::wstring item_text;
        std::list<IMAGE_EXPORT_FUNCTION> export_function_list;
        std::list<IMAGE_EXPORT_FUNCTION> use_function_list;
        std::list<IMAGE_IMPORT_DLL> import_dll_list;
    };

protected:
    BOOL OpenFile(const std::wstring& pe_path);
    void ClearTreeView();
    void ExpendTreeItem(HTREEITEM item);

    void InitListView();
    void ClearListView();

    HTREEITEM AddTreeItem(HTREEITEM hParent, HTREEITEM hPrev, LPCTSTR pszCaption);
    void AddListViewItem(HWND list_view, int item_index, int column_index, LPCTSTR text);

    void AddExportFunctions(const std::list<IMAGE_EXPORT_FUNCTION>& function_list);
    void AddUseFunctions(const std::list<IMAGE_EXPORT_FUNCTION>& function_list);

    void OnMenuFileOpen();
    void OnMenuFileClose();
    void OnMenuExit();
    void OnMenuAbout();
    void OnAddRightMenuContext();
    void OnDelRightMenuContext();
    void OnMenuExpendAll();
    void OnMenuCollapseAll();

    void OnMenuListItemCopy();
    void OnMenuListItemFind();
    void SetWindowTitle(const std::wstring& path);
    LRESULT OnClickLanguageMenu(int id);
    // callback
    void OnSwitchLanguage() override;
    void OnAddLanguage(const wchar_t* text) override;

private:
    BOOL PreTranslateMessage(MSG* pMsg) override;

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        OnMenuAbout();
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

    LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        OnMenuListItemCopy();
        return 0;
    }

    LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        list_view_ctrl_use_->SelectAllItems();
        list_view_ctrl_export_->SelectAllItems();
        return 0;
    }

    void CloseDialog(int nVal)
    {
        DestroyWindow();
        ::PostQuitMessage(nVal);
    }

public:
    LRESULT OnTvnSelchangedTree1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);

    LRESULT OnNMRclickList1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);

private:
    HWND tree_view_;
    HWND hwnd_dep_path_;
    HTREEITEM tree_root_item_;
    HACCEL hAccel;
    bool is_x64_archite_;
    std::wstring current_pe_dir_;
    std::wstring current_pe_path_;
    std::map<HTREEITEM, TREEITEM_DATA> tree_item_map_;

    HWND list_view_export_;
    HWND list_view_use_;
    std::unique_ptr<CListViewCtrl> list_view_ctrl_export_;
    std::unique_ptr<CListViewCtrl> list_view_ctrl_use_;
    bool left_splitting;
    int left_spliter_xpos_;
    bool right_splitting;
    int right_spliter_ypos;
    HCURSOR left_spliter_cursor;
    HCURSOR right_spliter_cursor;
};

