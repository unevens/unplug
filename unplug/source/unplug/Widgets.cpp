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

#include "unplug/Widgets.hpp"
#include "imgui_internal.h"
#include "unplug/MeterStorage.hpp"

namespace unplug {

using namespace detail;

static std::string makeLabel(ShowLabel showLabel, std::string const& parameterName, const char* controlSuffix)
{
  return showLabel == ShowLabel::yes ? parameterName + "##" + controlSuffix : "##" + parameterName + controlSuffix;
}

static std::string makeFormat(ParameterData const& parameter, const char* format)
{
  return !parameter.measureUnit.empty() ? format + (" " + parameter.measureUnit) : format;
}

bool Combo(ParamIndex paramIndex, ShowLabel showLabel)
{
  using namespace ImGui;

  auto& parameters = getParameters();

  auto const areaInfo = beginRegisterArea();

  bool const isList = parameters.isList(paramIndex);
  assert(isList);

  double const value = parameters.getValue(paramIndex);
  auto const parameterName = parameters.getName(paramIndex);

  auto const valueAsText = parameters.convertToText(paramIndex, value);

  auto const numSteps = parameters.getNumSteps(paramIndex);

  auto const controlName = makeLabel(showLabel, parameterName, "COMBO");

  if (!BeginCombo(controlName.c_str(), valueAsText.c_str(), ImGuiComboFlags_None)) {
    endRegisterArea(parameters, paramIndex, areaInfo);
    return false;
  }

  auto const numItems = numSteps + 1;

  bool hasValueChanged = false;
  auto newValue = value;
  for (int i = 0; i < numItems; i++) {
    PushID((void*)(intptr_t)i);
    const bool selectedItem = (i == newValue);
    auto const itemText = parameters.convertToText(paramIndex, (double)i);
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
    ImGuiContext const& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    auto const controlId = window->IDStack.back();
    parameters.beginEdit(paramIndex, controlName);
    parameters.setValueNormalized(paramIndex, newValue);
    parameters.endEdit(paramIndex);
    MarkItemEdited(g.LastItemData.ID);
  }

  endRegisterArea(parameters, paramIndex, areaInfo);

  return hasValueChanged;
}

bool Checkbox(ParamIndex paramIndex, ShowLabel showLabel)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto isChecked = parameter.value != 0.0;
    auto const controlName = makeLabel(showLabel, parameter.name, "CHECKBOX");
    bool const isActive = ImGui::Checkbox(controlName.c_str(), &isChecked);
    return ControlOutput{ controlName, isChecked ? 1.f : 0.f, isActive };
  });
}

void TextCentered(std::string const& text, float height)
{
  auto const bkgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
  ImGui::PushStyleColor(ImGuiCol_Button, bkgColor);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bkgColor);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, bkgColor);
  ImGui::Button(text.c_str(), { ImGui::CalcItemWidth(), height });
  ImGui::PopStyleColor(3);
}

void NameLabel(ParamIndex paramIndex)
{
  auto& parameters = getParameters();
  auto const name = parameters.getName(paramIndex);
  return ImGui::TextUnformatted(name.c_str());
}

void NameLabelCentered(ParamIndex paramIndex, float height)
{
  auto& parameters = getParameters();
  auto const name = parameters.getName(paramIndex);
  TextCentered(name + "##LABELCENTERED", height);
}

void ValueLabel(ParamIndex paramIndex, ShowLabel showLabel)
{
  auto& parameters = getParameters();
  auto const valueAsText = parameters.getValueAsText(paramIndex);
  auto const text = showLabel == ShowLabel::yes ? (parameters.getName(paramIndex) + ": " + valueAsText) : valueAsText;
  return ImGui::TextUnformatted(text.c_str());
}

void ValueLabelCentered(ParamIndex paramIndex, ShowLabel showLabel, float height)
{
  auto& parameters = getParameters();
  auto const valueAsText = parameters.getValueAsText(paramIndex);
  auto const text =
    (showLabel == ShowLabel::yes ? (parameters.getName(paramIndex) + ": " + valueAsText) : valueAsText) +
    "##VALUUEASTEXTCENTERED";
  TextCentered(text, height);
}

