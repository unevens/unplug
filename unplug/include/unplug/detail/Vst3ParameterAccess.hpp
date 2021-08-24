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
#include <cassert>
#include <unordered_set>

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
  ParameterAccess(EditControllerEx1& controller, MidiMapping& midiMapping)
    : controller(controller)
    , midiMapping(midiMapping)
  {
    controller.addRef();
  }

  ~ParameterAccess() { controller.release(); }

  double getValue(int tag)
  {
    auto const valueNormalized = getValueNormalized(tag);
    auto const valuePlain = controller.normalizedParamToPlain(tag, valueNormalized);
    return valuePlain;
  }

  double normalizeValue(int tag, double value) { return controller.normalizedParamToPlain(tag, value); }

  double valueFromNormalized(int tag, double value) { return controller.plainParamToNormalized(tag, value); }

  double getValueNormalized(int tag) { return controller.getParamNormalized(tag); }

  bool getDefaultValue(int tag, double& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = controller.normalizedParamToPlain(tag, info.defaultNormalizedValue);
      return true;
    }
    else {
      return false;
    }
  }

  double getDefaultValue(int tag)
  {
    double value = 0;
    bool const ok = getDefaultValue(tag, value);
    assert(ok);
    return value;
  }

  bool getDefaultValueNormalized(int tag, double& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.defaultNormalizedValue;
      return true;
    }
    else {
      return false;
    }
  }

  double getDefaultValueNormalized(int tag)
  {
    double value = 0;
    bool const ok = getDefaultValueNormalized(tag, value);
    assert(ok);
    return value;
  }

  bool getMinValue(int tag, double& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = controller.normalizedParamToPlain(tag, 0);
      return true;
    }
    else {
      return false;
    }
  }

  double getMinValue(int tag)
  {
    double value = 0;
    bool const ok = getMinValue(tag, value);
    assert(ok);
    return value;
  }

  bool getMaxValue(int tag, double& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = controller.normalizedParamToPlain(tag, 1);
      return true;
    }
    else {
      return false;
    }
  }

  double getMaxValue(int tag)
  {
    double value = 1;
    bool const ok = getMaxValue(tag, value);
    assert(ok);
    return value;
  }

  bool setValue(int tag, double value)
  {
    value = valueFromNormalized(tag, value);
    return setValueNormalized(tag, value);
  }

  bool setValueNormalized(int tag, double value)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    bool const setOk = controller.setParamNormalized(tag, value) == kResultTrue;
    if (setOk) {
      return controller.performEdit(tag, value) == kResultTrue;
    }
    else {
      return false;
    }
  }

  bool beginEdit(int tag)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter != paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    if (controller.beginEdit(tag) == kResultTrue) {
      auto const value = getValue(tag);
      paramsBeingEdited.insert(tag);
      return true;
    }
    else {
      return false;
    }
  }

  bool endEdit(int tag)
  {
    auto editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    paramsBeingEdited.erase(tag);
    return controller.endEdit(tag) == kResultTrue;
  }

  bool convertToText(int tag, double valueNormalized, std::string& result)
  {
    String128 text;
    if (controller.getParamStringByValue(tag, valueNormalized, text) == kResultTrue) {
      result = ToUtf8{}(text);
      return true;
    }
    else {
      return false;
    }
  }

  std::string convertToText(int tag, double valueNormalized)
  {
    std::string text;
    bool const ok = convertToText(tag, valueNormalized, text);
    assert(ok);
    return text;
  }

  std::string getValueAsText(int tag)
  {
    auto const valueNormalized = getValueNormalized(tag);
    return convertToText(tag, valueNormalized);
  }

  bool convertFromText(int tag, double& value, std::string const& text)
  {
    auto text16 = ToVstTChar{}(text);
    return controller.getParamValueByString(tag, const_cast<TChar*>(text16.c_str()), value) == kResultTrue;
  }

  double convertFromText(int tag, std::string const& text)
  {
    double value = 0;
    bool const ok = convertFromText(tag, value, text);
    assert(ok);
    return value;
  }

  void setFromText(int tag, std::string const& text)
  {
    auto const valueNormalized = convertFromText(tag, text);
    setValueNormalized(tag, valueNormalized);
  }

  bool getName(int tag, std::string& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = ToUtf8{}(info.title);
      return true;
    }
    else {
      return false;
    }
  }

  std::string getName(int tag)
  {
    std::string text;
    bool const ok = getName(tag, text);
    assert(ok);
    return text;
  }

  bool getMeasureUnit(int tag, std::string& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = ToUtf8{}(info.units);
      return true;
    }
    else {
      return false;
    }
  }

  std::string getMeasureUnit(int tag)
  {
    std::string text;
    bool const ok = getMeasureUnit(tag, text);
    assert(ok);
    return text;
  }

  bool getNumSteps(int tag, int& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.stepCount;
      return true;
    }
    else {
      return false;
    }
  }

  int getNumSteps(int tag)
  {
    int value = 0;
    bool const ok = getNumSteps(tag, value);
    assert(ok);
    return value;
  }

  bool canBeAutomated(int tag, bool& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.flags & ParameterInfo::kCanAutomate;
      return true;
    }
    else {
      return false;
    }
  }

  int canBeAutomated(int tag)
  {
    bool value = false;
    bool const ok = canBeAutomated(tag, value);
    assert(ok);
    return value;
  }

  bool isList(int tag, bool& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.flags & ParameterInfo::kIsList;
      return true;
    }
    else {
      return false;
    }
  }

  int isList(int tag)
  {
    bool value = false;
    bool const ok = isList(tag, value);
    assert(ok);
    return value;
  }

  bool isProgramChange(int tag, bool& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.flags & ParameterInfo::kIsProgramChange;
      return true;
    }
    else {
      return false;
    }
  }

  int isProgramChange(int tag)
  {
    bool value = false;
    bool const ok = isProgramChange(tag, value);
    assert(ok);
    return value;
  }

  bool isBypass(int tag, bool& result)
  {
    ParameterInfo info;
    if (controller.getParameterInfoByTag(tag, info) == kResultTrue) {
      result = info.flags & ParameterInfo::kIsBypass;
      return true;
    }
    else {
      return false;
    }
  }

  int isBypass(int tag)
  {
    bool value = false;
    bool const ok = isBypass(tag, value);
    assert(ok);
    return value;
  }

  bool isBeingEdited(int tag) const
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    return editedIter != paramsBeingEdited.cend();
  }

  void setMidiMapping(int parameterTag, MidiCC midiControl, int channel)
  {
    midiMapping.mapParameter(parameterTag, midiControl, channel);
  }

  void setMidiMapping(int parameterTag, MidiCC midiControl) { midiMapping.mapParameter(parameterTag, midiControl); }

private:
  EditControllerEx1& controller;
  MidiMapping& midiMapping;
  std::unordered_set<int> paramsBeingEdited;
};

} // namespace unplug::vst3