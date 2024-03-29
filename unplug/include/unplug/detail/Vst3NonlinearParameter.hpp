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

#include "public.sdk/source/vst/vstparameters.h"
#include <functional>

namespace Steinberg::Vst {

class NonlinearParameter : public Parameter
{
public:
  NonlinearParameter(const TChar* title,
                     ParamID tag,
                     std::function<double(double)> nonlinearToLinear,
                     std::function<double(double)> linearToNonlinear,
                     ParamValue minInNonlinearScale = -90.0,
                     ParamValue maxInNonlinearScale = 6.0,
                     ParamValue defaultValueInNonlinearScale = 0.0,
                     int32 flags = ParameterInfo::kCanAutomate,
                     const TChar* units = nullptr,
                     UnitID unitID = kRootUnitId,
                     const TChar* shortTitle = nullptr);

  ParamValue toPlain(ParamValue valueNormalized_) const override;

  ParamValue toNormalized(ParamValue plainValueInDB) const override;

  bool fromString(const TChar* string, ParamValue& valueNormalized_) const override;

  void toString(ParamValue _valueNormalized, String128 string) const override;

private:
  double normalizedToLinear(double normalized) const;
  double linearToNormalized(double linear) const;

  std::function<double(double)> nonlinearToLinear;
  std::function<double(double)> linearToNonlinear;

  ParamValue minLinear;
  ParamValue maxLinear;
};

} // namespace Steinberg::Vst