void MeterValueLabel(MeterIndex meterIndex, std::function<std::string(float)> const& toString, float fallbackValue)
{
  auto meters = getMeters();
  auto const value = meters ? meters->get(meterIndex) : fallbackValue;
  auto const valueAsText = toString(value);
  return ImGui::TextUnformatted(valueAsText.c_str());
}

void MeterValueLabelCentered(MeterIndex meterIndex,
                             std::function<std::string(float)> const& toString,
                             float fallbackValue,
                             float height)
{
  auto meters = getMeters();
  auto const value = meters ? meters->get(meterIndex) : fallbackValue;
  auto const valueAsText = toString(value);
  TextCentered(valueAsText, height);
}

ParameterData::ParameterData(ParameterAccess& parameters, ParamIndex paramIndex)
  : name(parameters.getName(paramIndex))
  , valueNormalized(static_cast<float>(parameters.getValueNormalized(paramIndex)))
  , value(static_cast<float>(parameters.valueFromNormalized(paramIndex, valueNormalized)))
  , minValue(static_cast<float>(parameters.getMinValue(paramIndex)))
  , maxValue(static_cast<float>(parameters.getMaxValue(paramIndex)))
  , measureUnit(parameters.getMeasureUnit(paramIndex))
  , isBeingEdited(parameters.isBeingEdited(paramIndex))
{}

EditingState::EditingState(const ParameterData& parameterData, bool isControlActive, std::string controlName)
  : isParameterBeingEdited(parameterData.isBeingEdited)
  , isControlActive(isControlActive)
  , controlName(std::move(controlName))
{}

bool Control(ParamIndex paramIndex, std::function<ControlOutput(ParameterData const& parameter)> const& control)
{
  auto& parameters = getParameters();
  auto const parameter = ParameterData{ parameters, paramIndex };
  auto const areaInfo = beginRegisterArea();
  auto const output = control(parameter);
  endRegisterArea(parameters, paramIndex, areaInfo);
  auto const editingState = EditingState{ parameter, output.isActive, output.controlName };
  applyRangedParameters(parameters, paramIndex, editingState, output.value);
  return output.isActive;
}

bool ValueAsText(ParamIndex paramIndex, ShowLabel showLabel, const char* format, bool noHighlight)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "FLOATASTEXT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = detail::EditableFloat(
      controlName.c_str(), &outputValue, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), noHighlight);
    return ControlOutput{ controlName, outputValue, isActive };
  });
}

bool SliderFloat(ParamIndex paramIndex, ShowLabel showLabel, const char* format, ImGuiSliderFlags flags)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "SLIDERFLOAT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::SliderFloat(
      controlName.c_str(), &outputValue, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), flags);
    return ControlOutput{ controlName, outputValue, isActive };
  });
}

bool VSliderFloat(ParamIndex paramIndex, ImVec2 size, ShowLabel showLabel, const char* format, ImGuiSliderFlags flags)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "VSLIDERFLOAT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::VSliderFloat(
      controlName.c_str(), size, &outputValue, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), flags);
    return ControlOutput{ controlName, outputValue, isActive };
  });
}

bool SliderInt(ParamIndex paramIndex, ShowLabel showLabel, const char* format, ImGuiSliderFlags flags)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<int>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "VSLIDERINT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::SliderInt(controlName.c_str(),
                                           &outputValue,
                                           static_cast<int>(parameter.minValue),
                                           static_cast<int>(parameter.maxValue),
                                           formatWithUnit.c_str(),
                                           flags);
    return ControlOutput{ controlName, static_cast<float>(outputValue), isActive };
  });
}

bool VSliderInt(ParamIndex paramIndex, ImVec2 size, ShowLabel showLabel, const char* format, ImGuiSliderFlags flags)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<int>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "VSLIDERINT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::VSliderInt(controlName.c_str(),
                                            size,
                                            &outputValue,
                                            static_cast<int>(parameter.minValue),
                                            static_cast<int>(parameter.maxValue),
                                            formatWithUnit.c_str(),
                                            flags);
    return ControlOutput{ controlName, static_cast<float>(outputValue), isActive };
  });
}

