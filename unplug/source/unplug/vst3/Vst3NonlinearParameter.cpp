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

#include "unplug/detail/Vst3NonlinearParameter.hpp"
#include "pluginterfaces/base/ustring.h"
#include "unplug/Math.hpp"
#include "unplug/StringConversion.hpp"

namespace Steinberg::Vst {
NonlinearParameter::NonlinearParameter(const TChar* title,
                                       ParamID tag,
                                       std::function<double(double)> nonlinearToLinear_,
                                       std::function<double(double)> linearToNonlinear_,
                                       ParamValue minInNonlinearScale,
                                       ParamValue maxInNonlinearScale,
                                       ParamValue defaultValueInNonlinearScale,
                                       int32 flags,
                                       const TChar* units,
                                       UnitID unitID,
                                       const TChar* shortTitle)
  : Parameter(title,
              tag,
              units,
              nonlinearToLinear_(defaultValueInNonlinearScale),
              0,
              flags,
              unitID,
              shortTitle)
  , nonlinearToLinear(std::move(nonlinearToLinear_))
  , linearToNonlinear(std::move(linearToNonlinear_))
  , minLinear(nonlinearToLinear(minInNonlinearScale))
  , maxLinear(nonlinearToLinear(maxInNonlinearScale))
{}

double NonlinearParameter::normalizedToLinear(double normalized) const
{
  return minLinear + normalized * (maxLinear - minLinear);
}

double NonlinearParameter::linearToNormalized(double linear) const
{
  return (linear - minLinear) / (maxLinear - minLinear);
}

ParamValue NonlinearParameter::toPlain(ParamValue valueNormalized_) const
{
  auto const valueInLinearScale = normalizedToLinear(valueNormalized_);
  auto const valueInNonlinearScale = linearToNonlinear(valueInLinearScale);
  return valueInNonlinearScale;
}

ParamValue NonlinearParameter::toNormalized(ParamValue plainValueInNonlinearScale) const
{
  auto const valueInLinearScale = nonlinearToLinear(plainValueInNonlinearScale);
  auto const valueNormalized = linearToNormalized(valueInLinearScale);
  return valueNormalized;
}

bool NonlinearParameter::fromString(const TChar* string, ParamValue& valueNormalized_) const
{
  UString wrapper(const_cast<TChar*>(string), tstrlen(string));
  double valueInNonlinearScale;
  if (wrapper.scanFloat(valueInNonlinearScale)) {
    auto valueInLinearScale = nonlinearToLinear(valueInNonlinearScale);
    valueInLinearScale = std::max(minLinear, std::min(maxLinear, valueInLinearScale));
    valueNormalized_ = linearToNormalized(valueInLinearScale);
    return true;
  }
  return false;
}

void NonlinearParameter::toString(ParamValue valueNormalized_, String128 string) const
{
  Parameter::toString(toPlain(valueNormalized_), string);
}

} // namespace Steinberg::Vst