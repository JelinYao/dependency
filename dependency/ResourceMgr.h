#pragma once
#include <memory>
#include "ResourceDelegate.h"

#define RES_TEXT(x)    ResourceMgr::Instance()->GetText(x)

class LanguageMgr;
class ResourceMgr {
public:
    static ResourceMgr* Instance();

    bool Init(ResourceDelegate* delegate);
    void Exit();
    const wchar_t* GetText(const std::string& key);
    const wchar_t* GetText(const char* key);
    void SwitchLanguage(int index);
    const std::vector<std::wstring>& GetLanguageList()const {
        return language_list_;
    }
    int GetSelLanguageId() const {
        return select_language_id_;
    }

protected:
    ResourceMgr();
    ~ResourceMgr();
    bool ParseConfig();

private:
    int select_language_id_ = -1;
    std::vector<std::wstring> language_list_;
    std::shared_ptr<LanguageMgr> language_mgr_;
    ResourceDelegate* delegate_ = nullptr;
};

