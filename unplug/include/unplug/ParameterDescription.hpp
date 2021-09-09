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
#include "unplug/Index.hpp"
#include "unplug/MidiMapping.hpp"
#include <atomic>
#include <cassert>
#include <functional>
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
  ParamIndex index;
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
    bool listensToAllChannels() const { return channel == -1; }
    bool isEnabled() const { return control > -1; }
  } defaultMidiMapping;

  ParameterDescription(ParamIndex index, std::string name_, std::vector<std::string> labels_, int defaultValue = 0);

  ParameterDescription(ParamIndex index,
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

  static ParameterDescription makeBypassParameter(ParamIndex index);

  bool isNonlinear() const;
};

} // namespace unplug