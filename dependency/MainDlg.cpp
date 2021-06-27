#include "StdAfx.h"
#include "MainDlg.h"
#include <commctrl.h>
#include <atlstr.h>
#include "utils/PEHelper.h"
#include "utils/common.h"
#include <WinUser.h>

constexpr wchar_t kDefaultWindowText[] = L"dependency";
constexpr wchar_t kSearchMSDNUrl[] = L"https://docs.microsoft.com/zh-cn/search/?terms=";
constexpr wchar_t kMsgboxTitleTip[] = L"提示";
constexpr wchar_t kMsgboxTitleError[] = L"错误";
constexpr int kSpliterHitTestWidth = 5;
constexpr int kSubControlMinWidth = 50;
constexpr int kSubControlMinHeigth = 50;

CMainDlg::CMainDlg()
	: tree_view_(NULL)
	, tree_root_item_(NULL)
	, list_view_export_(NULL)
	, list_view_use_(NULL)
	, left_spliter_xpos_(0)
	, right_spliter_ypos(0)
	, left_splitting(false)
	, right_splitting(false)
	, left_spliter_cursor(NULL)
	, right_spliter_cursor(NULL)
{
}

CMainDlg::~CMainDlg()
{
}

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
	left_spliter_cursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	right_spliter_cursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	tree_view_ = ::GetDlgItem(m_hWnd, IDC_TREE1);
	list_view_export_ = ::GetDlgItem(m_hWnd, IDC_LIST1);
	list_view_use_ = ::GetDlgItem(m_hWnd, IDC_LIST2);
	list_view_ctrl_export_ = std::make_unique<CListViewCtrl>(list_view_export_);
	list_view_ctrl_use_ = std::make_unique<CListViewCtrl>(list_view_use_);
	tree_root_item_ = NULL;
	InitListView();
	::DragAcceptFiles(m_hWnd, TRUE);
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	left_spliter_xpos_ = int((rc.right - rc.left)*0.3);
	right_spliter_ypos = int((rc.bottom - rc.top)*0.24);
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

void CMainDlg::AddListViewItem(HWND list_view, int item_index, int column_index, LPCTSTR text)
{
	LVITEM list_view_item;
	memset(&list_view_item, 0, sizeof(LVITEM));
	list_view_item.mask = LVIF_TEXT;
	list_view_item.cchTextMax = wcslen(text);
	list_view_item.iItem = item_index;
	list_view_item.iSubItem = column_index;
	list_view_item.pszText = (LPWSTR)text;
	::SendMessage(list_view, LVM_SETITEM, 0, (LPARAM)&list_view_item);
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
		SendMessage(list_view_export_, LVM_INSERTITEM, 0, (LPARAM)&item);

		wchar_t buffer[16] = { 0 };
		swprintf_s(buffer, L"%2d(0x%04x)", function.ordinal, function.ordinal);
		AddListViewItem(list_view_export_, count, 0, buffer);
		swprintf_s(buffer, L"%2d(0x%04x)", function.hint, function.hint);
		AddListViewItem(list_view_export_, count, 1, buffer);
		auto function_name = AToU(function.function_name);
		AddListViewItem(list_view_export_, count, 2, function_name.c_str());
		AddListViewItem(list_view_export_, count, 3, ToHexString(function.entry_point).c_str());
		AddListViewItem(list_view_export_, count, 4, ToHexString(function.thunk).c_str());
		count++;
		if (function_name.size() > max_lenth) {
			max_lenth = function_name.size();
		}
	}
	::SendMessage(list_view_export_, LVM_SETCOLUMNWIDTH, 2, MAKELPARAM(max_lenth * 6, 0));
}

