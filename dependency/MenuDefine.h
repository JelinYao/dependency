#pragma once

// maindlg menu
static CONSTEXPR char* kMenuList[] = {
        language::kFile,
        language::kView,
        language::kLanguage,
        language::kHelp,
};
static CONSTEXPR int kSubMenuCount = _countof(kMenuList);

// maindlg file menu
static CONSTEXPR char* kFileMenuList[] = {
    language::kOpenFile,    
    language::kCloseFile,
    language::kExit,
};
static CONSTEXPR int kFileMenuCount = _countof(kFileMenuList);
// view menu
static CONSTEXPR char* kViewMenuList[] = {
    language::kExpendAll,
    language::kCollapseAll,
};
static CONSTEXPR int kViewMenuCount = _countof(kViewMenuList);
// help menu
static CONSTEXPR char* kHelpMenuList[] = {
    language::kAddToSystemMenu,
    nullptr,
    language::kAbout,
};
static CONSTEXPR int kHelpMenuCount = _countof(kHelpMenuList);

struct SubMenuItem {
    int count;
    const char* array;
};

static const SubMenuItem kSubMenus[kSubMenuCount] = {
    {kFileMenuCount, (const char*)kFileMenuList},
    {kViewMenuCount, (const char*)kViewMenuList},
    {0, nullptr}, // language menu
    {kHelpMenuCount, (const char*)kHelpMenuList},
};
