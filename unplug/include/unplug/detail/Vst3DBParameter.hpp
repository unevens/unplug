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

namespace Steinberg::Vst {

class DBParameter : public Parameter
{
public:
  DBParameter(const TChar* title,
              ParamID tag,
              ParamValue minPlainInDB = -90.0,
              ParamValue maxPlainInDB = 6.0,
              ParamValue defaultValuePlain = 0.0,
              bool mapMinToLinearZero = true,
              int32 flags = ParameterInfo::kCanAutomate,
              UnitID unitID = kRootUnitId,
              const TChar* shortTitle = nullptr);

  double dBToLinear(double dB) const;

  ParamValue toPlain(ParamValue valueNormalized_) const override;

  ParamValue toNormalized(ParamValue plainValueInDB) const override;

  bool fromString(const TChar* string, ParamValue& valueNormalized_) const override;

  void toString(ParamValue _valueNormalized, String128 string) const override;

private:
  double normalizedToLinear(double normalized) const;
  double linearToNormalized(double linear) const;

  ParamValue minLinear;
  ParamValue maxLinear;
  bool mapMinToLinearZero;
};
} // namespace Steinberg::Vst