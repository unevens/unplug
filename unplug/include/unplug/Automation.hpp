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

#include "Parameters.hpp"
#include "unplug/ProcessingData.hpp"

namespace unplug {

template<class SampleType>
struct AutomationCache
{
  struct ParameterCache
  {
    SampleType currentValue = 0.f;
    SampleType delta = 0.f;
  };
  std::array<ParameterCache, unplug::NumParameters::value> parameters;
  SampleType increment(ParamIndex paramIndex)
  {
    auto& parameter = parameters[paramIndex];
    parameter.currentValue += parameter.delta;
    return parameter.currentValue;
  }

  explicit AutomationCache(ParameterStorage const& parameterStorage)
  {
    for (ParamIndex paramIndex = 0; paramIndex < unplug::NumParameters::value; ++paramIndex) {
      parameters[paramIndex].currentValue = parameterStorage.get(paramIndex);
    }
  }
};

template<class SampleType>
void setParameterAutomation(AutomationCache<SampleType>& automationCache,
                            ParamIndex paramIndex,
                            SampleType firstSample,
                            SampleType valueAtFirstSample,
                            SampleType lastSample,
                            SampleType valueAtLastSample)
{
  auto& parameter = automationCache.parameters[paramIndex];
  parameter.currentValue = valueAtFirstSample;
  parameter.delta = (valueAtLastSample - valueAtFirstSample) / (lastSample - firstSample);
}

} // namespace unplug