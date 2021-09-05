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
#include "unplug/ParameterDescription.hpp"
#include <array>
#include <atomic>

namespace unplug {

template<int numParameters>
class TParameterStorage final
{
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
  void set(int index, ParameterValueType value);

  ParameterValueType get(int index) const;

  void setNormalized(int index, ParameterValueType value);

  ParameterValueType getNormalized(int index) const;

  void initialize(std::vector<ParameterDescription> const& parameterDescriptions);

private:
  void initializeConversions(std::vector<ParameterDescription> const& parameterDescriptions);

  void initializeDefaultValues(std::vector<ParameterDescription> const& parameterDescriptions);

  std::array<StoredParameter, numParameters> parameters;
};

using ParameterStorage = TParameterStorage<NumParameters::value>;

// implementation

template<int numParameters>
void TParameterStorage<numParameters>::set(int index, ParameterValueType value)
{
  parameters[index].value.store(value, std::memory_order_relaxed);
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::get(int index) const
{
  return parameters[index].value.load(std::memory_order_relaxed);
}

template<int numParameters>
void TParameterStorage<numParameters>::setNormalized(int index, ParameterValueType value)
{
  set(index, parameters[index].convert.fromNormalized(value));
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::getNormalized(int index) const
{
  return parameters[index].convert.toNormalized(get(index));
}

template<int numParameters>
void TParameterStorage<numParameters>::initialize(const std::vector<ParameterDescription>& parameterDescriptions)
{
  initializeConversions(parameterDescriptions);
  initializeDefaultValues(parameterDescriptions);
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
    parameters[i].value.store(parameterDescriptions[i].defaultValue);
  }
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
  return (x - offset) * range;
}

template<int numParameters>
ParameterValueType TParameterStorage<numParameters>::ParameterNormalization::fromNormalized(ParameterValueType x) const
{
  return x * range + offset;
}

} // namespace unplug