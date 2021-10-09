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
#include <utility>

namespace unplug::vst3 {

double ParameterAccess::getValue(ParamIndex index)
{
  auto const valueNormalized = getValueNormalized(index);
  auto const valuePlain = controller.normalizedParamToPlain(index, valueNormalized);
  return valuePlain;
}

ParameterAccess::ParameterAccess(EditControllerEx1& controller, unplug::detail::MidiMapping& midiMapping)
  : controller(controller)
  , midiMapping(midiMapping)
{
  controller.addRef();
}

ParameterAccess::~ParameterAccess()
{
  controller.release();
}

double ParameterAccess::normalizeValue(ParamIndex index, double value)
{
  return controller.plainParamToNormalized(index, value);
}

double ParameterAccess::getValueNormalized(ParamIndex index)
{
  return controller.getParamNormalized(index);
}

bool ParameterAccess::getDefaultValue(ParamIndex index, double& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = controller.normalizedParamToPlain(index, info.defaultNormalizedValue);
    return true;
  }
  else {
    return false;
  }
}

double ParameterAccess::getDefaultValue(ParamIndex index)
{
  double value = 0;
  bool const ok = getDefaultValue(index, value);
  assert(ok);
  return value;
}

double ParameterAccess::valueFromNormalized(ParamIndex index, double value)
{
  return controller.normalizedParamToPlain(index, value);
}

bool ParameterAccess::getDefaultValueNormalized(ParamIndex index, double& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.defaultNormalizedValue;
    return true;
  }
  else {
    return false;
  }
}

