#ifndef UTFHELPERS_H
#define UTFHELPERS_H

#include <string>

#ifdef WINDOWS_UTF16
std::string utf16ToUtf8(std::wstring const& utf16);
std::wstring utf8ToUtf16(std::string const& utf8);
#endif

#endif // UTFHELPERS_H
