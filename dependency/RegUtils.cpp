#include "stdafx.h"
#include "RegUtils.h"

// 注册表大小限制, 参见 https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
#define REG_KEY_NAME_CCH_LIMITS            (255)             // 注册表键名大小限制
#define REG_VALUE_NAME_CB_LIMITS          (16383)           // 注册表值名大小限制
#define REG_VALUE_DATA_CB_LIMITS          (1024 * 1024)     // 注册表值数据大小限制(可能超过此大小)

//数据缓冲, 设置为 1 MB, 一般数据的最大长度不会超过此限制
static BYTE s_DataBuf[REG_VALUE_DATA_CB_LIMITS];

bool CRegUtils::SplitKey(const CString& strKey, HKEY& mainKey, CString& subKey)
{
    int pos = strKey.Find(_T('\\'));
    CString str = strKey.Left(pos);
    if (str == _T("HKEY_CLASSES_ROOT")) {
        mainKey = HKEY_CLASSES_ROOT;
    }else if (str == _T("HKEY_CURRENT_USER")) {
        mainKey = HKEY_CURRENT_USER;
    }else if (str == _T("HKEY_LOCAL_MACHINE")) {
        mainKey = HKEY_LOCAL_MACHINE;
    }else if (str == _T("HKEY_USERS")) {
        mainKey = HKEY_USERS;
    }else if (str == _T("HKEY_CURRENT_CONFIG")) {
        mainKey = HKEY_CURRENT_CONFIG;
    }
    else {
        return false; 
    }
    subKey = strKey.Right(strKey.GetLength() - pos - 1);
    return true; 
}

// 写入 REG_DWORD 类型数据
bool CRegUtils::SetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD dwValue)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegSetKeyValue(hKey, lpSubKey.GetString(), _T("DWORD"), REG_DWORD, &dwValue, sizeof(dwValue));
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 读取 REG_DWORD 类型数据
bool CRegUtils::GetDword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& dwValue)
{
    LSTATUS lr = ERROR_SUCCESS;
    DWORD cbSize = sizeof(dwValue);

    lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_REG_DWORD | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY, NULL, &dwValue, &cbSize);
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 写入 REG_QWORD 类型数据
bool CRegUtils::SetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG ullValue)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegSetKeyValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), REG_QWORD, &ullValue, sizeof(ullValue));
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 读取 REG_QWORD 类型数据
bool CRegUtils::GetQword(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, ULONGLONG& ullValue)
{
    LSTATUS lr = ERROR_SUCCESS;
    DWORD cbSize = sizeof(ullValue);

    lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_REG_QWORD | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY, NULL, &ullValue, &cbSize);
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 写入 REG_SZ 类型数据
bool CRegUtils::SetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, const CString& strValue)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegSetKeyValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), REG_SZ, strValue.GetString(), (DWORD)((strValue.GetLength() + 1) * sizeof(TCHAR)));
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 读取 REG_SZ 类型数据
bool CRegUtils::GetString(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, CString& strValue)
{
    LSTATUS lr = ERROR_SUCCESS;
    DWORD dwSize = sizeof(s_DataBuf);
    LPVOID lpBuf = NULL;

    lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_REG_SZ | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY, NULL, s_DataBuf, &dwSize);
    if (ERROR_SUCCESS == lr)
    {
        strValue = (LPCTSTR)s_DataBuf;
        return true;
    }

    if (ERROR_MORE_DATA != lr)
    {
        return false;
    }

    // 数据长度超过常规大小限制, 则重新分配空间
    lpBuf = HeapAlloc(GetProcessHeap(), 0, dwSize);
    if (lpBuf)
    {
        lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_REG_SZ | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY, NULL, lpBuf, &dwSize);
        if (ERROR_SUCCESS == lr)
        {
            strValue = (LPCTSTR)lpBuf;
        }

        HeapFree(GetProcessHeap(), 0, lpBuf);
    }

    return ERROR_SUCCESS == lr;
}

