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

#include "unplug/detail/Vst3ParameterAccess.hpp"
#include <cassert>

namespace unplug::vst3 {

double
ParameterAccess::getValue(int tag)
{
  auto const valueNormalized = getValueNormalized(tag);
  auto const valuePlain = controller.normalizedParamToPlain(tag, valueNormalized);
  return valuePlain;
}

ParameterAccess::ParameterAccess(EditControllerEx1& controller, MidiMapping& midiMapping)
  : controller(controller)
  , midiMapping(midiMapping)
{
  controller.addRef();
}

ParameterAccess::~ParameterAccess()
{
  controller.release();
}

double
ParameterAccess::normalizeValue(int tag, double value)
{
  return controller.plainParamToNormalized(tag, value);
}

double
ParameterAccess::getValueNormalized(int tag)
{
  return controller.getParamNormalized(tag);
}

bool
ParameterAccess::getDefaultValue(int tag, double& result)
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

double
ParameterAccess::getDefaultValue(int tag)
{
  double value = 0;
  bool const ok = getDefaultValue(tag, value);
  assert(ok);
  return value;
}

double
ParameterAccess::valueFromNormalized(int tag, double value)
{
  return controller.normalizedParamToPlain(tag, value);
}

bool
ParameterAccess::getDefaultValueNormalized(int tag, double& result)
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

double
ParameterAccess::getDefaultValueNormalized(int tag)
{
  double value = 0;
  bool const ok = getDefaultValueNormalized(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::getMinValue(int tag, double& result)
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

double
ParameterAccess::getMinValue(int tag)
{
  double value = 0;
  bool const ok = getMinValue(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::getMaxValue(int tag, double& result)
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

double
ParameterAccess::getMaxValue(int tag)
{
  double value = 1;
  bool const ok = getMaxValue(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::setValue(int tag, double value)
{
  value = normalizeValue(tag, value);
  return setValueNormalized(tag, value);
}

bool
ParameterAccess::setValueNormalized(int tag, double value)
{
  auto const editedIter = paramsBeingEditedByControls.find(tag);
  if (editedIter == paramsBeingEditedByControls.end()) {
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

bool
ParameterAccess::beginEdit(int tag, std::string control)
{
  auto const editedIter = paramsBeingEditedByControls.find(tag);
  if (editedIter != paramsBeingEditedByControls.end()) {
    assert(false);
    return false;
  }
  if (controller.beginEdit(tag) == kResultTrue) {
    auto const value = getValue(tag);
    paramsBeingEditedByControls.emplace(tag, control);
    return true;
  }
  else {
    return false;
  }
}

bool
ParameterAccess::endEdit(int tag, std::string control)
{
  auto editedIter = paramsBeingEditedByControls.find(tag);
  if (editedIter == paramsBeingEditedByControls.end()) {
    assert(false);
    return false;
  }
  paramsBeingEditedByControls.erase(tag);
  return controller.endEdit(tag) == kResultTrue;
}

bool
ParameterAccess::convertToText(int tag, double valueNormalized, std::string& result)
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

std::string
ParameterAccess::convertToText(int tag, double valueNormalized)
{
  std::string text;
  bool const ok = convertToText(tag, valueNormalized, text);
  assert(ok);
  return text;
}

std::string
ParameterAccess::getValueAsText(int tag)
{
  auto const valueNormalized = getValueNormalized(tag);
  return convertToText(tag, valueNormalized);
}

bool
ParameterAccess::convertFromText(int tag, double& value, const std::string& text)
{
  auto text16 = ToVstTChar{}(text);
  return controller.getParamValueByString(tag, const_cast<TChar*>(text16.c_str()), value) == kResultTrue;
}

double
ParameterAccess::convertFromText(int tag, const std::string& text)
{
  double value = 0;
  bool const ok = convertFromText(tag, value, text);
  assert(ok);
  return value;
}

void
ParameterAccess::setFromText(int tag, const std::string& text)
{
  auto const valueNormalized = convertFromText(tag, text);
  setValueNormalized(tag, valueNormalized);
}

bool
ParameterAccess::getName(int tag, std::string& result)
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

std::string
ParameterAccess::getName(int tag)
{
  std::string text;
  bool const ok = getName(tag, text);
  assert(ok);
  return text;
}

bool
ParameterAccess::getMeasureUnit(int tag, std::string& result)
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

std::string
ParameterAccess::getMeasureUnit(int tag)
{
  std::string text;
  bool const ok = getMeasureUnit(tag, text);
  assert(ok);
  return text;
}

bool
ParameterAccess::getNumSteps(int tag, int& result)
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

int
ParameterAccess::getNumSteps(int tag)
{
  int value = 0;
  bool const ok = getNumSteps(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::canBeAutomated(int tag, bool& result)
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

int
ParameterAccess::canBeAutomated(int tag)
{
  bool value = false;
  bool const ok = canBeAutomated(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::isList(int tag, bool& result)
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

int
ParameterAccess::isList(int tag)
{
  bool value = false;
  bool const ok = isList(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::isProgramChange(int tag, bool& result)
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

int
ParameterAccess::isProgramChange(int tag)
{
  bool value = false;
  bool const ok = isProgramChange(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::isBypass(int tag, bool& result)
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

int
ParameterAccess::isBypass(int tag)
{
  bool value = false;
  bool const ok = isBypass(tag, value);
  assert(ok);
  return value;
}

bool
ParameterAccess::isBeingEdited(int tag) const
{
  auto const editedIter = paramsBeingEditedByControls.find(tag);
  return editedIter != paramsBeingEditedByControls.cend();
}

std::string
ParameterAccess::getEditingControl(int tag) const
{
  auto const editedIter = paramsBeingEditedByControls.find(tag);
  return (editedIter != paramsBeingEditedByControls.cend()) ? editedIter->second : "";
}

void
ParameterAccess::setMidiMapping(int parameterTag, int midiControl, int channel)
{
  midiMapping.mapParameter(parameterTag, midiControl, channel);
}

void
ParameterAccess::setCurrent(ParameterAccess* parameterAccess)
{
  current = parameterAccess;
}

ParameterAccess*
ParameterAccess::getCurrent()
{
  return current;
}

ParameterAccess&
Parameters()
{
  return *ParameterAccess::getCurrent();
}

} // namespace unplug::vst3