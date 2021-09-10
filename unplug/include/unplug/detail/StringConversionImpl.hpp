/*
### Copyright (c) Dusan B. Jovanovic (dbj@dbj.org)

*All rights reserved.*

Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.
1. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.
1. Neither the names DBJ, dbjdbj or Dusan B. Jovanovic nor the names of the contributors may be used to endorse or
promote products derived from this software without specific prior written permission.

## Caveat Emptor

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.

IN NO EVENT SHALL Dusan B. Jovanovic BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// https://godbolt.org/z/bj6vzj
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace dbj {

template<typename C>
struct is_char
  : std::integral_constant<bool,
                           std::is_same<C, char>::value ||
                             //    std::is_same<C, char8_t>::value  ||
                             std::is_same<C, char16_t>::value || std::is_same<C, char32_t>::value ||
                             std::is_same<C, wchar_t>::value>
{};

// inspired by https://nlitsme.github.io/2019/10/c-type_tests/
struct not_this_one
{}; // Tag type for detecting which begin/ end are being selected

// Import begin/ end from std here so they are considered
// alongside the fallback (...) overloads in this namespace
using std::begin;
using std::end;

not_this_one begin(...);
not_this_one end(...);

template<typename T>
struct is_range
{
  constexpr static const bool value = !std::is_same<decltype(begin(std::declval<T>())), not_this_one>::value &&
                                      !std::is_same<decltype(end(std::declval<T>())), not_this_one>::value;
};

template<typename T>
constexpr inline bool is_range_v = dbj::is_range<T>::value;

namespace inner {
// (c) 2018 - 2021 by dbj.org,
// Disclaimer, Terms and Conditions:
// https://dbj.org/dbj_license
//
template<typename return_type>
struct meta_converter final
{
  template<typename T>
  return_type operator()(T arg)
  {
    if constexpr (dbj::is_range_v<T>) {
      static_assert(
        // arg must have this typedef
        dbj::is_char<typename T::value_type>{}(),
        "can not transform ranges not made of std char types");
      return { arg.begin(), arg.end() };
    }
    else {
      using actual_type = std::remove_cv_t<std::remove_pointer_t<T>>;
      return this->operator()(std::basic_string<actual_type>{ arg });
    }
  }
}; // meta_converter
} // namespace inner
// all the types required / implicit instantiations
using char_range_to_string = inner::meta_converter<std::string>;
using wchar_range_to_string = inner::meta_converter<std::wstring>;
using u16char_range_to_string = inner::meta_converter<std::u16string>;
using u32char_range_to_string = inner::meta_converter<std::u32string>;

} // namespace dbj