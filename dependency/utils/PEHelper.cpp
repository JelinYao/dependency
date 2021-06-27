#include "stdafx.h"
#include "PEHelper.h"
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")


PEHelper::PEHelper() : handle_(INVALID_HANDLE_VALUE)
, section_heaer_(NULL)
, base_address_(NULL)
, map_handle_(NULL) {
}

PEHelper::PEHelper(const wchar_t* file) : handle_(INVALID_HANDLE_VALUE)
, section_heaer_(NULL)
, base_address_(NULL)
, map_handle_(NULL) {
	Open(file);
}

PEHelper::PEHelper(const std::wstring &file) : handle_(INVALID_HANDLE_VALUE)
, section_heaer_(NULL)
, base_address_(NULL)
, map_handle_(NULL) {
	Open(file.c_str());
}

PEHelper::~PEHelper() {
	Close();
}

bool PEHelper::Open(const wchar_t* file) {
	HANDLE file_handle = ::CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE) {
		OutputDebugString(L"Create file error");
		return false;
	}
	handle_ = file_handle;
	map_handle_ = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (map_handle_ == NULL) {
		return false;
	}
	base_address_ = MapViewOfFile(map_handle_, FILE_MAP_READ, 0, 0, 0);
	if (base_address_ == NULL) {
		return false;
	}
	dos_header_ = (PIMAGE_DOS_HEADER)base_address_;
	if (dos_header_->e_magic != IMAGE_DOS_SIGNATURE) {
		OutputDebugString(L"Illegal PE header ");
		return false;
	}
	nt_header_ = (PIMAGE_NT_HEADERS)((LPBYTE)base_address_ + dos_header_->e_lfanew);
	if (nt_header_->Signature != IMAGE_NT_SIGNATURE) {
		OutputDebugString(L"Illegal PE header ");
		return false;
	}
	return true;
}

//https://blog.csdn.net/zang141588761/article/details/50401203
void PEHelper::Close() {
	if (handle_ != INVALID_HANDLE_VALUE) {
		CloseHandle(handle_);
		handle_ = INVALID_HANDLE_VALUE;
	}
	if (map_handle_) {
		if (base_address_) {
			UnmapViewOfFile(base_address_);
			base_address_ = NULL;
		}
		CloseHandle(map_handle_);
		map_handle_ = NULL;
	}
}
/*
0 IMAGE_DIRECTORY_ENTRY_EXPORT 导出表

1 IMAGE_DIRECTORY_ENTRY_IMPORT 导入表

2 IMAGE_DIRECTORY_ENTRY_RESOURCE 资源

3 IMAGE_DIRECTORY_ENTRY_EXCEPTION 异常（具体资料不详）

4 IMAGE_DIRECTORY_ENTRY_SECURITY 安全（具体资料不详）

5 IMAGE_DIRECTORY_ENTRY_BASERELOC 重定位表

6 IMAGE_DIRECTORY_ENTRY_DEBUG 调试信息

7 IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 版权信息

8 IMAGE_DIRECTORY_ENTRY_GLOBALPTR 具体资料不详

9 IMAGE_DIRECTORY_ENTRY_TLS Thread Local Storage

10 IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 具体资料不详

11 IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 具体资料不详

12 IMAGE_DIRECTORY_ENTRY_IAT 导入函数地址表

13 IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 具体资料不详

14 IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 具体资料不详

15未使用
*/

