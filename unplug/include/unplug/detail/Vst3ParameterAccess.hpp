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
#include "unplug/StringConversion.hpp"
#include "public.sdk/source/vst/vsteditcontroller.h"
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
  explicit ParameterAccess(EditControllerEx1& controller)
    : controller(controller)
  {
    controller.addRef();
  }

  ~ParameterAccess() { controller.release(); }

  double getValue(int tag)
  {
    auto value = controller.getParamNormalized(tag);
    value = controller.normalizedParamToPlain(tag, value);
    return value;
  }

  double normalizeValue(int tag, double value)
  {
    return controller.normalizedParamToPlain(tag, value);
  }

  double valueFromNormalized(int tag, double value)
  {
    return controller.plainParamToNormalized(tag, value);
  }

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

  bool setValue(int tag, double value)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    value = controller.plainParamToNormalized(tag, value);
    return controller.performEdit(tag, value) == kResultTrue;
  }

  bool setValueNormalized(int tag, double value)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    return controller.performEdit(tag, value) == kResultTrue;
  }

  bool beginEdit(int tag)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter != paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    if (controller.beginEdit(tag) == kResultTrue) {
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
    if (controller.endEdit(tag) == kResultTrue) {
      paramsBeingEdited.erase(tag);
      return true;
    }
    else {
      return false;
    }
  }

  bool convertToText(int tag, double value, std::string& result)
  {
    String128 text;
    if (controller.getParamStringByValue(tag, value, text) == kResultTrue) {
      result = ToUtf8{}(text);
      return true;
    }
    else {
      return false;
    }
  }

  bool convertFromText(int tag, double& value, std::string const& text)
  {
    auto text16 = ToVstTChar{}(text);
    return controller.getParamValueByString(tag, const_cast<TChar*>(text16.c_str()), value) == kResultTrue;
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

  bool isBeingEdited(int tag) const {
    auto const editedIter = paramsBeingEdited.find(tag);
    return editedIter != paramsBeingEdited.cend();
  }

private:
  EditControllerEx1& controller;
  std::unordered_set<int> paramsBeingEdited;
};

} // namespace unplug::vst3