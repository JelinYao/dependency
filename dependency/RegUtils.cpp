#include "stdafx.h"
#include "RegUtils.h"

// ע����С����, �μ� https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
#define REG_KEY_NAME_CCH_LIMITS            (255)             // ע��������С����
#define REG_VALUE_NAME_CB_LIMITS          (16383)           // ע���ֵ����С����
#define REG_VALUE_DATA_CB_LIMITS          (1024 * 1024)     // ע���ֵ���ݴ�С����(���ܳ����˴�С)

//���ݻ���, ����Ϊ 1 MB, һ�����ݵ���󳤶Ȳ��ᳬ��������
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

// д�� REG_DWORD ��������
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

// ��ȡ REG_DWORD ��������
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

// д�� REG_QWORD ��������
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

// ��ȡ REG_QWORD ��������
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

// д�� REG_SZ ��������
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

// ��ȡ REG_SZ ��������
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

    // ���ݳ��ȳ��������С����, �����·���ռ�
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

// д�� REG_BINARY ��������
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

// ��ȡREG_BINARY��������
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

// ��ȡ���������ֽڴ�С
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

// ���� ָ����
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

// ɾ�� ָ����
bool CRegUtils::DelKey(HKEY hKey, const CString& lpSubKey)
{
    LSTATUS lr = ERROR_SUCCESS;
    lr = ::RegDeleteTree(hKey, lpSubKey.GetString());
    return ERROR_SUCCESS == lr;
}

// ɾ�� ָ��ֵ
bool CRegUtils::DelValue(HKEY hKey, const CString& lpSubKey, const CString& lpValueName)
{
    LSTATUS lr = ERROR_SUCCESS;
    lr = ::RegDeleteKeyValue(hKey, lpSubKey.GetString(), lpValueName.GetString());
    return ERROR_SUCCESS == lr;
}

// ö�� ָ���� ���Ӽ�
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

// ö�� ָ���� ��ֵ��
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