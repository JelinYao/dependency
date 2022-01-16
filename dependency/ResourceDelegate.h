#pragma once

class ResourceDelegate {
public:
    virtual void OnSwitchLanguage() = 0;
    virtual void OnAddLanguage(const wchar_t* text) = 0;
};