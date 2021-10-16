//------------------------------------------------------------------------
// Copyright(c) 2021 Dario Mambro.
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.
//------------------------------------------------------------------------

#pragma once
#include "imgui.h"
#include "imgui_internal.h" //ImVec2 and ImVec4 operators
#include "unplug/Index.hpp"
#include <cmath>
#include <string>

namespace unplug {

template<class T>
inline static constexpr T pi = static_cast<T>(M_PI);

template<class T = double>
T linearToDB(T linear)
{
  return T(20) * std::log10(std::abs(linear) + std::numeric_limits<T>::epsilon());
}

template<class T = double>
T dBToLinear(T dB)
{
  return std::pow(T(10), dB / T(20));
}

std::string linearToDBAsText(float linear);

std::string linearToDBAsTextWithDecimalDigits(float linear, int numDecimalDigits);

struct FractionalIndex final
{
  float value;
  Index integer;
  float fractional;

  explicit FractionalIndex(float value)
    : value(value)
    , integer(static_cast<Index>(std::trunc(value)))
    , fractional(value - static_cast<float>(integer))
  {}
};

inline ImVec4 operator*(ImVec4 const& lhs, float rhs) noexcept
{
  return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
}

inline ImVec4 mix(ImVec4 a, ImVec4 b, float amountOfB)
{
  return a + (b - a) * amountOfB;
}

inline ImVec4 mix(ImVec4 a, ImVec4 b, ImVec4 intermediate, float amountOfB, float intermediatePoint = 0.5f)
{
  return amountOfB > intermediatePoint
           ? intermediate + (b - intermediate) * ((amountOfB - intermediatePoint) / (1.f - intermediatePoint))
           : a + (intermediate - a) * (amountOfB / intermediatePoint);
}

} // namespace unplug