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

#include "unplug/Controls.hpp"

namespace unplug {

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

void
Label(ParameterAccess& parameters, int parameterTag)
{
  auto const name = parameters.getName(parameterTag);
  return ImGui::TextUnformatted(name.c_str());
}

void
TextCentered(std::string const& text, ImVec2 size)
{
  auto const bkgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
  ImGui::PushStyleColor(ImGuiCol_Button, bkgColor);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bkgColor);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, bkgColor);
  ImGui::Button(text.c_str(), size);
  ImGui::PopStyleColor(3);
}

void
LabelCentered(ParameterAccess& parameters, int parameterTag, ImVec2 size)
{
  auto const name = parameters.getName(parameterTag);
  TextCentered(name, size);
}

void
ValueAsText(ParameterAccess& parameters, int parameterTag)
{
  auto const valueAsText = parameters.getValueAsText(parameterTag);
  return ImGui::TextUnformatted(valueAsText.c_str());
}

void
ValueAsTextCentered(ParameterAccess& parameters, int parameterTag, ImVec2 size)
{
  auto const valueAsText = parameters.getValueAsText(parameterTag);
  TextCentered(valueAsText, size);
}

detail::KnobOutput
detail::Knob(const char* name, float const inputValue, KnobLayout layout)
{
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
  ImVec2 center = ImVec2(cursorPosition.x + layout.radius, cursorPosition.y + layout.radius);

  auto const currentAngle = (pi - layout.angleOffset) * inputValue * 2.0f + layout.angleOffset;

  auto const x = -std::sin(currentAngle) * layout.radius + center.x;
  auto const y = std::cos(currentAngle) * layout.radius + center.y;

  auto const diameter = 2 * layout.radius;
  ImGui::InvisibleButton(name, ImVec2(diameter, diameter));

  bool const isActive = ImGui::IsItemActive();
  bool const isHovered = ImGui::IsItemHovered();
  auto outputValue = inputValue;

  if (isActive) {
    ImVec2 mp = ImGui::GetIO().MousePos;
    float nextAngle = std::atan2(mp.x - center.x, center.y - mp.y) + pi;
    nextAngle = std::max(layout.angleOffset, std::min(2.0f * pi - layout.angleOffset, nextAngle));
    outputValue = 0.5f * (nextAngle - layout.angleOffset) / (pi - layout.angleOffset);
    bool const hasGoneBelowTheBottom = inputValue == 0.0 && outputValue > 0.5;
    bool const hasGoneOverTheTop = inputValue == 1.0 && outputValue < 0.5;
    bool const hasGoneOutsideTheRange = hasGoneBelowTheBottom || hasGoneOverTheTop;
    if (hasGoneOutsideTheRange) {
      outputValue = inputValue;
    }
  }

  KnobDrawData drawData;
  drawData.layout = layout;
  drawData.center = center;
  drawData.pointerPosition = ImVec2(x, y);
  drawData.isActive = isActive;
  drawData.isHovered = isHovered;

  return { drawData, outputValue, isActive };
}

void
DrawSimpleKnob(KnobDrawData const& knob)
{
  ImU32 col32 = ImGui::GetColorU32(knob.isActive    ? ImGuiCol_FrameBgActive
    : knob.isHovered ? ImGuiCol_FrameBgHovered
    : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  auto const numSegments = static_cast<int>(1.5f*knob.layout.radius);
  draw_list->AddCircleFilled(knob.center, knob.layout.radius, col32, numSegments);
  draw_list->AddLine(knob.center, knob.pointerPosition, col32line, 1);
}

bool
Knob(ParameterAccess& parameters, int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> drawer)
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

  auto const output = detail::Knob(parameterName.c_str(), static_cast<float>(normalizedValue), layout);

  drawer(output.drawData);

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


bool
KnobWithLabels(ParameterAccess& parameters, int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> drawer)
{
  auto const size = ImVec2{ layout.radius * 2, 2 * ImGui::GetTextLineHeight() };
  LabelCentered(parameters, parameterTag, size);
  auto const isActive = Knob(parameters, parameterTag, layout, std::move(drawer));
  ValueAsTextCentered(parameters, parameterTag, size);
  return isActive;
}


} // namespace unplug
