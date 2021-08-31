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

/*
 * This file includes code adapted from https://github.com/ocornut/imgui/blob/master/imgui_widgets.cpp
 * Copyright (c) 2014-2021 Omar Cornut, MIT LICENSE
 * */

#include "unplug/Controls.hpp"

namespace unplug {

using namespace detail;

static std::string
makeLabel(ShowLabel showLabel, std::string const& parameterName, const char* controlSuffix)
{
  return showLabel == ShowLabel::yes ? parameterName + "##" + controlSuffix : "##" + parameterName + controlSuffix;
}

static std::string
makeFormat(ParameterData const& parameter, const char* format)
{
  return !parameter.measureUnit.empty() ? format + parameter.measureUnit : format;
}

bool
Combo(int parameterTag, ShowLabel showLabel)
{
  using namespace ImGui;

  auto& parameters = Parameters();

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

  auto controlName = makeLabel(showLabel, parameterName, "COMBO");

  if (!BeginCombo(controlName.c_str(), valueAsText.c_str(), ImGuiComboFlags_None))
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
    MarkItemEdited(g.LastItemData.ID);
  }

  return hasValueChanged;
}

bool
Checkbox(int parameterTag, ShowLabel showLabel)
{
  return Control(parameterTag, [=](ParameterData const& parameter) {
    auto isChecked = parameter.value != 0.0;
    auto const controlName = makeLabel(showLabel, parameter.name, "CHECKBOX");
    bool const isActive = ImGui::Checkbox(controlName.c_str(), &isChecked);
    return ControlOutput{ isChecked ? 1.f : 0.f, isActive };
  });
}

void
Label(int parameterTag)
{
  auto& parameters = Parameters();
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
LabelCentered(int parameterTag, ImVec2 size)
{
  auto& parameters = Parameters();
  auto const name = parameters.getName(parameterTag);
  TextCentered(name + "##LABELCENTERED", size);
}

void
ValueAsText(int parameterTag)
{
  auto& parameters = Parameters();
  auto const valueAsText = parameters.getValueAsText(parameterTag);
  return ImGui::TextUnformatted(valueAsText.c_str());
}

void
ValueAsTextCentered(int parameterTag, ImVec2 size, ShowLabel showLabel)
{
  auto& parameters = Parameters();
  auto const valueAsText = parameters.getValueAsText(parameterTag);
  auto const text =
    (showLabel == ShowLabel::yes ? (parameters.getName(parameterTag) + ": " + valueAsText) : valueAsText) +
    "##VALUUEASTEXTCENTERED";
  TextCentered(text, size);
}

ParameterData::ParameterData(ParameterAccess& parameters, int parameterTag)
  : name(parameters.getName(parameterTag))
  , valueNormalized(static_cast<float>(parameters.getValueNormalized(parameterTag)))
  , value(static_cast<float>(parameters.valueFromNormalized(parameterTag, valueNormalized)))
  , minValue(static_cast<float>(parameters.getMinValue(parameterTag)))
  , maxValue(static_cast<float>(parameters.getMaxValue(parameterTag)))
  , measureUnit(parameters.getMeasureUnit(parameterTag))
  , isBeingEdited(parameters.isBeingEdited(parameterTag))
{}

EditingState::EditingState(const ParameterData& parameterData, bool isControlActive)
  : isParameterBeingEdited(parameterData.isBeingEdited)
  , isControlActive(isControlActive)
{}

bool
Control(int parameterTag, std::function<ControlOutput(ParameterData const& parameter)> const& control)
{
  auto& parameters = Parameters();
  auto const parameter = ParameterData{ parameters, parameterTag };
  auto const output = control(parameter);
  auto const editingState = EditingState{ parameter, output.isActive };
  applyRangedParameters(parameters, parameterTag, editingState, output.value);
  return output.isActive;
}

bool
ValueAsText(int parameterTag, ShowLabel showLabel, const char* format)
{
  return Control(parameterTag, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "FLOATASTEXT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = detail::EditableFloat(
      controlName.c_str(), &outputValue, parameter.minValue, parameter.maxValue, formatWithUnit.c_str());
    auto& parameters = Parameters();
    outputValue = parameters.normalizeValue(parameterTag, outputValue);
    return ControlOutput{ outputValue, isActive };
  });
}

bool
SliderFloat(int parameterTag, ShowLabel showLabel, const char* format, ImGuiSliderFlags flags)
{
  return Control(parameterTag, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "SLIDERFLOAT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::SliderFloat(
      controlName.c_str(), &outputValue, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), flags);
    auto& parameters = Parameters();
    outputValue = parameters.normalizeValue(parameterTag, outputValue);
    return ControlOutput{ outputValue, isActive };
  });
}