bool DragFloat(ParamIndex paramIndex, ShowLabel showLabel, float speed, const char* format, ImGuiSliderFlags flags)
{
  return Control(paramIndex, [=](ParameterData const& parameter) {
    auto outputValue = static_cast<float>(parameter.value);
    auto const controlName = makeLabel(showLabel, parameter.name, "DRAGFLOAT");
    auto const formatWithUnit = makeFormat(parameter, format);
    bool const isActive = ImGui::DragFloat(
      controlName.c_str(), &outputValue, speed, parameter.minValue, parameter.maxValue, formatWithUnit.c_str(), flags);
    return ControlOutput{ controlName, outputValue, isActive };
  });
}

void DrawSimpleKnob(KnobDrawData const& knob)
{
  auto const radius = 0.5f * ImGui::CalcItemWidth();
  // originally based on https://github.com/ocornut/imgui/issues/942
  ImU32 col32 = ImGui::GetColorU32(knob.isActive    ? ImGuiCol_FrameBgActive
                                   : knob.isHovered ? ImGuiCol_FrameBgHovered
                                                    : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  auto const numSegments = static_cast<int>(1.5f * radius);
  drawList->AddCircleFilled(knob.center, radius, col32, numSegments);
  drawList->AddLine(knob.center, knob.pointerPosition, col32line, 1);
}

bool Knob(ParamIndex paramIndex, float power, float angleOffset, std::function<void(KnobDrawData const&)> const& drawer)
{
  return Control(paramIndex, [&](ParameterData const& parameter) {
    auto controlName = parameter.name + "##KNOB";
    auto const scaledInput =
      std::pow((parameter.value - parameter.minValue) / (parameter.maxValue - parameter.minValue), 1.f / power);
    auto const knobOutput = Knob(controlName.c_str(), scaledInput, angleOffset);
    drawer(knobOutput.drawData);
    auto const outputValue =
      parameter.minValue + (parameter.maxValue - parameter.minValue) * std::pow(static_cast<float>(knobOutput.value), power);
    return ControlOutput{ controlName, outputValue, knobOutput.isActive };
  });
}

bool KnobWithLabels(ParamIndex paramIndex,
                    float power,
                    float angleOffset,
                    std::function<void(KnobDrawData const&)> const& drawer)
{
  NameLabelCentered(paramIndex);
  auto const isActive = Knob(paramIndex, power, angleOffset, drawer);
  auto const bkgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
  ImGui::PushStyleColor(ImGuiCol_FrameBg, bkgColor);
  ValueAsText(paramIndex, ShowLabel::no);
  ImGui::PopStyleColor();
  return isActive;
}

static void DrawLevelMeter(float scaledValue,
                           ImVec2 cursorPosition,
                           ImVec2 size,
                           LevelMeterSettings const& settings,
                           LevelMeterAlign alignment,
                           bool isHorizontal)
{
  auto const normalizedValue =
    std::max(0.f, std::min(1.f, (scaledValue - settings.minValue) / (settings.maxValue - settings.minValue)));
  auto const valueColor = mix(settings.minValueColor,
                              settings.maxValueColor,
                              settings.intermediateColor,
                              normalizedValue,
                              settings.relativePositionOfIntermediateColor);
  auto topLeftColor = valueColor;
  auto topRightColor = valueColor;
  auto bottomRightColor = valueColor;
  auto bottomLeftColor = valueColor;

  float left, top, right, bottom;
  if (isHorizontal) {
    top = cursorPosition.y;
    bottom = cursorPosition.y + size.y;
    auto const valueX = cursorPosition.x + size.x * normalizedValue;
    if (alignment == LevelMeterAlign::toMinValue) {
      left = cursorPosition.x;
      right = valueX;
      if (settings.fillStyle == FillStyle::gradient) {
        topLeftColor = settings.minValueColor;
        bottomLeftColor = settings.minValueColor;
      }
    }
    else if (alignment == LevelMeterAlign::toMaxValue) {
      right = cursorPosition.x + size.x;
      left = valueX;
      if (settings.fillStyle == FillStyle::gradient) {
        topRightColor = settings.maxValueColor;
        bottomRightColor = settings.maxValueColor;
      }
    }
  }
  else { // is vertical
    left = cursorPosition.x;
    right = left + size.x;
    auto const valueY = cursorPosition.y + size.y * (1.f - normalizedValue);
    if (alignment == LevelMeterAlign::toMinValue) {
      bottom = cursorPosition.y + size.y;
      top = valueY;
    }
    else if (alignment == LevelMeterAlign::toMaxValue) {
      top = cursorPosition.y;
      bottom = valueY;
    }
  }
  if (right > left && bottom > top) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (settings.fillStyle == FillStyle::gradient) {
      bool const beyondIntermediate = alignment == LevelMeterAlign::toMinValue
                                        ? normalizedValue > settings.relativePositionOfIntermediateColor
                                        : normalizedValue < settings.relativePositionOfIntermediateColor;
      if (beyondIntermediate) {
        auto intermediateX = cursorPosition.x + settings.relativePositionOfIntermediateColor * size.x;
        auto intermediateY = cursorPosition.y + size.y * (1.f - settings.relativePositionOfIntermediateColor);
        {
          drawList->AddRectFilledMultiColor(
            { left, top },
            { isHorizontal ? intermediateX : right, isHorizontal ? bottom : intermediateY },
            ImGui::ColorConvertFloat4ToU32(isHorizontal ? topLeftColor : settings.intermediateColor),
            ImGui::ColorConvertFloat4ToU32(isHorizontal ? settings.intermediateColor : topRightColor),
            ImGui::ColorConvertFloat4ToU32(isHorizontal ? settings.intermediateColor : bottomRightColor),
            ImGui::ColorConvertFloat4ToU32(bottomLeftColor));
        }
        {
          drawList->AddRectFilledMultiColor(
            { isHorizontal ? intermediateX : left, isHorizontal ? top : intermediateY },
            { right, bottom },
            ImGui::ColorConvertFloat4ToU32(isHorizontal ? settings.intermediateColor : topLeftColor),
            ImGui::ColorConvertFloat4ToU32(topRightColor),
            ImGui::ColorConvertFloat4ToU32(isHorizontal ? bottomRightColor : settings.intermediateColor),
            ImGui::ColorConvertFloat4ToU32(settings.intermediateColor));
        }
      }
      else {
        drawList->AddRectFilledMultiColor({ left, top },
                                          { right, bottom },
                                          ImGui::ColorConvertFloat4ToU32(topLeftColor),
                                          ImGui::ColorConvertFloat4ToU32(topRightColor),
                                          ImGui::ColorConvertFloat4ToU32(bottomRightColor),
                                          ImGui::ColorConvertFloat4ToU32(bottomLeftColor));
      }
    }
    else {
      drawList->AddRectFilled({ left, top }, { right, bottom }, ImGui::ColorConvertFloat4ToU32(valueColor));
    }
  }
}

