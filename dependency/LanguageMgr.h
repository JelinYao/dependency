#pragma once
#include <map>

class LanguageMgr
{
public:
    LanguageMgr();
    ~LanguageMgr();

    bool LoadLanguage(const std::string& language);
    const wchar_t* GetText(const std::string& key);

protected:

private:
    std::map<std::string, std::wstring> lang_texts_;
};

