#pragma once
#include <string>

#if ENABLE_CONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR const
#endif
namespace language {
    static CONSTEXPR char* kFile = "File";
    static CONSTEXPR char* kView = "View";
    static CONSTEXPR char* kLanguage = "Language";
    static CONSTEXPR char* kHelp = "Help";
    static CONSTEXPR char* kOpenFile = "OpenFile";
    static CONSTEXPR char* kCloseFile = "CloseFile";
    static CONSTEXPR char* kExit = "Exit";
    static CONSTEXPR char* kExpendAll = "ExpendAll";
    static CONSTEXPR char* kCollapseAll = "CollapseAll";
    static CONSTEXPR char* kAbout = "About";
    static CONSTEXPR char* kOpenInvalidFile = "OpenInvalidFile";
    static CONSTEXPR char* kTipTitle = "TipTitle";
    static CONSTEXPR char* kErrorTitle = "ErrorTitle";
    static CONSTEXPR char* kMsgDragMuiltipleFiles = "MsgDragMuiltipleFiles";
    static CONSTEXPR char* kMsgCallFailed = "MsgCallFailed";
} // namespace language