void CMainDlg::AddUseFunctions(const std::list<IMAGE_EXPORT_FUNCTION>& function_list)
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
		SendMessage(list_view_use_, LVM_INSERTITEM, 0, (LPARAM)&item);

		wchar_t buffer[16] = { 0 };
		swprintf_s(buffer, L"%2d(0x%04x)", function.ordinal, function.ordinal);
		AddListViewItem(list_view_use_, count, 0, buffer);
		swprintf_s(buffer, L"%2d(0x%04x)", function.hint, function.hint);
		AddListViewItem(list_view_use_, count, 1, buffer);
		if (function.function_name != NULL) {
			auto function_name = AToU(function.function_name);
			AddListViewItem(list_view_use_, count, 2, function_name.c_str());
			if (function_name.size() > max_lenth) {
				max_lenth = function_name.size();
			}
		}
		AddListViewItem(list_view_use_, count, 3, ToHexString(function.entry_point).c_str());
		AddListViewItem(list_view_use_, count, 4, ToHexString(function.thunk).c_str());
		count++;
	}
	int length = max(max_lenth * 6, 60);
	::SendMessage(list_view_use_, LVM_SETCOLUMNWIDTH, 2, MAKELPARAM(length, 0));
}

void CMainDlg::OnMenuFileOpen()
{
	OPENFILENAME ofn = { 0 };
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
	while (true)
	{
		if (child_item == NULL) {
			break;
		}
		TreeView_Expand(tree_view, child_item, code);
		ExpendAllItem(tree_view, child_item, code);
		child_item = TreeView_GetNextSibling(tree_view, child_item);
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

void CMainDlg::OnMenuListItemCopy()
{
	int select_index = (int)::SendMessage(list_view_export_, LVM_GETNEXTITEM, (WPARAM)-1, MAKELPARAM(LVNI_ALL | LVNI_SELECTED, 0));
	CString item_text;
	list_view_ctrl_export_->GetItemText(select_index, 2, item_text.GetBufferSetLength(128), 128);
	if (!item_text.IsEmpty()) {
		CopyToClipbord(item_text.GetBuffer());
	}
}

void CMainDlg::OnMenuListItemFind()
{
	int select_index = (int)::SendMessage(list_view_export_, LVM_GETNEXTITEM, (WPARAM)-1, MAKELPARAM(LVNI_ALL | LVNI_SELECTED, 0));
	CString strText;
	list_view_ctrl_export_->GetItemText(select_index, 2, strText.GetBufferSetLength(128), 128);
	if (!strText.IsEmpty()) {
		CString url;
		url.Format(L"%s%s", kSearchMSDNUrl, strText.GetBuffer());
		::ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOW);
	}
}

void CMainDlg::SetWindowTitle(const std::wstring& path)
{
	CString wnd_text;
	if (path.empty()) {
		wnd_text = kDefaultWindowText;
	}
	else {
		wnd_text.Format(L"%s - %s", kDefaultWindowText, path.c_str());
	}
	::SetWindowText(m_hWnd, wnd_text);
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

		::MoveWindow(tree_view_, 0, 0, left_spliter_xpos_ - 1, height, TRUE);
		::MoveWindow(list_view_use_, left_spliter_xpos_ + 1, 0, width - left_spliter_xpos_ - 1, right_spliter_ypos - 1, TRUE);
		::MoveWindow(list_view_export_, left_spliter_xpos_ + 1, right_spliter_ypos + 1,
			width - left_spliter_xpos_ - 1, height - right_spliter_ypos - 1, TRUE);
	}
	return 0;
}

