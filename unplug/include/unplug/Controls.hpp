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
#include "unplug/ParameterAccess.hpp"
#include "imgui_user_config.h"

namespace unplug {


/**
 * Combo ImGui control associated with a plugin parameter
 * */

bool
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

bool
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


struct KnobOutput{
  bool isActive;
  double outputValue;
};

/**
 * Knob ImGui control, based on https://github.com/ocornut/imgui/issues/942
 * */
KnobOutput
Knob(const char* name,
     const char* valueLabel,
     const double inputValue,
     const float radius,
     const double angleOffset = M_PI / 4)
{
  ImGuiStyle& style = ImGui::GetStyle();
  auto const lineHeight = ImGui::GetTextLineHeight();
  ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
  ImVec2 center = ImVec2(cursorPosition.x + radius, cursorPosition.y + radius);

  ImVec2 textpos = cursorPosition;
  auto const currentAngle = (M_PI - angleOffset) * inputValue * 2.0f + angleOffset;

  auto const x = -std::sin(currentAngle) * radius + center.x;
  auto const y = std::cos(currentAngle) * radius + center.y;

  auto const diameter = 2 * radius;
  ImGui::InvisibleButton(name, ImVec2(diameter, diameter + lineHeight + style.ItemInnerSpacing.y));

  bool const isActive = ImGui::IsItemActive();
  bool const isHovered = ImGui::IsItemHovered();
  auto outputValue = inputValue;

  if (isActive) {
    ImVec2 mp = ImGui::GetIO().MousePos;
    double nextAngle = std::atan2(mp.x - center.x, center.y - mp.y) + M_PI;
    nextAngle = std::max(angleOffset, std::min(2.0f * M_PI - angleOffset, nextAngle));
    outputValue = 0.5f * (nextAngle - angleOffset) / (M_PI - angleOffset);
  }

  ImU32 col32 = ImGui::GetColorU32(isActive    ? ImGuiCol_FrameBgActive
                                               : isHovered ? ImGuiCol_FrameBgHovered
                                                           : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImU32 col32text = ImGui::GetColorU32(ImGuiCol_Text);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddCircleFilled(center, radius, col32, 16);
  draw_list->AddLine(center, ImVec2(x, y), col32line, 1);
  draw_list->AddText(textpos, col32text, valueLabel);
  draw_list->AddText(ImVec2(cursorPosition.x, cursorPosition.y + diameter + style.ItemInnerSpacing.y), col32text, name);

  return { isActive, outputValue };
}


/**
 * Knob ImGui control associated with a plugin parameter
 * */

bool
Knob(ParameterAccess& parameters, int parameterTag, float radius, const double angleOffset = M_PI / 4)
{
  bool const isBeingEdited = parameters.isBeingEdited(parameterTag);
  double const normalizedValue = parameters.getValueNormalized(parameterTag);
  double const value = parameters.valueFromNormalized(parameterTag, normalizedValue);
  std::string parameterName;
  bool const gotNameOk = parameters.getName(parameterTag, parameterName);
  assert(gotNameOk);
  std::string valueAsText;
  bool const convertedOk = parameters.convertToText(parameterTag, value, valueAsText);
  assert(convertedOk);

  auto const output = Knob(parameterName.c_str(), valueAsText.c_str(), normalizedValue, radius, angleOffset);

  if (isBeingEdited) {
    if (output.isActive) {
      parameters.setValueNormalized(parameterTag, output.outputValue);
    }
    else {
      parameters.endEdit(parameterTag);
    }
  }
  else {
    if (output.isActive) {
      parameters.beginEdit(parameterTag);
      parameters.setValueNormalized(parameterTag, output.outputValue);
    }
  }

  return output.isActive;
}


} // namespace unplug
