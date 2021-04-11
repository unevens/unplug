#pragma once
#include "detail/StringConversionImpl.hpp"

namespace unplug {

using ToUtf8 = dbj::char_range_to_string;

#if defined(_NATIVE_WCHAR_T_DEFINED) || defined(__MINGW32__)
using ToVstTChar = dbj::wchar_range_to_string;
using TString = std::wstring;
#else
using ToVstTChar = dbj::u16char_range_to_string;
using TString = std::u16string;
#endif

} // namespace unplug