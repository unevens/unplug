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
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "unplug/MidiMapping.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/detail/EditRegister.hpp"
#include "unplug/detail/ParameterFromUserInterfaceCoordinates.hpp"

namespace unplug::vst3 {

using EditControllerEx1 = Steinberg::Vst::EditControllerEx1;
using ParamValue = Steinberg::Vst::ParamValue;
using ParameterInfo = Steinberg::Vst::ParameterInfo;
using TChar = Steinberg::Vst::TChar;
using String128 = Steinberg::Vst::String128;
using tresult = Steinberg::tresult;
static constexpr auto kResultTrue = Steinberg::kResultTrue;
static constexpr auto kResultFalse = Steinberg::kResultFalse;

/**
 * The ParameterAccess class exposes the plugin parameters to the user interface.
 * */
class ParameterAccess final
{

public:
  ParameterAccess(EditControllerEx1& controller, MidiMapping& midiMapping);

  ~ParameterAccess();

  double getValue(ParamIndex index);

  double normalizeValue(ParamIndex index, double value);

  double valueFromNormalized(ParamIndex index, double value);

  double getValueNormalized(ParamIndex index);

  double getDefaultValue(ParamIndex index);

  double getDefaultValueNormalized(ParamIndex index);

  double getMinValue(ParamIndex index);

  double getMaxValue(ParamIndex index);

  bool setValue(ParamIndex index, double value);

  bool setValueNormalized(ParamIndex index, double value);

  bool beginEdit(ParamIndex index, std::string control);

  bool endEdit(ParamIndex index);

  bool isBeingEdited(ParamIndex index) const;

  std::string getEditingControl(ParamIndex index) const;

  std::string convertToText(ParamIndex index, double valueNormalized);

  std::string getValueAsText(ParamIndex index);

  std::string getName(ParamIndex index);

  std::string getMeasureUnit(ParamIndex index);

  int getNumSteps(ParamIndex index);

  int canBeAutomated(ParamIndex index);

  int isList(ParamIndex index);

  int isProgramChange(ParamIndex index);

  bool isBypass(ParamIndex index, bool& result);

  int isBypass(ParamIndex index);

  void setMidiMapping(ParamIndex index, int midiControl, int channel);

  void setMidiMapping(ParamIndex index, int midiControl);

  void addParameterRectangle(ParamIndex index, int left, int top, int right, int bottom);

  void clearParameterRectangles();

  bool isProgramChange(ParamIndex index, bool& result);

  bool findParameterFromUserInterfaceCoordinates(int xPos, int yPos, ParamIndex& parameterTag);

#ifndef UNPLUG_EXPOSE_VST3STYLE_PARAMETER_API
private:
#endif

  bool getDefaultValue(ParamIndex index, double& result);
  bool getDefaultValueNormalized(ParamIndex index, double& result);
  bool getMinValue(ParamIndex index, double& result);
  bool getMaxValue(ParamIndex index, double& result);
  bool convertToText(ParamIndex index, double valueNormalized, std::string& result);
  bool convertFromText(ParamIndex index, double& value, std::string const& text);
  bool getMeasureUnit(ParamIndex index, std::string& result);
  bool canBeAutomated(ParamIndex index, bool& result);
  bool isList(ParamIndex index, bool& result);
  double convertFromText(ParamIndex index, std::string const& text);
  void setFromText(ParamIndex index, std::string const& text);
  bool getName(ParamIndex index, std::string& result);
  bool getNumSteps(ParamIndex index, int& result);

private:
  EditControllerEx1& controller;
  MidiMapping& midiMapping;
  unplug::detail::ParameterEditRegister editRegister;
  unplug::detail::ParameterFromUserInterfaceCoordinates parameterFinder;
  inline static thread_local ParameterAccess* current = nullptr;
};

ParameterAccess& getParameters();

namespace detail {
void setParameters(ParameterAccess* parameterAccess);
} // namespace detail

} // namespace unplug::vst3