double ParameterAccess::getDefaultValueNormalized(ParamIndex index)
{
  double value = 0;
  bool const ok = getDefaultValueNormalized(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::getMinValue(ParamIndex index, double& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = controller.normalizedParamToPlain(index, 0);
    return true;
  }
  else {
    return false;
  }
}

double ParameterAccess::getMinValue(ParamIndex index)
{
  double value = 0;
  bool const ok = getMinValue(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::getMaxValue(ParamIndex index, double& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = controller.normalizedParamToPlain(index, 1);
    return true;
  }
  else {
    return false;
  }
}

double ParameterAccess::getMaxValue(ParamIndex index)
{
  double value = 1;
  bool const ok = getMaxValue(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::setValue(ParamIndex index, double value)
{
  value = normalizeValue(index, value);
  return setValueNormalized(index, value);
}

bool ParameterAccess::setValueNormalized(ParamIndex index, double value)
{
  bool const isBeingEdited = editRegister.isParameterBeingEdited(index);
  if (!isBeingEdited) {
    assert(false);
    return false;
  }
  bool const setOk = controller.setParamNormalized(index, value) == kResultTrue;
  if (setOk) {
    return controller.performEdit(index, value) == kResultTrue;
  }
  else {
    return false;
  }
}

bool ParameterAccess::beginEdit(ParamIndex index, std::string control)
{
  bool const isBeingEdited = editRegister.isParameterBeingEdited(index);
  if (isBeingEdited) {
    assert(false);
    return false;
  }
  if (controller.beginEdit(index) == kResultTrue) {
    auto const value = getValue(index);
    editRegister.registerEdit(index, std::move(control));
    return true;
  }
  else {
    return false;
  }
}

bool ParameterAccess::endEdit(ParamIndex index)
{
  bool const isBeingEdited = editRegister.isParameterBeingEdited(index);
  if (!isBeingEdited) {
    assert(false);
    return false;
  }
  editRegister.unregisterEdit(index);
  return controller.endEdit(index) == kResultTrue;
}

bool ParameterAccess::convertToText(ParamIndex index, double valueNormalized, std::string& result)
{
  String128 text;
  if (controller.getParamStringByValue(index, valueNormalized, text) == kResultTrue) {
    result = ToUtf8{}(text);
    return true;
  }
  else {
    return false;
  }
}

std::string ParameterAccess::convertToText(ParamIndex index, double valueNormalized)
{
  std::string text;
  bool const ok = convertToText(index, valueNormalized, text);
  assert(ok);
  return text;
}

std::string ParameterAccess::getValueAsText(ParamIndex index)
{
  auto const valueNormalized = getValueNormalized(index);
  return convertToText(index, valueNormalized);
}

bool ParameterAccess::convertFromText(ParamIndex index, double& value, const std::string& text)
{
  auto text16 = ToVstTChar{}(text);
  return controller.getParamValueByString(index, const_cast<TChar*>(text16.c_str()), value) == kResultTrue;
}

double ParameterAccess::convertFromText(ParamIndex index, const std::string& text)
{
  double value = 0;
  bool const ok = convertFromText(index, value, text);
  assert(ok);
  return value;
}

void ParameterAccess::setFromText(ParamIndex index, const std::string& text)
{
  auto const valueNormalized = convertFromText(index, text);
  setValueNormalized(index, valueNormalized);
}

bool ParameterAccess::getName(ParamIndex index, std::string& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = ToUtf8{}(info.title);
    return true;
  }
  else {
    return false;
  }
}

std::string ParameterAccess::getName(ParamIndex index)
{
  std::string text;
  bool const ok = getName(index, text);
  assert(ok);
  return text;
}

bool ParameterAccess::getMeasureUnit(ParamIndex index, std::string& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = ToUtf8{}(info.units);
    return true;
  }
  else {
    return false;
  }
}

std::string ParameterAccess::getMeasureUnit(ParamIndex index)
{
  std::string text;
  bool const ok = getMeasureUnit(index, text);
  assert(ok);
  return text;
}

bool ParameterAccess::getNumSteps(ParamIndex index, int& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.stepCount;
    return true;
  }
  else {
    return false;
  }
}

int ParameterAccess::getNumSteps(ParamIndex index)
{
  int value = 0;
  bool const ok = getNumSteps(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::canBeAutomated(ParamIndex index, bool& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.flags & ParameterInfo::kCanAutomate;
    return true;
  }
  else {
    return false;
  }
}

bool ParameterAccess::canBeAutomated(ParamIndex index)
{
  bool value = false;
  bool const ok = canBeAutomated(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::isList(ParamIndex index, bool& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.flags & ParameterInfo::kIsList;
    return true;
  }
  else {
    return false;
  }
}

bool ParameterAccess::isList(ParamIndex index)
{
  bool value = false;
  bool const ok = isList(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::isProgramChange(ParamIndex index, bool& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.flags & ParameterInfo::kIsProgramChange;
    return true;
  }
  else {
    return false;
  }
}

bool ParameterAccess::isProgramChange(ParamIndex index)
{
  bool value = false;
  bool const ok = isProgramChange(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::isBypass(ParamIndex index, bool& result)
{
  ParameterInfo info;
  if (controller.getParameterInfoByTag(index, info) == kResultTrue) {
    result = info.flags & ParameterInfo::kIsBypass;
    return true;
  }
  else {
    return false;
  }
}

bool ParameterAccess::isBypass(ParamIndex index)
{
  bool value = false;
  bool const ok = isBypass(index, value);
  assert(ok);
  return value;
}

bool ParameterAccess::isBeingEdited(ParamIndex index) const
{
  return editRegister.isParameterBeingEdited(index);
}

std::string ParameterAccess::getEditingControl(ParamIndex index) const
{
  return editRegister.getControllerEditingParameter(index);
}

void ParameterAccess::setMidiMapping(ParamIndex index, int midiControl, int channel)
{
  midiMapping.mapParameter(index, midiControl, channel);
}

void ParameterAccess::setMidiMapping(ParamIndex index, int midiControl)
{
  midiMapping.mapParameter(index, midiControl);
}

bool ParameterAccess::findParameterFromUserInterfaceCoordinates(int xPos, int yPos, ParamIndex& index)
{
  return parameterFinder.findParameterFromUserInterfaceCoordinates(xPos, yPos, index);
}

void ParameterAccess::addParameterRectangle(ParamIndex index, int left, int top, int right, int bottom)
{
  parameterFinder.addParameterRectangle(index, left, top, right, bottom);
}

void ParameterAccess::clearParameterRectangles()
{
  parameterFinder.clear();
}

namespace {
thread_local ParameterAccess* currentParameterAccess = nullptr;
}

ParameterAccess& getParameters()
{
  return *currentParameterAccess;
}

namespace detail {
void setParameters(ParameterAccess* parameterAccess)
{
  currentParameterAccess = parameterAccess;
}
} // namespace detail

} // namespace unplug::vst3