// 写入 REG_BINARY 类型数据
bool CRegUtils::SetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPCVOID lpData, DWORD cbSize)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegSetKeyValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), REG_BINARY, lpData, cbSize);
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    return ERROR_SUCCESS == lr;
}

// 读取REG_BINARY类型数据
bool CRegUtils::GetBinary(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, LPVOID lpData, DWORD cbSize)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_REG_BINARY | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY, NULL, lpData, &cbSize);
    if (ERROR_SUCCESS == lr)
    {
        return true;
    }

    return ERROR_SUCCESS == lr;
}

// 读取数据所需字节大小
bool CRegUtils::GetValueSize(HKEY hKey, const CString& lpSubKey, const CString& lpValueName, DWORD& cbSize)
{
    LSTATUS lr = ERROR_SUCCESS;

    lr = ::RegGetValue(hKey, lpSubKey.GetString(), lpValueName.GetString(), RRF_RT_ANY, NULL, s_DataBuf, &cbSize);
    if (ERROR_SUCCESS == lr)
    {
        return true;
    }

    return ERROR_SUCCESS == lr;
}

// 创建 指定键
bool CRegUtils::CreateKey(HKEY hKey, const CString& lpSubKey)
{
    LSTATUS lr = ERROR_SUCCESS;
    HKEY hNewKey = NULL;
    DWORD dwDisposition = 0;

    lr = ::RegCreateKeyEx(hKey, lpSubKey.GetString(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwDisposition);

    if (hNewKey)
    {
        ::RegCloseKey(hNewKey);
    }

    return ERROR_SUCCESS == lr;
}

// 删除 指定键
bool CRegUtils::DelKey(HKEY hKey, const CString& lpSubKey)
{
    LSTATUS lr = ERROR_SUCCESS;
    lr = ::RegDeleteTree(hKey, lpSubKey.GetString());
    return ERROR_SUCCESS == lr;
}

// 删除 指定值
bool CRegUtils::DelValue(HKEY hKey, const CString& lpSubKey, const CString& lpValueName)
{
    LSTATUS lr = ERROR_SUCCESS;
    lr = ::RegDeleteKeyValue(hKey, lpSubKey.GetString(), lpValueName.GetString());
    return ERROR_SUCCESS == lr;
}

// 枚举 指定键 的子键
bool CRegUtils::EnumKeyName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& keyList)
{
    TCHAR szKeyName[MAX_PATH] = { 0 };
    DWORD dwCchName = _countof(szKeyName);
    LSTATUS lr = ERROR_SUCCESS;
    HKEY hResultKey = NULL;
    DWORD dwIndex = 0;

    keyList.clear();

    lr = ::RegOpenKeyEx(hKey, lpSubKey.GetString(), 0, KEY_READ, &hResultKey);
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    do
    {
        dwCchName = _countof(szKeyName);
        lr = ::RegEnumKeyEx(hResultKey, dwIndex++, szKeyName, &dwCchName, NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS == lr)
        {
            keyList.push_back(szKeyName);
        }
    } while (ERROR_SUCCESS == lr);

    ::RegCloseKey(hResultKey);

    return true;
}

// 枚举 指定键 的值名
bool CRegUtils::EnumValueName(HKEY hKey, const CString& lpSubKey, std::vector<CString>& valueList)
{
    TCHAR szValueName[MAX_PATH] = { 0 };
    DWORD dwCchName = _countof(szValueName);
    LSTATUS lr = ERROR_SUCCESS;
    HKEY hResultKey = NULL;
    DWORD dwIndex = 0;

    valueList.clear();

    lr = ::RegOpenKeyEx(hKey, lpSubKey.GetString(), 0, KEY_READ, &hResultKey);
    if (ERROR_SUCCESS != lr)
    {
        return false;
    }

    do
    {
        dwCchName = _countof(szValueName);
        lr = ::RegEnumValue(hResultKey, dwIndex++, szValueName, &dwCchName, NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS == lr)
        {
            valueList.push_back(szValueName);
        }
    } while (ERROR_SUCCESS == lr);

    ::RegCloseKey(hResultKey);

    return true;
}