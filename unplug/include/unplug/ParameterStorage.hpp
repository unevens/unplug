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
#include "unplug/Math.hpp"
#include "unplug/MidiMapping.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <string>
#include <vector>

namespace unplug {

using ParameterValueType = std::conditional<std::atomic<double>::is_always_lock_free, double, float>::type;

static_assert(std::atomic<float>::is_always_lock_free, "At least atomic<float> should be lockfree");

struct ParameterDescription
{
  enum class Type
  {
    numeric,
    list
  };
  Type type;
  int tag;
  std::string name;
  std::string shortName;
  std::string measureUnit;
  bool canBeAutomated = true;
  ParameterValueType min;
  ParameterValueType max;
  ParameterValueType defaultValue;
  int numSteps;
  std::vector<std::string> labels;
  bool isBypass = false;
  std::function<double(double)> linearToNonlinear;
  std::function<double(double)> nonlinearToLinear;
  struct
  {
    int control = -1;
    int channel = -1;
    bool listensToAllChannels() const
    {
      return channel == -1;
    }
    bool isEnabled() const
    {
      return control > -1;
    }
  } defaultMidiMapping;

  ParameterDescription(int tag, std::string name_, std::vector<std::string> labels_, int defaultValue = 0);

  ParameterDescription(int tag,
                       std::string name_,
                       ParameterValueType min,
                       ParameterValueType max,
                       ParameterValueType defaultValue = 0,
                       int numSteps = 0);

  ParameterDescription Automatable(bool isAutomatable);

  ParameterDescription ShortName(std::string shortName_);

  ParameterDescription MeasureUnit(std::string measureUnit_);

  ParameterDescription MidiMapping(int control);

  ParameterDescription MidiMapping(int control, int channel);

  ParameterDescription ControlledByDecibels(bool mapMinToLinearZero = true);

  ParameterDescription Nonlinear(std::function<double(double)> linearToNonlinear_,
                                 std::function<double(double)> nonlinearToLinear_);

  static ParameterDescription makeBypassParameter(int tag);

  bool isNonlinear() const;
};

template<int numParameters>
class ParameterStorage final
{
  friend class ParameterInitializer;

  class ParameterNormalization final
  {
  public:
    explicit ParameterNormalization(ParameterValueType min = 0, ParameterValueType max = 1)
      : range(max - min)
      , offset(min)
    {}
    ParameterValueType toNormalized(ParameterValueType x) const
    {
      return (x - offset) * range;
    }
    ParameterValueType fromNormalized(ParameterValueType x) const
    {
      return x * range + offset;
    }

  private:
    ParameterValueType offset;
    ParameterValueType range;
  };

  struct StoredParameter final{
    std::atomic<ParameterValueType> value{0};
    ParameterNormalization convert;
  };

public:
  void set(int index, ParameterValueType value)
  {
    parameters[index].value.store(value);
  }

  ParameterValueType get(int index) const
  {
    return parameters[index].value.load();
  }

  void setNormalized(int index, ParameterValueType value)
  {
    parameters[index].value.store(parameters[index].convert.fromNormalized(value));
  }

  ParameterValueType getNormalized(int index) const
  {
    return parameters[index].convert.toNormalized(parameters[index].value.load());
  }

private:
  void initialize(std::vector<ParameterDescription> const& parameters)
  {
    initializeConversions(parameters);
    initializeDefaultValues(parameters);
  }

  void initializeConversions(std::vector<ParameterDescription> const& parameterDescriptions)
  {
    int i = 0;
    for (auto& parameter : parameterDescriptions) {
      auto const min = parameter.isNonlinear() ? parameter.nonlinearToLinear(parameter.min) : parameter.min;
      auto const max = parameter.isNonlinear() ? parameter.nonlinearToLinear(parameter.max) : parameter.max;
      parameters[i].convert = ParameterNormalization{ min, max };
      ++i;
    }
  }

  void initializeDefaultValues(std::vector<ParameterDescription> const& parameterDescriptions)
  {
    for (int i = 0; i < parameterDescriptions.size(); ++i) {
      parameters[i].value.store(parameterDescriptions[i].defaultValue);
    }
  }

  std::array<StoredParameter, numParameters> parameters;
};

class ParameterInitializer final
{
  friend class ParameterCreator;

public:
  template<class InitializeParameter>
  void initializeParameters(InitializeParameter initializeParameter)
  {
    for (auto& description : descriptions) {
      initializeParameter(description);
    }
  }

  template<int numParameters>
  void initializeStorage(ParameterStorage<numParameters>& storage)
  {
    assert(numParameters == descriptions.size());
    storage.initialize(descriptions);
  }

  std::vector<ParameterDescription> const& getDescriptions() const
  {
    return descriptions;
  }

private:
  explicit ParameterInitializer(std::vector<ParameterDescription> descriptions);

  std::vector<ParameterDescription> descriptions;
};

class ParameterCreator final
{
public:
  void addParameter(ParameterDescription&& parameterDescription)
  {
    descriptions.emplace_back(parameterDescription);
  }

  ParameterInitializer done();

private:
  void sortParametersByTag();
  std::vector<ParameterDescription> descriptions;
};

} // namespace unplug