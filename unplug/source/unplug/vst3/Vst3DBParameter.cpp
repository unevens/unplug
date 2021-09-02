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

#include "unplug/detail/Vst3DBParameter.hpp"
#include "pluginterfaces/base/ustring.h"
#include "unplug/StringConversion.hpp"
#include <cmath>

namespace Steinberg::Vst {
DBParameter::DBParameter(const TChar* title,
                         ParamID tag,
                         ParamValue minPlainInDB,
                         ParamValue maxPlainInDB,
                         ParamValue defaultValuePlain,
                         ParamValue linearZeroInDB,
                         int32 stepCount,
                         int32 flags,
                         UnitID unitID,
                         const TChar* shortTitle)
  : RangeParameter(title,
                   tag,
                   unplug::ToVstTChar{}("dB").c_str(),
                   dBToLinear(minPlainInDB),
                   dBToLinear(maxPlainInDB),
                   defaultValuePlain,
                   stepCount,
                   flags,
                   unitID,
                   shortTitle)
  , linearZeroInDB(linearZeroInDB)
{}

double
DBParameter::linearToDB(double linear) const
{
  auto const inDB = 10.0 * std::log10(std::abs(linear) + std::numeric_limits<double>::epsilon());
  return std::max(inDB, linearZeroInDB);
}
double
DBParameter::dBToLinear(double dB) const
{
  if (dB <= linearZeroInDB)
    return 0.0;
  else
    return std::pow(10.0, dB / 10.0);
}
ParamValue
DBParameter::toPlain(ParamValue valueNormalized_) const
{
  auto const valueInLinearScale = RangeParameter::toPlain(valueNormalized_);
  auto const valueInDB = linearToDB(valueInLinearScale);
  return valueInDB;
}
ParamValue
DBParameter::toNormalized(ParamValue plainValueInDB) const
{
  auto const valueInLinearScale = dBToLinear(plainValueInDB);
  auto const valueNormalized = RangeParameter::toNormalized(valueInLinearScale);
  return valueNormalized;
}
bool
DBParameter::fromString(const TChar* string, ParamValue& valueNormalized_) const
{
  if (info.stepCount > 1) {
    return RangeParameter::fromString(string, valueNormalized_);
  }
  UString wrapper(const_cast<TChar*>(string), tstrlen(string));
  double valueInDB;
  if (wrapper.scanFloat(valueInDB)) {
    auto valueInLinearScale = dBToLinear(valueInDB);
    valueInLinearScale = std::max(getMin(), std::min(getMax(), valueInLinearScale));
    valueNormalized_ = RangeParameter::toNormalized(valueInLinearScale);
    return true;
  }
  return false;
}

} // namespace Steinberg::Vst