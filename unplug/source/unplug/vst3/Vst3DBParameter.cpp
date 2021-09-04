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
#include "unplug/Math.hpp"
#include "unplug/StringConversion.hpp"

namespace Steinberg::Vst {
DBParameter::DBParameter(const TChar* title,
                         ParamID tag,
                         ParamValue minPlainInDB,
                         ParamValue maxPlainInDB,
                         ParamValue defaultValuePlain,
                         bool mapMinToLinearZero,
                         int32 flags,
                         UnitID unitID,
                         const TChar* shortTitle)
  : Parameter(title, tag, unplug::ToVstTChar{}("dB").c_str(), defaultValuePlain, 0, flags, unitID, shortTitle)
  , mapMinToLinearZero(mapMinToLinearZero)
  , minLinear(dBToLinear(minPlainInDB))
  , maxLinear(dBToLinear(maxPlainInDB))
{}

double DBParameter::dBToLinear(double dB) const
{
  auto const linear = unplug::dBToLinear(dB);
  if (mapMinToLinearZero && linear <= minLinear)
    return 0.0;
  else
    return linear;
}

double DBParameter::normalizedToLinear(double normalized) const
{
  return minLinear + normalized * (maxLinear - minLinear);
}
double DBParameter::linearToNormalized(double linear) const
{
  return (linear - minLinear) / (maxLinear - minLinear);
}

ParamValue DBParameter::toPlain(ParamValue valueNormalized_) const
{
  auto const valueInLinearScale = normalizedToLinear(valueNormalized_);
  auto const valueInDB = unplug::linearToDB(valueInLinearScale);
  return valueInDB;
}

ParamValue DBParameter::toNormalized(ParamValue plainValueInDB) const
{
  auto const valueInLinearScale = dBToLinear(plainValueInDB);
  auto const valueNormalized = linearToNormalized(valueInLinearScale);
  return valueNormalized;
}

bool DBParameter::fromString(const TChar* string, ParamValue& valueNormalized_) const
{
  UString wrapper(const_cast<TChar*>(string), tstrlen(string));
  double valueInDB;
  if (wrapper.scanFloat(valueInDB)) {
    auto valueInLinearScale = dBToLinear(valueInDB);
    valueInLinearScale = std::max(minLinear, std::min(maxLinear, valueInLinearScale));
    valueNormalized_ = linearToNormalized(valueInLinearScale);
    return true;
  }
  return false;
}

void DBParameter::toString(ParamValue valueNormalized_, String128 string) const
{
  Parameter::toString(toPlain(valueNormalized_), string);
}

} // namespace Steinberg::Vst