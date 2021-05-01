#include "StdAfx.h"
#include "MainDlg.h"
#include <commctrl.h>
#include <atlstr.h>
#include "utils/PEHelper.h"
#include "utils/common.h"

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	is_x64_archite_ = FALSE;
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	tree_view_ = ::GetDlgItem(m_hWnd, IDC_TREE1);
	list_view_ = ::GetDlgItem(m_hWnd, IDC_LIST1);

	InitListView();
	// OpenFile(L"C:\\Program Files (x86)\\Tencent\\WeChat\\WeChatApp.exe");
	return TRUE;
}

HTREEITEM CMainDlg::AddTreeItem(HTREEITEM hParent, HTREEITEM hPrev, LPCTSTR pszCaption) {
	TVITEM tree_view_item = { 0 };
	TVINSERTSTRUCT tree_view_struct = { 0 };
	tree_view_item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tree_view_item.pszText = (LPTSTR)pszCaption;
	tree_view_item.cchTextMax = wcslen(pszCaption);

	tree_view_struct.item = tree_view_item;
	tree_view_struct.hInsertAfter = hPrev;
	tree_view_struct.hParent = hParent;
	return TreeView_InsertItem(tree_view_, &tree_view_struct);
}

void CMainDlg::AddListViewItem(int item_index, int column_index, LPCTSTR text)
{
	LVITEM list_view_item;
	memset(&list_view_item, 0, sizeof(LVITEM));
	list_view_item.mask = LVIF_TEXT;
	list_view_item.cchTextMax = wcslen(text);
	list_view_item.iItem = item_index;
	list_view_item.iSubItem = column_index;
	list_view_item.pszText = (LPWSTR)text;
	::SendMessage(list_view_, LVM_SETITEM, 0, (LPARAM)&list_view_item);
}

void CMainDlg::AddExportFunctions(const std::list<IMAGE_EXPORT_FUNCTION>& function_list)
{
	int count = 0;
	size_t max_lenth = 0;
	for (auto& function : function_list) {
		LVITEM item = {};
		item.mask = LVIF_TEXT;
		item.iItem = count;
		item.iSubItem = 0;
		item.pszText = (LPTSTR)std::to_wstring(function.hint).c_str();
		item.state = 0;
		item.stateMask = 0;
		item.iImage = 0;
		item.lParam = 0;
		SendMessage(list_view_, LVM_INSERTITEM, 0, (LPARAM)&item);

		wchar_t buffer[16] = { 0 };
		swprintf_s(buffer, L"%2d(0x%04x)", function.ordinal, function.ordinal);
		AddListViewItem(count, 0, buffer);
		swprintf_s(buffer, L"%2d(0x%04x)", function.hint, function.hint);
		AddListViewItem(count, 1, buffer);
		auto function_name = AToU(function.function_name);
		AddListViewItem(count, 2, function_name.c_str());
		AddListViewItem(count, 3, ToHexString(function.entry_point).c_str());
		AddListViewItem(count, 4, ToHexString(function.thunk).c_str());
		count++;
		if (function_name.size() > max_lenth) {
			max_lenth = function_name.size();
		}
	}
	::SendMessage(list_view_, LVM_SETCOLUMNWIDTH, 2, MAKELPARAM(max_lenth * 6, 0));
}

void CMainDlg::OnMenuFileOpen()
{
	OPENFILENAME ofn = {0};
	TCHAR szFile[MAX_PATH] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = _T("可执行文件(*.exe)\0*.exe\0dll文件(*.dll)\0*.dll\0所有文件(*.*)\0*.*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; /*OFN_EXPLORER | OFN_ALLOWMULTISELECT;*/ 
	if (GetOpenFileName(&ofn)) {
		OpenFile(std::wstring(ofn.lpstrFile));
	}
}

void CMainDlg::OnMenuFileClose()
{
	ClearTreeView();
	ClearListView();
}

void CMainDlg::OnMenuExit()
{
	CloseDialog(IDCLOSE);
}

void CMainDlg::OnMenuAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void ExpendAllItem(HWND tree_view, HTREEITEM item, UINT code) {
	if (item == NULL) {
		return;
	}
	TreeView_Expand(tree_view, item, code);
	auto child_item = TreeView_GetChild(tree_view, item);
	if (child_item != NULL) {
		ExpendAllItem(tree_view, child_item, code);
	}

	while (true) {
		item = TreeView_GetNextSibling(tree_view, item);
		if (item == NULL) {
			break;
		}
		ExpendAllItem(tree_view, item, code);
	}
}

void CMainDlg::OnMenuExpendAll()
{
	ExpendAllItem(tree_view_, tree_root_item_, TVE_EXPAND);
}

void CMainDlg::OnMenuCollapseAll()
{
	ExpendAllItem(tree_view_, tree_root_item_, TVE_COLLAPSE);
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
	ClearTreeView();
	return 0;
}

LRESULT CMainDlg::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (wParam != SIZE_MINIMIZED) {
		auto width = GET_X_LPARAM(lParam);
		auto height = GET_Y_LPARAM(lParam);
		auto tree_width = int(width * 0.3);
		::MoveWindow(tree_view_, 0, 0, tree_width, height, TRUE);
		::MoveWindow(list_view_, tree_width, 0, width - tree_width, height, TRUE);
	}
	return 0;
}

