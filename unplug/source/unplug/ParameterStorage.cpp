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

#include "unplug/ParameterStorage.hpp"

#include <utility>

namespace unplug {

unplug::ParameterDescription::ParameterDescription(int tag,
                                                   std::string name_,
                                                   std::vector<std::string> labels_,
                                                   int defaultValue)
  : type{ Type::list }
  , tag{ tag }
  , labels{ std::move(labels_) }
  , name{ std::move(name_) }
  , defaultValue{ static_cast<ParameterValueType>(defaultValue) }
{
  numSteps = static_cast<int>(labels.size()) - 1;
  max = static_cast<ParameterValueType>(numSteps);
}

ParameterDescription::ParameterDescription(int tag,
                                           std::string name_,
                                           ParameterValueType min,
                                           ParameterValueType max,
                                           ParameterValueType defaultValue,
                                           int numSteps)
  : type{ Type::numeric }
  , tag{ tag }
  , min{ min }
  , max{ max }
  , defaultValue{ defaultValue }
  , numSteps{ numSteps }
  , name{ std::move(name_) }
{}

ParameterDescription ParameterDescription::Automatable(bool isAutomatable)
{
  canBeAutomated = isAutomatable;
  return *this;
}

ParameterDescription ParameterDescription::ShortName(std::string shortName_)
{
  shortName = std::move(shortName_);
  return *this;
}

ParameterDescription ParameterDescription::MeasureUnit(std::string measureUnit_)
{
  measureUnit = std::move(measureUnit_);
  return *this;
}

ParameterDescription ParameterDescription::MidiMapping(int control)
{
  assert(control > -1 && control < 130);
  return MidiMapping(control, -1);
}

ParameterDescription ParameterDescription::MidiMapping(int control, int channel)
{
  assert(control > -1 && control < 130);
  assert(channel > -1 && channel < 17);
  defaultMidiMapping.control = control;
  defaultMidiMapping.channel = channel;
  return *this;
}

ParameterDescription ParameterDescription::ControlledByDecibels(bool mapMinToLinearZero)
{
  auto dBToLinear = [minInDB = min, mapMinToLinearZero](double db) {
    if (mapMinToLinearZero && db <= minInDB)
      return 0.0;
    else
      return unplug::dBToLinear(db);
  };
  auto linearToDB = [minInDB = min, minInLinear = unplug::dBToLinear(min), mapMinToLinearZero](double linear) {
    if (mapMinToLinearZero && linear <= minInLinear)
      return minInDB;
    else
      return unplug::linearToDB(linear);
  };
  return Nonlinear(linearToDB, dBToLinear);
}

ParameterDescription ParameterDescription::makeBypassParameter(int tag)
{
  auto parameter = ParameterDescription(tag, "Bypass", 0, 1, 0, 1);
  parameter.isBypass = true;
  return parameter;
}

ParameterDescription ParameterDescription::Nonlinear(std::function<double(double)> linearToNonlinear_,
                                                     std::function<double(double)> nonlinearToLinear_)
{
  linearToNonlinear = std::move(linearToNonlinear_);
  nonlinearToLinear = std::move(nonlinearToLinear_);
  return *this;
}

bool ParameterDescription::isNonlinear() const
{
  return linearToNonlinear != nullptr && nonlinearToLinear != nullptr;
}

ParameterInitializer ParameterCreator::done()
{
  sortParametersByTag();
  return ParameterInitializer{ descriptions };
}

void ParameterCreator::sortParametersByTag()
{
  std::sort(descriptions.begin(), descriptions.end(), [](auto& lhs, auto& rhs) { return lhs.tag < rhs.tag; });
}

ParameterInitializer::ParameterInitializer(std::vector<ParameterDescription> descriptions)
  : descriptions(std::move(descriptions))
{}

} // namespace unplug