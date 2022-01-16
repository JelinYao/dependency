#include "stdafx.h"
#include "LanguageMgr.h"
#include "utils/common.h"

LanguageMgr::LanguageMgr()
{

}

LanguageMgr::~LanguageMgr()
{

}

bool LanguageMgr::LoadLanguage(const std::string& language)
{
    lang_texts_.clear();
    auto configPath = GetExeRunPathA() + "\\language\\" + language + ".xml";
    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_SUCCESS != doc.LoadFile(configPath.c_str())) {
        return false;
    }
    auto root = doc.RootElement();
    if (root == nullptr) {
        return false;
    }
    auto childElem = root->FirstChildElement("string");
    while (childElem) {
        auto id = childElem->Attribute("id");
        if (id == nullptr) {
            childElem = childElem->NextSiblingElement();
            continue;
        }
        auto text = childElem->GetText();
        if (text == nullptr || strlen(text) < 1) {
            childElem = childElem->NextSiblingElement();
            continue;
        }
        lang_texts_.emplace(id, Utf8ToU(text));
        childElem = childElem->NextSiblingElement();
    }
    return true;
}

const wchar_t* LanguageMgr::GetText(const std::string& key)
{
    auto iter = lang_texts_.find(key);
    if (iter == lang_texts_.end()) {
        return nullptr;
    }
    return iter->second.c_str();
}