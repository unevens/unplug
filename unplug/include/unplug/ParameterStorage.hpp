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
#include "unplug/Index.hpp"
#include "unplug/ParameterDescription.hpp"
#include <array>
#include <atomic>
#include <unordered_set>

#ifdef UNPLUG_VST3
namespace Steinberg::Vst {
class UnplugProcessor;
}
#endif

namespace unplug {

/**
 * This class holds the values of the parameters used by the dsp code
 * */
template<int numParameters>
class TParameterStorage final
{
#ifdef UNPLUG_VST3
  friend class Steinberg::Vst::UnplugProcessor;
#endif

  class ParameterNormalization final
  {
  public:
    explicit ParameterNormalization(ParameterValueType min = 0, ParameterValueType max = 1);
    ParameterValueType toNormalized(ParameterValueType x) const;
    ParameterValueType fromNormalized(ParameterValueType x) const;

  private:
    ParameterValueType offset;
    ParameterValueType range;
  };

  struct StoredParameter final
  {
    std::atomic<ParameterValueType> value{ 0 };
    ParameterNormalization convert;
  };

public:
  /**
   * Sets the value of a parameter
   * @paramIndex the index of the parameter to set
   * @value the value to set the parameter to
   * */
  void set(ParamIndex paramIndex, ParameterValueType value);

  /**
   * Gets the value of a parameter
   * @paramIndex the index of the parameter to get
   * @return the value of the parameter
   * */
  ParameterValueType get(ParamIndex paramIndex) const;

  /**
   * Sets the value of a parameter from a normalized value
   * @paramIndex the index of the parameter to set
   * @value the normalized value to set the parameter to
   * */
  ParameterValueType setNormalized(ParamIndex paramIndex, ParameterValueType valueNormalized);

  /**
   * Gets the normalized value of a parameter
   * @paramIndex the index of the parameter to get
   * @return the normalized value of the parameter
   * */
  ParameterValueType getNormalized(ParamIndex paramIndex) const;

  /**
   * Converts a normalized value of to plain value for a specific parameter
   * @paramIndex the index of the parameter
   * @valueNormalized the normalized value to convert
   * @return the plain value
   * */
  ParameterValueType valueFromNormalized(ParamIndex paramIndex, ParameterValueType valueNormalized);

  TParameterStorage() = default;

  TParameterStorage(TParameterStorage const&) = delete;

  TParameterStorage& operator=(TParameterStorage const&) = delete;

  bool isParameterAutomatable(ParamIndex paramIndex) const
  {
    return !notAutomatalbeParameters.contains(paramIndex);
  }

  uint32_t getNumNotAutomatableParameters() const
  {
    return notAutomatalbeParameters.size();
  }

private:
  void initialize(std::vector<ParameterDescription> const& parameterDescriptions);

  void initializeConversions(std::vector<ParameterDescription> const& parameterDescriptions);

  void initializeDefaultValues(std::vector<ParameterDescription> const& parameterDescriptions);

  void initializeNotAutomatableParameters(std::vector<ParameterDescription> const& parameterDescriptions);

  std::array<StoredParameter, numParameters> parameters;
  std::unordered_set<ParamIndex> notAutomatalbeParameters;
};

using ParameterStorage = TParameterStorage<NumParameters::value>;

// implementation

template<int numParameters>
void TParameterStorage<numParameters>::set(ParamIndex paramIndex, ParameterValueType value)
{
  parameters[paramIndex].value.store(value, std::memory_order_release);
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::get(ParamIndex paramIndex) const
{
  return parameters[paramIndex].value.load(std::memory_order_acquire);
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::setNormalized(ParamIndex paramIndex,
                                                                   ParameterValueType valueNormalized)
{
  auto const value = valueFromNormalized(paramIndex, valueNormalized);
  set(paramIndex, value);
  return value;
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::getNormalized(ParamIndex paramIndex) const
{
  return parameters[paramIndex].convert.toNormalized(get(paramIndex));
}

template<int numParameters>
void TParameterStorage<numParameters>::initialize(const std::vector<ParameterDescription>& parameterDescriptions)
{
  initializeConversions(parameterDescriptions);
  initializeDefaultValues(parameterDescriptions);
  initializeNotAutomatableParameters(parameterDescriptions);
}

template<int numParameters>
void TParameterStorage<numParameters>::initializeConversions(
  const std::vector<ParameterDescription>& parameterDescriptions)
{
  int i = 0;
  for (auto& parameter : parameterDescriptions) {
    auto const min = parameter.isNonlinear() ? parameter.nonlinearToLinear(parameter.min) : parameter.min;
    auto const max = parameter.isNonlinear() ? parameter.nonlinearToLinear(parameter.max) : parameter.max;
    parameters[i].convert = ParameterNormalization{ min, max };
    ++i;
  }
}

template<int numParameters>
void TParameterStorage<numParameters>::initializeDefaultValues(
  const std::vector<ParameterDescription>& parameterDescriptions)
{
  for (int i = 0; i < parameterDescriptions.size(); ++i) {
    auto const& parameter = parameterDescriptions[i];
    auto const defaultValue =
      parameter.isNonlinear() ? parameter.nonlinearToLinear(parameter.defaultValue) : parameter.defaultValue;
    parameters[i].value.store(defaultValue);
  }
}

template<int numParameters>
void TParameterStorage<numParameters>::initializeNotAutomatableParameters(
  const std::vector<ParameterDescription>& parameterDescriptions)
{
  for (auto& description : parameterDescriptions) {
    if (description.editPolicy != ParamEditPolicy::automatable) {
      notAutomatalbeParameters.insert(description.index);
    }
  }
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::valueFromNormalized(ParamIndex paramIndex,
                                                                         ParameterValueType valueNormalized)
{
  return parameters[paramIndex].convert.fromNormalized(valueNormalized);
}

template<int numParameters>
TParameterStorage<numParameters>::ParameterNormalization::ParameterNormalization(ParameterValueType min,
                                                                                 ParameterValueType max)
  : range(max - min)
  , offset(min)
{}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::ParameterNormalization::toNormalized(ParameterValueType x) const
{
  return (x - offset) / range;
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::ParameterNormalization::fromNormalized(ParameterValueType x) const
{
  return x * range + offset;
}

} // namespace unplug