static float getMeterValue(MeterIndex meterIndex, LevelMeterSettings const& settings)
{
  auto const meters = getMeters();
  auto const rawValue = meters ? meters->get(meterIndex) : settings.fallbackValue;
  return rawValue;
}

void LevelMeterRaw(float rawValue,
                   std::string const& name,
                   ImVec2 size,
                   LevelMeterSettings const& settings,
                   LevelMeterAlign alignment)
{
  auto const scaledValue = settings.scaling(rawValue);
  auto const cursorPosition = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton((name + "##LEVELMETER").c_str(), size);
  bool const isHorizontal = size.x > size.y;
  DrawLevelMeter(scaledValue, cursorPosition, size, settings, alignment, isHorizontal);
}

void LevelMeter(MeterIndex meterIndex,
                std::string const& name,
                ImVec2 size,
                LevelMeterSettings const& settings,
                LevelMeterAlign alignment)
{
  auto const rawValue = getMeterValue(meterIndex, settings);
  LevelMeterRaw(rawValue, name, size, settings, alignment);
}

void DifferenceLevelMeterRaw(float rawValue,
                             std::string const& name,
                             ImVec2 size,
                             DifferenceLevelMeterSettings settings)
{

  assert(settings.minValue < settings.centerValue);
  assert(settings.maxValue > settings.centerValue);
  auto const scaledValue = settings.scaling(rawValue);
  auto const scaledDifference = scaledValue - settings.centerValue;
  auto const centerPercentage = (settings.centerValue - settings.minValue) / (settings.maxValue - settings.minValue);
  bool const isHorizontal = size.x > size.y;
  auto const cursorPosition = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton((name + "##LEVELMETERBI").c_str(), size);
  if (isHorizontal) {
    auto const lowerMeterLeftTop = cursorPosition;
    auto const lowerMeterSize = ImVec2{ centerPercentage * size.x, size.y };
    if (scaledDifference < 0) {
      settings.maxValue = settings.centerValue;
      std::swap(settings.maxValueColor, settings.minValueColor);
      DrawLevelMeter(
        scaledDifference, lowerMeterLeftTop, lowerMeterSize, settings, LevelMeterAlign::toMaxValue, isHorizontal);
    }
    if (scaledDifference > 0) {
      auto const higherMeterLeftTop = ImVec2{ lowerMeterLeftTop.x + lowerMeterSize.x, lowerMeterLeftTop.y };
      auto const higherMeterSize = ImVec2{ size.x - lowerMeterSize.x, lowerMeterSize.y };
      settings.minValue = settings.centerValue;
      DrawLevelMeter(
        scaledDifference, higherMeterLeftTop, higherMeterSize, settings, LevelMeterAlign::toMinValue, isHorizontal);
    }
    if (std::abs(scaledDifference) < std::numeric_limits<float>::epsilon()) {
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      auto const color = ImGui::ColorConvertFloat4ToU32(settings.minValueColor);
      auto const x = cursorPosition.x + centerPercentage * size.x;
      auto const top = ImVec2{ x, cursorPosition.y };
      auto const bottom = ImVec2{ x, cursorPosition.y + size.y };
      drawList->AddLine(top, bottom, color, settings.thicknessAtZero);
    }
  }
  else {
    auto const higherMeterLeftTop = cursorPosition;
    auto const higherMeterSize = ImVec2{ size.x, centerPercentage * size.y };
    if (scaledDifference > 0) {
      settings.minValue = settings.centerValue;
      DrawLevelMeter(
        scaledDifference, higherMeterLeftTop, higherMeterSize, settings, LevelMeterAlign::toMinValue, isHorizontal);
    }
    auto const lowerMeterLeftTop = ImVec2{ higherMeterLeftTop.x, higherMeterLeftTop.y + higherMeterSize.y };
    auto const lowerMeterSize = ImVec2{ size.x, size.y - higherMeterSize.y };
    if (scaledDifference < 0) {
      settings.maxValue = settings.centerValue;
      std::swap(settings.maxValueColor, settings.minValueColor);
      DrawLevelMeter(
        scaledDifference, lowerMeterLeftTop, lowerMeterSize, settings, LevelMeterAlign::toMaxValue, isHorizontal);
    }
    if (std::abs(scaledDifference) < std::numeric_limits<float>::epsilon()) {
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      auto const color = ImGui::ColorConvertFloat4ToU32(settings.minValueColor);
      auto const y = cursorPosition.y + centerPercentage * size.y;
      auto const left = ImVec2{ cursorPosition.x, y };
      auto const right = ImVec2{ cursorPosition.x + size.x, y };
      drawList->AddLine(left, right, color, settings.thicknessAtZero);
    }
  }
}