LRESULT CMainDlg::OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	int notify_code = HIWORD(wParam);
	int ctrl_id = LOWORD(wParam);
	if (notify_code == 0) {
		switch (ctrl_id) {
		case ID_FILE_OPEN: OnMenuFileOpen(); break;
		case ID_FILE_CLOSE: OnMenuFileClose(); break;
		case ID_FILE_EXIT: OnMenuExit(); break;
		case ID_HELP_ABOUT: OnMenuAbout(); break;
		case ID_VIEW_EXPEND: OnMenuExpendAll(); break;
		case ID_VIEW_COLLAPSE: OnMenuCollapseAll(); break;
		case ID_LISTITEM_COPY: OnMenuListItemCopy(); break;
		case ID_LISTITEM_FIND: OnMenuListItemFind(); break;
		default:
			break;
		}
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CMainDlg::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = TRUE;
	HDROP hDrop = (HDROP)wParam;
	UINT count = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (count > 1) {
		MessageBox(L"不支持拖入多个文件", kMsgboxTitleTip, MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	wchar_t file_path[MAX_PATH] = { 0 };
	count = ::DragQueryFile(hDrop, 0, file_path, MAX_PATH);
	if (count == 0) {
		CString msg;
		msg.Format(L"DragQueryFile调用失败，错误码：%u", GetLastError());
		MessageBox(msg, kMsgboxTitleError, MB_OK | MB_ICONERROR);
		return 0;
	}
	OpenFile(file_path);
	return 0;
}

BOOL CMainDlg::OpenFile(const std::wstring& pe_path)
{
	if (!IsPeFile(pe_path)) {
		MessageBox(L"请打开有效的PE文件", kMsgboxTitleError, MB_OK | MB_ICONERROR);
		return 0;
	}
	ClearTreeView();
	current_pe_path_ = pe_path;
	is_x64_archite_ = Is64bitArchiteFileW(pe_path);
	auto file_name = GetFileNameByPath(pe_path);
	current_pe_dir_ = GetFileDirByPath(pe_path);
	PEHelper pe(pe_path);
	std::list<IMAGE_IMPORT_DLL> dll_list;
	pe.GetImportDlls(dll_list);
	tree_root_item_ = AddTreeItem(TVI_ROOT, TVI_FIRST, file_name.c_str());
	TREEITEM_DATA item_data;
	item_data.item_text = std::move(file_name);
	tree_item_map_[tree_root_item_] = std::move(item_data);
	ExpendTreeItem(tree_root_item_);
	SetWindowTitle(pe_path);
	return TRUE;
}

void CMainDlg::ClearTreeView()
{
	tree_root_item_ = NULL;
	is_x64_archite_ = FALSE;
	current_pe_dir_.clear();
	current_pe_path_.clear();
	if (!tree_item_map_.empty()) {
		for (auto& item : tree_item_map_) {
			for (auto& dll : item.second.import_dll_list) {
				ReleaseImageImportDll(&dll);
			}
			for (auto& function : item.second.export_function_list) {
				ReleaseImageExportFunction(&function);
			}
			for (auto& function : item.second.use_function_list) {
				ReleaseImageExportFunction(&function);
			}
		}
		tree_item_map_.clear();
		TreeView_DeleteAllItems(tree_view_);
	}
	SetWindowTitle(L"");
}

void CMainDlg::ExpendTreeItem(HTREEITEM item)
{
	ClearListView();
	auto find_itor = tree_item_map_.find(item);
	ATLASSERT(find_itor != tree_item_map_.end());
	AddUseFunctions(find_itor->second.use_function_list);
	if (find_itor->second.read_flag) {
		AddExportFunctions(find_itor->second.export_function_list);
		return;
	}
	find_itor->second.read_flag = TRUE;
	std::wstring pe_path;
	if (!SearchDllPath(is_x64_archite_, current_pe_dir_, find_itor->second.item_text, pe_path)) {
		// 没有搜索到
		return;
	}
	PEHelper pe(pe_path);
	std::list<IMAGE_IMPORT_DLL> dll_list;
	TREEITEM_DATA item_data;
	if (pe.GetImportDlls(dll_list)) {
		HTREEITEM prev_item = TVI_FIRST;
		for (auto itor = dll_list.begin(); itor != dll_list.end(); ++itor) {
			auto name = AToU(itor->dll_name);
			prev_item = AddTreeItem(item, prev_item, name.c_str());
			item_data.item_text = name;
			//排序
			itor->use_function_list.sort([](const IMAGE_EXPORT_FUNCTION& left, const IMAGE_EXPORT_FUNCTION& right) {
				return left.hint < right.hint;
			});
			item_data.use_function_list = std::move(itor->use_function_list);
			tree_item_map_[prev_item] = std::move(item_data);
		}
		TreeView_Expand(tree_view_, item, TVM_EXPAND);
	}
	std::list<IMAGE_EXPORT_FUNCTION> function_list;
	pe.GetExportFunctions(function_list);
	if (!function_list.empty()) {
		AddExportFunctions(function_list);
	}
	find_itor = tree_item_map_.find(item);
	ATLASSERT(find_itor != tree_item_map_.end());
	find_itor->second.read_flag = TRUE;
	find_itor->second.import_dll_list = std::move(dll_list);
	find_itor->second.export_function_list = std::move(function_list);
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
		ListView_InsertColumn(list_view_export_, i, &lvcol);
		ListView_InsertColumn(list_view_use_, i, &lvcol);
	}
	DWORD dwStyle = ListView_GetExtendedListViewStyle(list_view_export_);
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
		| LVS_EX_HEADERDRAGDROP;
	ListView_SetExtendedListViewStyle(list_view_export_, dwStyle);
	ListView_SetExtendedListViewStyle(list_view_use_, dwStyle);
}

void CMainDlg::ClearListView()
{
	::SendMessage(list_view_export_, LVM_DELETEALLITEMS, 0, 0L);
	::SendMessage(list_view_use_, LVM_DELETEALLITEMS, 0, 0L);
}

LRESULT CMainDlg::OnTvnSelchangedTree1(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	ExpendTreeItem(pNMTreeView->itemNew.hItem);
	return 0;
}

LRESULT CMainDlg::OnNMRclickList1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	int item_count = (int)::SendMessage(list_view_export_, LVM_GETITEMCOUNT, 0, 0L);
	if (item_count == 0) {
		return 0;
	}
	HMENU hMenu = ::LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU2));
	HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
	POINT pt;
	::GetCursorPos(&pt);
	::TrackPopupMenu(hSubMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
	::DestroyMenu(hMenu);

	return 0;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F5) {
		if (!current_pe_path_.empty()) {
			auto pe_path = current_pe_path_;
			OpenFile(pe_path);
		}
	}
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CMainDlg::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	if (x >= left_spliter_xpos_ - kSpliterHitTestWidth && x <= left_spliter_xpos_ + kSpliterHitTestWidth) {
		left_splitting = true;
	}
	else if (y >= right_spliter_ypos - kSpliterHitTestWidth && y <= right_spliter_ypos + kSpliterHitTestWidth) {
		right_splitting = true;
	}
	return 0;
}

