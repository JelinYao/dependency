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
    // д�� REG_DWORD ��������
    static bool SetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD dwValue);

    // ��ȡ REG_DWORD ��������
    static bool GetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& dwValue);

    // д�� REG_QWORD ��������
    static bool SetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG ullValue);

    // ��ȡ REG_QWORD ��������
    static bool GetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG& ullValue);

    // д�� REG_SZ ��������
    static bool SetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, const CString& strValue);

    // ��ȡ REG_SZ ��������
    static bool GetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, CString& strValue);

    // д�� REG_BINARY ��������
    static bool SetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPCVOID lpData, DWORD dwSize);

    // ��ȡ REG_BINARY ��������
    static bool GetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPVOID lpData, DWORD cbSize);

    // ��ȡ ָ��ֵ �����ֽڴ�С
    static bool GetValueSize(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& cbSize);

    // ���� ָ����
    static bool CreateKey(HKEY hKey, const CString& lpSubKey);

    // ɾ�� ָ����(�ݹ�ɾ��)
    static bool DelKey(HKEY hKey, const CString& lpSubKey);

    // ɾ�� ָ��ֵ
    static bool DelValue(HKEY hKey, const CString& lpSubKey, const CString& lpValueName);

    // ö�� ָ���� ���Ӽ�
    static bool EnumKeyName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& keyList);

    // ö�� ָ���� ��ֵ��
    static bool EnumValueName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& valueList);
};