void DifferenceLevelMeter(MeterIndex meterIndex,
                          std::string const& name,
                          ImVec2 size,
                          DifferenceLevelMeterSettings settings)
{
  auto const rawValue = getMeterValue(meterIndex, settings);
  DifferenceLevelMeterRaw(rawValue, name, size, settings);
}

namespace detail {

void applyRangedParameters(ParameterAccess& parameters, ParamIndex paramIndex, EditingState editingState, float value)
{

  if (editingState.isParameterBeingEdited) {
    auto const controlDoingTheEditing = parameters.getEditingControl(paramIndex);
    if (editingState.controlName != controlDoingTheEditing) {
      return;
    }
    if (editingState.isControlActive) {
      bool const setValueOk = parameters.setValue(paramIndex, value);
      assert(setValueOk);
    }
    else {
      bool const endEditOk = parameters.endEdit(paramIndex);
      assert(endEditOk);
    }
  }
  else {
    if (editingState.isControlActive) {
      bool const beginEditOk = parameters.beginEdit(paramIndex, editingState.controlName);
      assert(beginEditOk);
      bool const setValueOk = parameters.setValue(paramIndex, value);
      assert(setValueOk);
    }
  }
}

KnobOutput Knob(const char* name, float const inputValue, float angleOffset)
{
  // originally based on https://github.com/ocornut/imgui/issues/942
  auto const radius = 0.5f * ImGui::CalcItemWidth();
  ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
  ImVec2 center = ImVec2(cursorPosition.x + radius, cursorPosition.y + radius);

  auto const currentAngle = (pi<float> - angleOffset) * inputValue * 2.0f + angleOffset;

  auto const x = -std::sin(currentAngle) * radius + center.x;
  auto const y = std::cos(currentAngle) * radius + center.y;

  auto const diameter = 2 * radius;
  ImGui::InvisibleButton(name, ImVec2(diameter, diameter));

  bool const isActive = ImGui::IsItemActive();
  bool const isHovered = ImGui::IsItemHovered();
  auto outputValue = inputValue;

  if (isActive) {
    ImVec2 mp = ImGui::GetIO().MousePos;
    float nextAngle = std::atan2(mp.x - center.x, center.y - mp.y) + pi<float>;
    nextAngle = std::max(angleOffset, std::min(2.0f * pi<float> - angleOffset, nextAngle));
    outputValue = 0.5f * (nextAngle - angleOffset) / (pi<float> - angleOffset);
    bool const hasGoneBelowTheBottom = inputValue == 0.0 && outputValue > 0.5;
    bool const hasGoneOverTheTop = inputValue == 1.0 && outputValue < 0.5;
    bool const hasGoneOutsideTheRange = hasGoneBelowTheBottom || hasGoneOverTheTop;
    if (hasGoneOutsideTheRange) {
      outputValue = inputValue;
    }
  }

  KnobDrawData drawData;
  drawData.angleOffset = angleOffset;
  drawData.center = center;
  drawData.pointerPosition = ImVec2(x, y);
  drawData.isActive = isActive;
  drawData.isHovered = isHovered;

  return { drawData, outputValue, isActive };
}

bool EditableScalar(const char* label,
                    ImGuiDataType data_type,
                    void* p_data,
                    void* p_min,
                    void* p_max,
                    const char* format,
                    bool noHighlight)
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
  const ImU32 frame_col = GetColorU32(noHighlight        ? ImGuiCol_FrameBg
                                      : g.ActiveId == id ? ImGuiCol_FrameBgActive
                                      : hovered          ? ImGuiCol_FrameBgHovered
                                                         : ImGuiCol_FrameBg);
  if (!noHighlight)
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

bool EditableFloat(const char* label, float* value, float min, float max, const char* format, bool noHighlight)
{
  return EditableScalar(label, ImGuiDataType_Float, value, &min, &max, format, noHighlight);
}

bool EditableInt(const char* label, int* value, int min, int max, const char* format, bool noHighlight)
{
  return EditableScalar(label, ImGuiDataType_S32, value, &min, &max, format, noHighlight);
}

BeginRegisteAreaInfo beginRegisterArea()
{
  auto const width = 0.5f * ImGui::CalcItemWidth();
  auto const leftTop = ImGui::GetCursorScreenPos();
  return { static_cast<int>(leftTop.x), static_cast<int>(leftTop.y), static_cast<int>(leftTop.x + width) };
}

void endRegisterArea(ParameterAccess& parameters, ParamIndex paramIndex, BeginRegisteAreaInfo const& area)
{
  auto const bottom = static_cast<int>(ImGui::GetCursorScreenPos().y);
  parameters.addParameterRectangle(paramIndex, area[0], area[1], area[2], bottom);
}

} // namespace detail
} // namespace unplug
