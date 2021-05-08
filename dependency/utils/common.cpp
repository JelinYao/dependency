#include "stdafx.h"
#include "common.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

std::string GetExeRunPathA() {
  char module_path[MAX_PATH];
  ::GetModuleFileNameA(NULL, module_path, MAX_PATH);
  std::string str_path(module_path);
  int pos = str_path.rfind('\\');
  str_path = str_path.substr(0, pos);
  return str_path;
}

std::wstring GetExeRunPathW()
{
	wchar_t module_path[MAX_PATH];
	::GetModuleFileNameW(NULL, module_path, MAX_PATH);
	std::wstring str_path(module_path);
	int pos = str_path.rfind('\\');
	str_path = str_path.substr(0, pos);
	return str_path;
}

std::wstring GetFileNameByPath(const std::wstring& file_path) {
	size_t pos = file_path.rfind('\\');
	if (pos != std::wstring::npos) {
		return file_path.substr(pos + 1);
	}
	return L"";
}

std::wstring GetFileDirByPath(const std::wstring& file_path)
{
	size_t pos = file_path.rfind('\\');
	if (pos != std::wstring::npos) {
		return file_path.substr(0, pos);
	}
	return file_path;
}

void SafeGetNativeSystemInfo(LPSYSTEM_INFO lpSysInfo)
{
	if (NULL == lpSysInfo)
		return;
	typedef VOID(WINAPI *pfnGetNativeSystemInfo)(LPSYSTEM_INFO);
	pfnGetNativeSystemInfo pFun = (pfnGetNativeSystemInfo)GetProcAddress(GetModuleHandle(L"Kernel32"), "GetNativeSystemInfo");
	if (pFun)
		pFun(lpSysInfo);
	else
		GetSystemInfo(lpSysInfo);
}

bool Is64bitSystem()
{
	SYSTEM_INFO si;
	SafeGetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64
		|| si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		return true;
	return false;
}

bool IsPeFile(const std::wstring& file_path)
{
	HANDLE handle = ::CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	LARGE_INTEGER file_size = { 0 };
	::GetFileSizeEx(handle, &file_size);
	if (file_size.QuadPart <= (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS))) {
		::CloseHandle(handle);
		return false;
	}
	IMAGE_DOS_HEADER dos_header;
	DWORD read_bytes;
	::ReadFile(handle, &dos_header, sizeof(IMAGE_DOS_HEADER), &read_bytes, NULL);
	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
		::CloseHandle(handle);
		return false;
	}
	::SetFilePointer(handle, dos_header.e_lfanew, 0, FILE_BEGIN);
	IMAGE_NT_HEADERS nt_header;
	::ReadFile(handle, &nt_header, sizeof(IMAGE_NT_HEADERS), &read_bytes, NULL);
	if (nt_header.Signature != IMAGE_NT_SIGNATURE) {
		::CloseHandle(handle);
		return false;
	}
	::CloseHandle(handle);
	return true;
}

bool Is64bitArchiteFileA(const std::string& file_path)
{
	HANDLE handle = ::CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	IMAGE_DOS_HEADER dos_header;
	DWORD read_bytes;
	::ReadFile(handle, &dos_header, sizeof(IMAGE_DOS_HEADER), &read_bytes, NULL);
	::SetFilePointer(handle, dos_header.e_lfanew, 0, FILE_BEGIN);
	IMAGE_NT_HEADERS nt_header;
	::ReadFile(handle, &nt_header, sizeof(IMAGE_NT_HEADERS), &read_bytes, NULL);
	::CloseHandle(handle);
	return (nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64
		|| nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_IA64);
}

bool Is64bitArchiteFileW(const std::wstring& file_path)
{
	HANDLE handle = ::CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	IMAGE_DOS_HEADER dos_header;
	DWORD read_bytes;
	::ReadFile(handle, &dos_header, sizeof(IMAGE_DOS_HEADER), &read_bytes, NULL);
	::SetFilePointer(handle, dos_header.e_lfanew, 0, FILE_BEGIN);
	IMAGE_NT_HEADERS nt_header;
	::ReadFile(handle, &nt_header, sizeof(IMAGE_NT_HEADERS), &read_bytes, NULL);
	::CloseHandle(handle);
	return (nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64
		|| nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_IA64);
}

std::string UToA(const std::wstring& str)
{
	std::string strDes;
	if (str.empty())
		goto __end;
	int nLen = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	if (0 == nLen)
		goto __end;
	char* pBuffer = new char[nLen + 1];
	memset(pBuffer, 0, nLen + 1);
	::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
	pBuffer[nLen] = '\0';
	strDes.append(pBuffer);
	delete[] pBuffer;
__end:
	return strDes;
}

std::wstring AToU(const std::string& str)
{
	std::wstring strDes;
	if (str.empty())
		goto __end;
	int nLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if (0 == nLen)
		goto __end;
	wchar_t* pBuffer = new wchar_t[nLen + 1];
	memset(pBuffer, 0, nLen + 1);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen);
	pBuffer[nLen] = '\0';
	strDes.append(pBuffer);
	delete[] pBuffer;
__end:
	return strDes;
}

std::wstring ToHexString(DWORD value)
{
	wchar_t buffer[16] = { 0 };
	wsprintf(buffer, L"0x%p", value);
	return std::wstring(buffer);
}

std::wstring GetWindowsPath()
{
	std::wstring path;
	wchar_t buffer[MAX_PATH] = {0};
	::GetWindowsDirectory(buffer, MAX_PATH);
	path.assign(buffer);
	return path;
}

std::wstring GetSystemPath()
{
	std::wstring path;
	wchar_t buffer[MAX_PATH] = {0};
	::GetSystemDirectory(buffer, MAX_PATH);
	path.assign(buffer);
	return path;
}

std::wstring GetSystemWow64Path()
{
	std::wstring path;
	wchar_t buffer[MAX_PATH] = {0};
	::GetSystemWow64Directory(buffer, MAX_PATH);
	path.assign(buffer);
	return path;
}

bool SearchDllPath(bool is_x64_archite, const std::wstring& dll_dir, const std::wstring& dll_name, std::wstring& dll_path)
{
	if (dll_name.empty()) {
		return false;
	}
	static const auto system_path = GetSystemPath();
	static const auto system_wow64_path = GetSystemWow64Path();
	//from current path
	auto search_path = dll_dir + L"\\" + dll_name;
	if (PathFileExists(search_path.c_str())) {
		dll_path = std::move(search_path);
		return true;
	}
	search_path = (is_x64_archite ? system_path : system_wow64_path) + L"\\" + dll_name;
	if (PathFileExists(search_path.c_str())) {
		dll_path = std::move(search_path);
		return true;
	}
	return false;
}
