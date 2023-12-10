#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <string>
#include <vector>
#include "stdafx.h"
#include <atlstr.h>

#define HKCR                (HKEY_CLASSES_ROOT)
#define HKCU                (HKEY_CURRENT_USER)
#define HKLM                (HKEY_LOCAL_MACHINE)
#define HKU                 (HKEY_USERS)

#define HKPD                (HKEY_PERFORMANCE_DATA)
#define HKPT                (HKEY_PERFORMANCE_TEXT)
#define HKPN                (HKEY_PERFORMANCE_NLSTEXT)

#if(WINVER >= 0x0400)
#define HKCC                (HKEY_CURRENT_CONFIG)
#define HKDD                (HKEY_DYN_DATA)
#define HKCULS              (HKEY_CURRENT_USER_LOCAL_SETTINGS)
#endif

class CRegUtils
{
public:
    static bool SplitKey(const CString& strKey , HKEY& mainKey , CString& subKey);
    // 写入 REG_DWORD 类型数据
    static bool SetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD dwValue);

    // 读取 REG_DWORD 类型数据
    static bool GetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& dwValue);

    // 写入 REG_QWORD 类型数据
    static bool SetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG ullValue);

    // 读取 REG_QWORD 类型数据
    static bool GetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG& ullValue);

    // 写入 REG_SZ 类型数据
    static bool SetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, const CString& strValue);

    // 读取 REG_SZ 类型数据
    static bool GetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, CString& strValue);

    // 写入 REG_BINARY 类型数据
    static bool SetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPCVOID lpData, DWORD dwSize);

    // 读取 REG_BINARY 类型数据
    static bool GetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPVOID lpData, DWORD cbSize);

    // 读取 指定值 所需字节大小
    static bool GetValueSize(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& cbSize);

    // 创建 指定键
    static bool CreateKey(HKEY hKey, const CString& lpSubKey);

    // 删除 指定键(递归删除)
    static bool DelKey(HKEY hKey, const CString& lpSubKey);

    // 删除 指定值
    static bool DelValue(HKEY hKey, const CString& lpSubKey, const CString& lpValueName);

    // 枚举 指定键 的子键
    static bool EnumKeyName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& keyList);

    // 枚举 指定键 的值名
    static bool EnumValueName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& valueList);
};