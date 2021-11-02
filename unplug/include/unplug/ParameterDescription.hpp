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
/**
 * A struct that describes a parameter. It is used to create the parameter.
 * */
enum class ParamEditPolicy
{
  automatable,
  notAutomatable,
  notAutomatableAndMayChangeLatencyOnEdit
};

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
  ParamEditPolicy editPolicy = ParamEditPolicy::automatable;
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

  /**
   * Constructor for a list parameter
   * */
  ParameterDescription(ParamIndex index, std::string name_, std::vector<std::string> labels_, int defaultValue = 0);

  /**
   * Constructor for a numeric parameter
   * */
  ParameterDescription(ParamIndex index,
                       std::string name_,
                       ParameterValueType min,
                       ParameterValueType max,
                       ParameterValueType defaultValue = 0,
                       int numSteps = 0);

  /**
   * Makes the parameter automatable
   * */
  ParameterDescription EditPolicy(ParamEditPolicy editPolicy_);

  /**
   * Sets the parameter short name. May be used by the host where there is not much space to show the name of the
   * parameter.
   * */
  ParameterDescription ShortName(std::string shortName_);

  /**
   * Sets the parameter measure unit
   * */
  ParameterDescription MeasureUnit(std::string measureUnit_);

  /**
   * Gives the parameter a midi mapping (listening to all channels)
   * */
  ParameterDescription MidiMapping(int control);

  /**
   * Gives the parameter a midi mapping (listening to a specific channel)
   * */
  ParameterDescription MidiMapping(int control, int channel);

  /**
   * Makes the parameter controlled by decibels (but keeps it linear in the dsp)
   * */
  ParameterDescription ControlledByDecibels(bool mapMinToLinearZero = true);

  /**
   * Makes the parameter controlled with a nonlinear scaling (but keeps it linear in the dsp)
   * */
  ParameterDescription Nonlinear(std::function<double(double)> linearToNonlinear_,
                                 std::function<double(double)> nonlinearToLinear_);

  /**
   * Makes the parameter the bypass parameter of the plugin
   * */
  static ParameterDescription makeBypassParameter(ParamIndex index);

  /**
   * @return true if the parameter has a nonlinear scaling
   * */
  bool isNonlinear() const;

  bool isAutomatable() const;

  bool mayChangeLatencyOnEdit() const;
};

} // namespace unplug