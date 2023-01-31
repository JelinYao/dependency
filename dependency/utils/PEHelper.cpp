#include "stdafx.h"
#include "PEHelper.h"
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")


PEHelper::PEHelper()
{
}

PEHelper::PEHelper(const wchar_t* file)
{
    Open(file);
}

PEHelper::PEHelper(const std::wstring& file)
{
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
    nt_header32_ = (PIMAGE_NT_HEADERS32)((LPBYTE)base_address_ + dos_header_->e_lfanew);
    if (nt_header32_->Signature != IMAGE_NT_SIGNATURE) {
        OutputDebugString(L"Illegal PE header ");
        return false;
    }
    is_x64_ = (nt_header32_->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64
        || nt_header32_->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64);
    if (is_x64_) {
        nt_header64_ = (PIMAGE_NT_HEADERS64)nt_header32_;
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
    DWORD rva_export_table = is_x64_ ? nt_header64_->OptionalHeader.DataDirectory[0].VirtualAddress
        : nt_header32_->OptionalHeader.DataDirectory[0].VirtualAddress;
    if (rva_export_table == NULL) {
        return false;
    }
    PIMAGE_EXPORT_DIRECTORY image_export_dir = (PIMAGE_EXPORT_DIRECTORY)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        rva_export_table, NULL);

    //现在加载每个节点
    //以下全部是RVA。换算到数组入口处的VA：
    PDWORD functions = (PDWORD)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        image_export_dir->AddressOfFunctions,
        NULL);
    if (functions == NULL) {
        return true;
    }

    PWORD ordinals = (PWORD)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        image_export_dir->AddressOfNameOrdinals,
        NULL);
    if (ordinals == NULL) {
        return true;
    }

    if (image_export_dir->AddressOfNames == 0) {
        // 没有导出函数
        return true;
    }

    PDWORD pNames = (PDWORD)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        image_export_dir->AddressOfNames, NULL);
    for (DWORD i = 0; i < image_export_dir->NumberOfFunctions; i++)
    {
        //函数名称
        LPCSTR function_name = (LPCSTR)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
            pNames[i], NULL);
        if (function_name == NULL) {
            continue;
        }
        auto temp = ordinals[i];
        IMAGE_EXPORT_FUNCTION function;
        function.entry_point = functions[i];
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
    if (nt_header32_ == NULL) {
        return false;
    }
    DWORD rva_import_table = nt_header32_->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (rva_import_table == NULL) {
        return false;
    }
    PIMAGE_IMPORT_DESCRIPTOR import_table = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
        (PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        rva_import_table, NULL);
    //减去内存映射的首地址，就是文件地址了
    int i = 0;
    while (true) {
        IMAGE_IMPORT_DESCRIPTOR import_desc = { 0 };
        memcpy(&import_desc, import_table + i, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        if (import_desc.TimeDateStamp == NULL && import_desc.Name == NULL) {
            break;
        }
        LPCSTR dll_name = (LPCSTR)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
            import_table[i].Name, NULL);

        PIMAGE_THUNK_DATA32 thunk = (PIMAGE_THUNK_DATA32)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
            import_table[i].OriginalFirstThunk, //【注意】这里使用的是OriginalFirstThunk
            NULL);
        if (thunk == NULL) {
            DWORD code = ::GetLastError();
            break;
        }
        int j = 0;
        IMAGE_THUNK_DATA32 null_thunk = { 0 };
        // https://blog.csdn.net/zang141588761/article/details/50401203
        std::list<IMAGE_EXPORT_FUNCTION> function_list;
        while (true) {
            memcpy(&null_thunk, thunk + j, sizeof(IMAGE_THUNK_DATA32));
            if (null_thunk.u1.AddressOfData == 0 || null_thunk.u1.Function == 0) {
                break;
            }
            IMAGE_EXPORT_FUNCTION use_function;
            if (thunk[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG32) {
                // printf(" \t [%d] \t %ld \t 按序号导入\n ", j, thunk[j].u1.AddressOfData & 0xffff);
                use_function.ordinal = thunk[j].u1.AddressOfData & 0xffff;
            }
            else {
                PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
                    thunk[j].u1.AddressOfData,
                    NULL);
                if (pFuncName == NULL || pFuncName->Name == NULL) {
                    j++;
                    continue;
                }
                // printf(" \t [%d] \t %ld \t %s\n ", j, pFuncName->Hint, pFuncName->Name);
                use_function.hint = pFuncName->Hint;
                CopyStringByMalloc(&use_function.function_name, (const char*)pFuncName->Name);
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

bool PEHelper::GetImportDllsX64(std::list<IMAGE_IMPORT_DLL>& dll_list)
{
    if (nt_header64_ == NULL) {
        return false;
    }
    DWORD rva_import_table = nt_header64_->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (rva_import_table == NULL) {
        return false;
    }
    PIMAGE_IMPORT_DESCRIPTOR import_table = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
        (PIMAGE_NT_HEADERS)nt_header32_, base_address_,
        rva_import_table, NULL);
    //减去内存映射的首地址，就是文件地址了
    int i = 0;
    while (true) {
        IMAGE_IMPORT_DESCRIPTOR import_desc = { 0 };
        memcpy(&import_desc, import_table + i, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        if (import_desc.TimeDateStamp == NULL && import_desc.Name == NULL) {
            break;
        }
        LPCSTR dll_name = (LPCSTR)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
            import_table[i].Name, NULL);

        PIMAGE_THUNK_DATA64 thunk = (PIMAGE_THUNK_DATA64)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
            import_table[i].OriginalFirstThunk, //【注意】这里使用的是OriginalFirstThunk
            NULL);
        int j = 0;
        IMAGE_THUNK_DATA64 null_thunk = { 0 };
        // https://blog.csdn.net/zang141588761/article/details/50401203
        std::list<IMAGE_EXPORT_FUNCTION> function_list;
        while (true) {
            memcpy(&null_thunk, thunk + j, sizeof(IMAGE_THUNK_DATA64));
            if (null_thunk.u1.AddressOfData == 0 || null_thunk.u1.Function == 0) {
                break;
            }
            IMAGE_EXPORT_FUNCTION use_function;
            if (thunk[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG64) {
                // printf(" \t [%d] \t %ld \t 按序号导入\n ", j, thunk[j].u1.AddressOfData & 0xffff);
                use_function.ordinal = thunk[j].u1.AddressOfData & 0xffff;
            }
            else {
                PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa((PIMAGE_NT_HEADERS)nt_header32_, base_address_,
                    thunk[j].u1.AddressOfData,
                    NULL);
                if (pFuncName == NULL || pFuncName->Name == NULL) {
                    j++;
                    continue;
                }
                // printf(" \t [%d] \t %ld \t %s\n ", j, pFuncName->Hint, pFuncName->Name);
                use_function.hint = pFuncName->Hint;
                CopyStringByMalloc(&use_function.function_name, (const char*)pFuncName->Name);
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