bool
DragFloat(int parameterTag, ShowLabel showLabel, float speed, const char* format, ImGuiSliderFlags flags)
{
  return Control(parameterTag, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "DRAGFLOAT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::DragFloat(
      controlName.c_str(), &outputValue, speed, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), flags);
    auto& parameters = Parameters();
    outputValue = parameters.normalizeValue(parameterTag, outputValue);
    return ControlOutput{ outputValue, isActive };
  });
}

void
DrawSimpleKnob(KnobDrawData const& knob)
{
  // originally based on https://github.com/ocornut/imgui/issues/942
  ImU32 col32 = ImGui::GetColorU32(knob.isActive    ? ImGuiCol_FrameBgActive
                                   : knob.isHovered ? ImGuiCol_FrameBgHovered
                                                    : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  auto const numSegments = static_cast<int>(1.5f * knob.layout.radius);
  draw_list->AddCircleFilled(knob.center, knob.layout.radius, col32, numSegments);
  draw_list->AddLine(knob.center, knob.pointerPosition, col32line, 1);
}

bool
Knob(int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> const& drawer)
{
  return Control(parameterTag, [&](ParameterData const& parameter) {
    auto controlName = parameter.name + "##KNOB";
    auto const knobOutput = Knob(controlName.c_str(), static_cast<float>(parameter.valueNormalized), layout);
    drawer(knobOutput.drawData);
    return knobOutput.output;
  });
}

bool
KnobWithLabels(int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> const& drawer)
{
  auto& parameters = Parameters();
  auto const size = ImVec2{ layout.radius * 2, 2 * ImGui::GetTextLineHeight() };
  LabelCentered(parameterTag, size);
  auto const isActive = Knob(parameterTag, layout, drawer);
  ValueAsTextCentered(parameterTag, size, ShowLabel::no);
  return isActive;
}

namespace detail {

void
applyRangedParameters(ParameterAccess& parameters, int parameterTag, EditingState editingState, float valueNormalized)
{
  if (editingState.isParameterBeingEdited) {
    if (editingState.isControlActive) {
      bool const setValueOk = parameters.setValueNormalized(parameterTag, valueNormalized);
      assert(setValueOk);
    }
    else {
      bool const endEditOk = parameters.endEdit(parameterTag);
      assert(endEditOk);
    }
  }
  else {
    if (editingState.isControlActive) {
      bool const beginEditOk = parameters.beginEdit(parameterTag);
      assert(beginEditOk);
      bool const setValueOk = parameters.setValueNormalized(parameterTag, valueNormalized);
      assert(setValueOk);
    }
  }
}

KnobOutput
Knob(const char* name, float const inputValue, KnobLayout layout)
{
  // originally based on https://github.com/ocornut/imgui/issues/942
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

bool
EditableScalar(const char* label, ImGuiDataType data_type, void* p_data, void* p_min, void* p_max, const char* format)
{
  // based on ImGui::DragScalar
  using namespace ImGui;
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);
  const float w = CalcItemWidth();

  const ImVec2 label_size = CalcTextSize(label, NULL, true);
  const ImRect frame_bb(window->DC.CursorPos,
                        window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
  const ImRect total_bb(
    frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id, &frame_bb, ImGuiItemAddFlags_Focusable))
    return false;

  // Tabbing or CTRL-clicking on Drag turns it into an InputText
  const bool hovered = ItemHoverable(frame_bb, id);
  bool temp_input_is_active = TempInputIsActive(id);
  if (!temp_input_is_active) {
    const bool focus_requested = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Focused) != 0;
    const bool clicked = (hovered && g.IO.MouseClicked[0]);
    const bool double_clicked = (hovered && g.IO.MouseDoubleClicked[0]);
    if (focus_requested || clicked || double_clicked || g.NavActivateId == id || g.NavInputId == id) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
      if (focus_requested || (clicked && g.IO.KeyCtrl) || double_clicked || g.NavInputId == id)
        temp_input_is_active = true;
    }
    if (!temp_input_is_active)
      if (/*g.IO.ConfigDragClickToInputText && */ g.ActiveId == id && hovered && g.IO.MouseReleased[0]) {
        g.NavInputId = id;
        temp_input_is_active = true;
      }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (p_min == NULL || p_max == NULL || DataTypeCompare(data_type, p_min, p_max) < 0);
    return TempInputScalar(
      frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive
                                      : hovered        ? ImGuiCol_FrameBgHovered
                                                       : ImGuiCol_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

  // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
  char value_buf[64];
  const char* value_buf_end =
    value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

  if (label_size.x > 0.0f)
    RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

  IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
  return false;
}

bool
EditableFloat(const char* label, float* value, float min, float max, const char* format)
{
  return EditableScalar(label, ImGuiDataType_Float, value, &min, &max, format);
}

bool
EditableInt(const char* label, int* value, int min, int max, const char* format)
{
  return EditableScalar(label, ImGuiDataType_S32, value, &min, &max, format);
}

} // namespace detail
} // namespace unplug
