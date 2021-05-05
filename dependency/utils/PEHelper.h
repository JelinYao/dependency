#pragma once
#include <list>

struct IMAGE_EXPORT_FUNCTION {
	char* function_name = NULL;
	DWORD thunkRVA = 0;
	DWORD	thunk = 0;
	WORD hint = 0;
	WORD ordinal = 0;
	DWORD entry_point = 0;
};

inline void ReleaseImageExportFunction(IMAGE_EXPORT_FUNCTION* p) {
	if (p) {
		free(p->function_name);
		p->function_name = NULL;
	}
}

inline void CopyStringByMalloc(char** dest, const char* source) {
	if (source == NULL) {
		return;
	}
	int length = strlen(source);
	char* p = (char*)malloc(length + 1);
	memcpy(p, source, length);
	p[length] = '\0';
	*dest = p;
}

struct IMAGE_IMPORT_DLL {
	char* dll_name = NULL;
	DWORD originalFirstThunk = 0;
	DWORD forwarderChain = 0;
	DWORD firstThunk = 0;
	std::list<IMAGE_EXPORT_FUNCTION> use_function_list;
};

inline void ReleaseImageImportDll(IMAGE_IMPORT_DLL* p) {
	if (p) {
		free(p->dll_name);
		p->dll_name = NULL;
	}
}

class PEHelper {
 public:
  PEHelper();
  PEHelper(const wchar_t* file);
	PEHelper(const std::wstring &file);
  ~PEHelper();

  bool Open(const wchar_t* file);
  void Close();

	bool GetExportFunctions(std::list<IMAGE_EXPORT_FUNCTION>& function_list);
	bool GetImportDlls(std::list<IMAGE_IMPORT_DLL>& dll_list);

 private:
	 LPVOID base_address_;
	 HANDLE map_handle_;
  HANDLE handle_;
  PIMAGE_NT_HEADERS nt_header_;
  PIMAGE_DOS_HEADER dos_header_;
  IMAGE_SECTION_HEADER* section_heaer_;
};