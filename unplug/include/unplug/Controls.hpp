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
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_user_config.h"
#include "unplug/ParameterAccess.hpp"

namespace unplug {

inline constexpr auto pi = (float)M_PI;

/**
 * Combo ImGui control associated with a plugin parameter
 * */
inline bool
Combo(ParameterAccess& parameters, int parameterTag)
{
  using namespace ImGui;

  bool isList = false;
  parameters.isList(parameterTag, isList);
  assert(isList);

  double const value = parameters.getValue(parameterTag);
  std::string parameterName;
  bool const gotNameOk = parameters.getName(parameterTag, parameterName);
  assert(gotNameOk);

  std::string valueAsText;
  bool const convertedOk = parameters.convertToText(parameterTag, value, valueAsText);
  assert(convertedOk);

  int numSteps = 0;
  bool const gotNumStepsOk = parameters.getNumSteps(parameterTag, numSteps);
  assert(gotNumStepsOk);

  if (!BeginCombo(parameterName.c_str(), valueAsText.c_str(), ImGuiComboFlags_None))
    return false;

  auto const numItems = numSteps + 1;

  bool hasValueChanged = false;
  auto newValue = value;
  for (int i = 0; i < numItems; i++) {
    PushID((void*)(intptr_t)i);
    const bool selectedItem = (i == newValue);
    std::string itemText;
    parameters.convertToText(parameterTag, (double)i, itemText);
    if (Selectable(itemText.c_str(), selectedItem)) {
      hasValueChanged = true;
      newValue = i;
    }
    if (selectedItem)
      SetItemDefaultFocus();
    PopID();
  }

  EndCombo();
  if (hasValueChanged) {
    parameters.beginEdit(parameterTag);
    parameters.setValueNormalized(parameterTag, newValue);
    parameters.endEdit(parameterTag);
    auto& g = *GImGui;
    MarkItemEdited(g.CurrentWindow->DC.LastItemId);
  }

  return hasValueChanged;
}

/**
 * Checkbox ImGui control associated with a plugin parameter
 * */
inline bool
Checkbox(ParameterAccess& parameters, int parameterTag)
{
  using namespace ImGui;

  double const value = parameters.getValue(parameterTag);
  std::string parameterName;
  bool const gotNameOk = parameters.getName(parameterTag, parameterName);
  assert(gotNameOk);

  bool newValue = value != 0;
  bool const hasValueChanged = ImGui::Checkbox(parameterName.c_str(), &newValue);
  if (hasValueChanged) {
    parameters.beginEdit(parameterTag);
    parameters.setValueNormalized(parameterTag, newValue);
    parameters.endEdit(parameterTag);
  }

  return hasValueChanged;
}

inline void
Label(ParameterAccess& parameters, int parameterTag)
{
  auto const name = parameters.getName(parameterTag);
  return ImGui::TextUnformatted(name.c_str());
}

inline void
ValueAsText(ParameterAccess& parameters, int parameterTag)
{
  auto const value = parameters.getValue(parameterTag);
  auto const valueAsText = parameters.convertToText(parameterTag, value);
  return ImGui::TextUnformatted(valueAsText.c_str());
}

struct KnobOutput
{
  bool isActive;
  double outputValue;
};

struct KnobDrawData
{
  float radius;
  ImVec2 center;
  ImVec2 pointerPosition;
  float angleOffset;
  bool isActive;
  bool isHovered;
};

template<int numSegments>
inline void
DrawSimpleKnob(KnobDrawData const& knob)
{
  ImU32 col32 = ImGui::GetColorU32(knob.isActive    ? ImGuiCol_FrameBgActive
                                   : knob.isHovered ? ImGuiCol_FrameBgHovered
                                                    : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddCircleFilled(knob.center, knob.radius, col32, numSegments);
  draw_list->AddLine(knob.center, knob.pointerPosition, col32line, 1);
}

/**
 * Knob ImGui control, based on https://github.com/ocornut/imgui/issues/942
 * */
template<class Drawer>
inline KnobOutput
Knob(const char* name,
     const float inputValue,
     const float radius,
     Drawer drawer = DrawSimpleKnob<64>,
     const float angleOffset = pi / 4)
{
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
  ImVec2 center = ImVec2(cursorPosition.x + radius, cursorPosition.y + radius);

  auto const currentAngle = (pi - angleOffset) * inputValue * 2.0f + angleOffset;

  auto const x = -std::sin(currentAngle) * radius + center.x;
  auto const y = std::cos(currentAngle) * radius + center.y;

  auto const diameter = 2 * radius;
  ImGui::InvisibleButton(name, ImVec2(diameter, diameter));

  bool const isActive = ImGui::IsItemActive();
  bool const isHovered = ImGui::IsItemHovered();
  auto outputValue = inputValue;

  if (isActive) {
    ImVec2 mp = ImGui::GetIO().MousePos;
    float nextAngle = std::atan2(mp.x - center.x, center.y - mp.y) + pi;
    nextAngle = std::max(angleOffset, std::min(2.0f * pi - angleOffset, nextAngle));
    outputValue = 0.5f * (nextAngle - angleOffset) / (pi - angleOffset);
    bool const hasGoneBelowTheBottom = inputValue == 0.0 && outputValue > 0.5;
    bool const hasGoneOverTheTop = inputValue == 1.0 && outputValue < 0.5;
    bool const hasGoneOutsideTheRange = hasGoneBelowTheBottom || hasGoneOverTheTop;
    if (hasGoneOutsideTheRange) {
      outputValue = inputValue;
    }
  }

  KnobDrawData drawData;
  drawData.radius = radius;
  drawData.center = center;
  drawData.pointerPosition = ImVec2(x, y);
  drawData.angleOffset = angleOffset;
  drawData.isActive = isActive;
  drawData.isHovered = isHovered;
  drawer(drawData);

  return { isActive, outputValue };
}

/**
 * Knob ImGui control associated with a plugin parameter
 * */
template<class Drawer>
inline bool
Knob(ParameterAccess& parameters, int parameterTag, float radius, Drawer drawer, const float angleOffset = pi / 4)
{
  bool const isParameterBeingEdited = parameters.isBeingEdited(parameterTag);
  double const normalizedValue = parameters.getValueNormalized(parameterTag);
  double const value = parameters.valueFromNormalized(parameterTag, normalizedValue);
  std::string parameterName;
  bool const gotNameOk = parameters.getName(parameterTag, parameterName);
  assert(gotNameOk);
  std::string valueAsText;
  bool const convertedOk = parameters.convertToText(parameterTag, value, valueAsText);
  assert(convertedOk);

  auto const output = Knob(parameterName.c_str(), normalizedValue, radius, drawer, angleOffset);

  if (isParameterBeingEdited) {
    if (output.isActive) {
      bool const setValueOk = parameters.setValueNormalized(parameterTag, output.outputValue);
      assert(setValueOk);
    }
    else {
      bool const endEditOk = parameters.endEdit(parameterTag);
      assert(endEditOk);
    }
  }
  else {
    if (output.isActive) {
      bool const beginEditOk = parameters.beginEdit(parameterTag);
      assert(beginEditOk);
      bool const setValueOk = parameters.setValueNormalized(parameterTag, output.outputValue);
      assert(setValueOk);
    }
  }

  return output.isActive;
}

inline bool
Knob(ParameterAccess& parameters, int parameterTag, float radius, const float angleOffset = pi / 4)
{
  return Knob(parameters, parameterTag, radius, DrawSimpleKnob<64>, angleOffset);
}

} // namespace unplug