LRESULT CMainDlg::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (left_splitting) {
		left_splitting = false;
		::ReleaseCapture();
	}
	else if (right_splitting) {
		right_splitting = false;
		::ReleaseCapture();
	}
	return 0;
}

LRESULT CMainDlg::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	if (x >= left_spliter_xpos_ - kSpliterHitTestWidth && x <= left_spliter_xpos_ + kSpliterHitTestWidth) {
		::SetCursor(left_spliter_cursor);
		::SetCapture(m_hWnd);
	}
	else if (y >= right_spliter_ypos - kSpliterHitTestWidth && y <= right_spliter_ypos + kSpliterHitTestWidth) {
		::SetCursor(right_spliter_cursor);
		::SetCapture(m_hWnd);
	}
	else {
		::ReleaseCapture();
	}
	if (wParam == MK_LBUTTON && (left_splitting || right_splitting)) {
		RECT rc;
		::GetClientRect(m_hWnd, &rc);
		if (left_splitting) {
			if (x<kSubControlMinWidth || x>rc.right - kSubControlMinWidth) {
				return 0;
			}
			left_spliter_xpos_ = x;
		}
		else if (right_splitting) {
			if (y<kSubControlMinHeigth || y>rc.right - kSubControlMinHeigth) {
				return 0;
			}
			right_spliter_ypos = y;
		}
		//根据新的分界位置发送WM_SIZE消息,重新调整左右控件的位置
		SendMessage(m_hWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
	}
	return 0;
}

//https://docs.microsoft.com/en-us/search/?terms=

/*
C++函数符号：
https://blog.csdn.net/hejinjing_tom_com/article/details/6288816?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~aggregatepage~first_rank_v2~rank_aggregation-1-6288816.pc_agg_rank_aggregation&utm_term=c%2B%2B%E5%87%BD%E6%95%B0%E5%90%8D%E5%91%BD%E5%90%8D%E8%A7%84%E5%88%99&spm=1000.2123.3001.4430

Windows WDDM thunk API:
https://zhuanlan.zhihu.com/p/169311344

*/