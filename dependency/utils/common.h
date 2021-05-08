#pragma once
#include <string>

std::string GetExeRunPathA();
std::wstring GetExeRunPathW();

std::wstring GetFileNameByPath(const std::wstring& file_path);
std::wstring GetFileDirByPath(const std::wstring& file_path);

bool Is64bitSystem();

bool IsPeFile(const std::wstring& file_path);

bool Is64bitArchiteFileA(const std::string& file_path);

bool Is64bitArchiteFileW(const std::wstring& file_path);

std::string	UToA(const std::wstring& str);
std::wstring AToU(const std::string& str);

std::wstring ToHexString(DWORD value);

std::wstring GetWindowsPath();

std::wstring GetSystemPath();

std::wstring GetSystemWow64Path();

bool SearchDllPath(bool is_x64_archite, const std::wstring& dll_dir, const std::wstring& dll_name, std::wstring& dll_path);