//https://www.cnblogs.com/hoodlum1980/archive/2010/09/11/1824133.html
bool PEHelper::GetExportFunctions(std::list<IMAGE_EXPORT_FUNCTION>& function_list) {
	DWORD rva_export_table = nt_header_->OptionalHeader.DataDirectory[0].VirtualAddress;
	if (rva_export_table == NULL) {
		return false;
	}
	PIMAGE_EXPORT_DIRECTORY image_export_dir = (PIMAGE_EXPORT_DIRECTORY)ImageRvaToVa(nt_header_, base_address_, rva_export_table, NULL);

	//dll名称（char*）
	LPCSTR szDllName = (LPCSTR)ImageRvaToVa(
		nt_header_, base_address_,
		image_export_dir->Name,
		NULL);

	//现在加载每个节点
	DWORD ImageBase = nt_header_->OptionalHeader.ImageBase;

	//以下全部是RVA。换算到数组入口处的VA：
	PDWORD pFunctions = (PDWORD)ImageRvaToVa(
		nt_header_, base_address_,
		image_export_dir->AddressOfFunctions,
		NULL);

	PWORD pOrdinals = (PWORD)ImageRvaToVa(
		nt_header_, base_address_,
		image_export_dir->AddressOfNameOrdinals,
		NULL);

	if (image_export_dir->AddressOfNames == 0) {
		// 没有导出函数
		return true;
	}

	PDWORD pNames = (PDWORD)ImageRvaToVa(nt_header_, base_address_, image_export_dir->AddressOfNames, NULL);

	for (DWORD i = 0; i < image_export_dir->NumberOfFunctions; i++)
	{
		//函数名称
		LPCSTR function_name = (LPCSTR)ImageRvaToVa(nt_header_, base_address_, pNames[i], NULL);
		if (function_name == NULL) {
			continue;
		}
		auto temp = pOrdinals[i];
		IMAGE_EXPORT_FUNCTION function;
		function.entry_point = pFunctions[i];
		CopyStringByMalloc(&function.function_name, function_name);
		function.hint = (WORD)i;
		function.ordinal = (WORD)(image_export_dir->Base + i);
		function_list.push_back(function);
	}
	return true;
}

//https://www.cnblogs.com/hoodlum1980/archive/2010/09/08/1821778.html
bool PEHelper::GetImportDlls(std::list<IMAGE_IMPORT_DLL>& dll_list)
{
	DWORD rva_import_table = nt_header_->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (rva_import_table == NULL) {
		return false;
	}
	PIMAGE_IMPORT_DESCRIPTOR import_table = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
		nt_header_,
		base_address_,
		rva_import_table,
		NULL
	);
	//减去内存映射的首地址，就是文件地址了
	int i = 0;
	while (true) {
		IMAGE_IMPORT_DESCRIPTOR import_desc = { 0 };
		memcpy(&import_desc, import_table + i, sizeof(IMAGE_IMPORT_DESCRIPTOR));
		if (import_desc.TimeDateStamp == NULL && import_desc.Name == NULL) {
			break;
		}
		LPCSTR dll_name = (LPCSTR)ImageRvaToVa(
			nt_header_, base_address_,
			import_table[i].Name, //DLL名称的RVA
			NULL);

		PIMAGE_THUNK_DATA32 pThunk = (PIMAGE_THUNK_DATA32)ImageRvaToVa(
			nt_header_, base_address_,
			import_table[i].OriginalFirstThunk, //【注意】这里使用的是OriginalFirstThunk
			NULL);
		int j = 0;
		IMAGE_THUNK_DATA32 null_thunk = { 0 };
		// https://blog.csdn.net/zang141588761/article/details/50401203
		std::list<IMAGE_EXPORT_FUNCTION> function_list;
		while (true) {
			memcpy(&null_thunk, pThunk + j, sizeof(IMAGE_THUNK_DATA32));
			if (null_thunk.u1.AddressOfData == 0 || null_thunk.u1.Function == 0) {
				break;
			}
			IMAGE_EXPORT_FUNCTION use_function;
			if (pThunk[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG32) {
				printf(" \t [%d] \t %ld \t 按序号导入\n ", j, pThunk[j].u1.AddressOfData & 0xffff);
				use_function.ordinal = pThunk[j].u1.AddressOfData & 0xffff;
			}
			else {
				PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(
					nt_header_, base_address_,
					pThunk[j].u1.AddressOfData,
					NULL);
				if (pFuncName == NULL || pFuncName->Name == NULL) {
					continue;
				}
				printf(" \t [%d] \t %ld \t %s\n ", j, pFuncName->Hint, pFuncName->Name);
				use_function.hint = pFuncName->Hint;
				CopyStringByMalloc(&use_function.function_name, pFuncName->Name);
			}
			j++;
			function_list.push_back(use_function);
		}
		IMAGE_IMPORT_DLL dll_info;
		CopyStringByMalloc(&dll_info.dll_name, dll_name);
		dll_info.original_first_thunk = import_table[i].OriginalFirstThunk;
		dll_info.forwarder_chain = import_table[i].ForwarderChain;
		dll_info.first_thunk = import_table[i].FirstThunk;
		dll_info.use_function_list = std::move(function_list);
		dll_list.push_back(dll_info);
		i++;
	}
	// IMAGE_IMPORT_BY_NAME
	return true;
}