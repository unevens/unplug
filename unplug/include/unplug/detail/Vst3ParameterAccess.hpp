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
#include <unordered_map>

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

  double getValue(int tag);

  double normalizeValue(int tag, double value);

  double valueFromNormalized(int tag, double value);

  double getValueNormalized(int tag);

  bool getDefaultValue(int tag, double& result);

  double getDefaultValue(int tag);

  bool getDefaultValueNormalized(int tag, double& result);

  double getDefaultValueNormalized(int tag);

  bool getMinValue(int tag, double& result);

  double getMinValue(int tag);

  bool getMaxValue(int tag, double& result);

  double getMaxValue(int tag);

  bool setValue(int tag, double value);

  bool setValueNormalized(int tag, double value);

  bool beginEdit(int tag, std::string control);

  bool endEdit(int tag, std::string control);

  bool isBeingEdited(int tag) const;

  std::string getEditingControl(int tag) const;

  bool convertToText(int tag, double valueNormalized, std::string& result);

  std::string convertToText(int tag, double valueNormalized);

  std::string getValueAsText(int tag);

  bool convertFromText(int tag, double& value, std::string const& text);

  double convertFromText(int tag, std::string const& text);

  void setFromText(int tag, std::string const& text);

  bool getName(int tag, std::string& result);

  std::string getName(int tag);

  bool getMeasureUnit(int tag, std::string& result);

  std::string getMeasureUnit(int tag);

  bool getNumSteps(int tag, int& result);

  int getNumSteps(int tag);

  bool canBeAutomated(int tag, bool& result);

  int canBeAutomated(int tag);

  bool isList(int tag, bool& result);

  int isList(int tag);

  bool isProgramChange(int tag, bool& result);

  int isProgramChange(int tag);

  bool isBypass(int tag, bool& result);

  int isBypass(int tag);

  void setMidiMapping(int parameterTag, int midiControl, int channel);

  void setMidiMapping(int parameterTag, int midiControl)
  {
    midiMapping.mapParameter(parameterTag, midiControl);
  }

  static void setCurrent(ParameterAccess* parameterAccess);

  static ParameterAccess* getCurrent();

private:
  EditControllerEx1& controller;
  MidiMapping& midiMapping;
  std::unordered_map<int, std::string> paramsBeingEditedByControls;
  inline static thread_local ParameterAccess* current = nullptr;
};

ParameterAccess& Parameters();

} // namespace unplug::vst3