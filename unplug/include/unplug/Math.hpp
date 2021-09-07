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
#include <cmath>
#include <string>

namespace unplug {

inline static constexpr auto pi = static_cast<float>(M_PI);

inline double linearToDB(double linear)
{
  return 20.0 * std::log10(std::abs(linear) + std::numeric_limits<double>::epsilon());
}

inline double dBToLinear(double dB)
{
  return std::pow(10.0, dB / 20.0);
}

std::string linearToDBAsText(float linear);

std::string linearToDBAsTextWithDecimalDigits(float linear, int numDecimalDigits);

} // namespace unplug