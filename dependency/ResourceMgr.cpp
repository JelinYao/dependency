#include "stdafx.h"
#include "ResourceMgr.h"
#include "utils/common.h"
#include "LanguageMgr.h"

ResourceMgr* ResourceMgr::Instance() {
    static ResourceMgr mgr;
    return &mgr;
}

ResourceMgr::ResourceMgr()
    : language_mgr_(std::make_shared<LanguageMgr>())
{
}

ResourceMgr::~ResourceMgr()
{
}

bool ResourceMgr::Init(ResourceDelegate* delegate)
{
    delegate_ = delegate;
    if (!ParseConfig()) {
        return false;
    }
    return true;
}

void ResourceMgr::Exit()
{
    delegate_ = nullptr;
}

const wchar_t* ResourceMgr::GetText(const std::string& key)
{
    if (key.empty()) {
        return nullptr;
    }
    if (!language_mgr_) {
        return nullptr;
    }
    return language_mgr_->GetText(key);
}

const wchar_t* ResourceMgr::GetText(const char* key)
{
    if (key == nullptr) {
        return nullptr;
    }
    if (!language_mgr_) {
        return nullptr;
    }
    return language_mgr_->GetText(key);
}

void ResourceMgr::SwitchLanguage(int index)
{
    if (index < 0 || index >= (int)language_list_.size() || !language_mgr_) {
        return;
    }
    if (select_language_id_ == index) {
        return;
    }
    select_language_id_ = index;
    auto& language = language_list_[select_language_id_];
    language_mgr_->LoadLanguage(UToUtf8(language));
    if (delegate_) {
        delegate_->OnSwitchLanguage();
    }
}

bool ResourceMgr::ParseConfig()
{
    auto configPath = GetExeRunPathA() + "\\config.xml";
    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_SUCCESS != doc.LoadFile(configPath.c_str())) {
        return false;
    }
    auto root = doc.RootElement();
    if (root == nullptr) {
        return false;
    }
    auto language = root->FirstChildElement("language");
    if (nullptr == language) {
        return false;
    }
    auto attribute = language->FindAttribute("selected");
    int index = 0;
    if (attribute) {
        auto selected = attribute->Value();
        if (selected && strlen(selected) > 0) {
            index = atoi(selected);
        }
    }
    auto childElem = language->FirstChildElement();
    while (childElem) {
        auto text = childElem->Attribute("text");
        if (text && delegate_) {
            auto unicodeText = Utf8ToU(text);
            delegate_->OnAddLanguage(unicodeText.c_str());
        }
        text = childElem->GetText();
        if (text && strlen(text) > 0) {
            language_list_.emplace_back(Utf8ToU(text));
        }
        childElem = childElem->NextSiblingElement();
    }
    SwitchLanguage(index);
    return true;
}
