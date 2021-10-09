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
#include "unplug/AutomationEvent.hpp"
#include "unplug/PluginState.hpp"

namespace unplug {
/**
 * LinearAutomation provides the logic and the buffer to automate the plugin parameters using linear interpolation
 * */
template<class SampleType>
struct LinearAutomation
{
  struct ParameterCache
  {
    SampleType currentValue = 0.f;
    SampleType delta = 0.f;
  };

  std::array<ParameterCache, unplug::NumParameters::value> parameters;

  /**
   * Computes the next value of the parameter. Call this one per frame.
   * @paramIndex the index of the parameter
   * @return the next value of the parameter
   * */
  SampleType next(ParamIndex paramIndex)
  {
    auto& parameter = parameters[paramIndex];
    parameter.currentValue += parameter.delta;
    return parameter.currentValue;
  }

  /**
   * Constructor
   * @parameterStorage a reference to the parameter storage owned by the plugin processor
   * */
  explicit LinearAutomation(ParameterStorage const& parameterStorage)
  {
    for (ParamIndex paramIndex = 0; paramIndex < unplug::NumParameters::value; ++paramIndex) {
      parameters[paramIndex].currentValue = parameterStorage.get(paramIndex);
    }
  }
};

/**
 * Apply an automation event (received from the host), to the corresponding parameter in the LinearAutomation object.
 * @automation the LinearAutomation object to apply the AutomationEvent to
 * @automationEvent, the AutomationEvent to apply
 * */
template<class SampleType>
void setParameterAutomation(LinearAutomation<SampleType>& automation,
                            AutomationEvent<SampleType> const& automationEvent)
{
  auto& parameter = automation.parameters[automationEvent.paramIndex];
  parameter.currentValue = automationEvent.valueAtFirstSample;
  parameter.delta = (automationEvent.valueAtLastSample - automationEvent.valueAtFirstSample) /
                    (automationEvent.lastSample - automationEvent.firstSample);
}

} // namespace unplug