LRESULT CMainDlg::OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (HIWORD(wParam) == 0) {
		switch (LOWORD(wParam))
		{
		case ID_FILE_OPEN: OnMenuFileOpen(); break;
		case ID_FILE_CLOSE: OnMenuFileClose(); break;
		case ID_FILE_EXIT: OnMenuExit(); break;
		case ID_HELP_ABOUT: OnMenuAbout(); break;
		case ID_VIEW_EXPEND: OnMenuExpendAll(); break;
		case ID_VIEW_COLLAPSE: OnMenuCollapseAll(); break;
		default:
			break;
		}
	}
	bHandled = FALSE;
	return 0;
}

BOOL CMainDlg::OpenFile(const std::wstring& pe_path)
{
	ClearTreeView();
	is_x64_archite_ = Is64bitArchiteFileW(pe_path);
	auto file_name = GetFileNameByPath(pe_path);
	current_pe_dir_ = GetFileDirByPath(pe_path);
	PEHelper pe(pe_path);
	std::list<IMAGE_IMPORT_DLL> dll_list;
	pe.GetImportDlls(dll_list);
	tree_root_item_ = AddTreeItem(TVI_ROOT, TVI_FIRST, file_name.c_str());
	TREEITEM_DATA item_data = { 0 };
	item_data.item_text = std::move(file_name);
	tree_item_map_[tree_root_item_] = std::move(item_data);
	ExpendTreeItem(tree_root_item_);
	return TRUE;
}

void CMainDlg::ClearTreeView()
{
	tree_root_item_ = NULL;
	is_x64_archite_ = FALSE;
	current_pe_dir_.clear();
	if (!tree_item_map_.empty()) {
		for (auto& item: tree_item_map_) {
			for (auto& dll : item.second.import_dll_list) {
				ReleaseImageImportDll(&dll);
			}
			for (auto& function : item.second.export_function_list) {
				ReleaseImageExportFunction(&function);
			}
		}
		tree_item_map_.clear();
		TreeView_DeleteAllItems(tree_view_);
	}
}

void CMainDlg::ExpendTreeItem(HTREEITEM item)
{
	ClearListView();
	auto itor = tree_item_map_.find(item);
	ATLASSERT(itor != tree_item_map_.end());
	if (itor->second.read_flag) {
		AddExportFunctions(itor->second.export_function_list);
		return;
	}
	itor->second.read_flag = TRUE;
	std::wstring pe_path;
	if (!SearchDllPath(is_x64_archite_, current_pe_dir_, itor->second.item_text, pe_path)) {
		// 没有搜索到
		return;
	}
	PEHelper pe(pe_path);
	std::list<IMAGE_IMPORT_DLL> dll_list;
	TREEITEM_DATA item_data = { 0 };
	if (pe.GetImportDlls(dll_list)) {
		HTREEITEM prev_item = TVI_FIRST;
		for (auto itor = dll_list.begin(); itor != dll_list.end(); ++itor) {
			auto name = AToU(itor->dll_name);
			prev_item = AddTreeItem(item, prev_item, name.c_str());
			item_data.item_text = name;
			tree_item_map_[prev_item] = std::move(item_data);
		}
		TreeView_Expand(tree_view_, item, TVM_EXPAND);
	}
	std::list<IMAGE_EXPORT_FUNCTION> function_list;
	pe.GetExportFunctions(function_list);
	if (!function_list.empty()) {
		AddExportFunctions(function_list);
	}
	item_data.read_flag = TRUE;
	item_data.import_dll_list = std::move(dll_list);
	item_data.export_function_list = std::move(function_list);
	tree_item_map_[item] = std::move(item_data);
}

void CMainDlg::InitListView()
{
	static const int column_count = 5;
	static const std::wstring column_texts[column_count] = { L"Ordinal", L"Hint", L"Function", L"Entry Point", L"thunk" };
	static const int column_widths[column_count] = { 80, 80, 100, 80, 80 };
	for (int i = 0; i < column_count; ++i) {
		LVCOLUMN lvcol;
		memset(&lvcol, 0, sizeof(LVCOLUMN));
		lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvcol.pszText = (LPWSTR)column_texts[i].c_str();
		lvcol.cchTextMax = column_texts[i].size();
		lvcol.cx = column_widths[i];
		ListView_InsertColumn(list_view_, i, &lvcol);
	}
	DWORD dwStyle = ListView_GetExtendedListViewStyle(list_view_);
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
		| LVS_EX_HEADERDRAGDROP;
	ListView_SetExtendedListViewStyle(list_view_, dwStyle);
}

void CMainDlg::ClearListView()
{
	::SendMessage(list_view_, LVM_DELETEALLITEMS, 0, 0L);
}

LRESULT CMainDlg::OnTvnSelchangedTree1(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	ExpendTreeItem(pNMTreeView->itemNew.hItem);
	return 0;
}


LRESULT CMainDlg::OnNMRclickTree(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	return 0;
}
