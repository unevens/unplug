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
#include "imgui_internal.h" //ImVec2 and ImVec4 operators
#include "unplug/Index.hpp"
#include "unplug/Math.hpp"
#include "unplug/ParameterAccess.hpp"
#include <functional>
#include <string>

namespace unplug {

enum class ShowLabel
{
  no,
  yes
};

/**
 * Combo ImGui control associated with a plugin parameter
 * */
bool Combo(ParamIndex paramIndex, ShowLabel showLabel = ShowLabel::yes);

/**
 * Checkbox ImGui control associated with a plugin parameter
 * */
bool Checkbox(ParamIndex paramIndex, ShowLabel showLabel = ShowLabel::yes);

/**
 * Displays some text centered in the rectangle between the current position and size
 * */
void TextCentered(std::string const& text, float height = 0.f);

/**
 * Displays the name of a parameter
 * */
void NameLabel(ParamIndex paramIndex);

/**
 * Displays the name of a parameter, centered in the rectangle between the current position and size
 * */
void NameLabelCentered(ParamIndex paramIndex, float height = 0.f);

/**
 * Displays the value of a parameter as text. Not editable
 * */
void ValueLabel(ParamIndex paramIndex, ShowLabel showLabel = ShowLabel::yes);

/**
 * Displays the value of a parameter as text, centered in the rectangle between the current position and size
 * */
void ValueLabelCentered(ParamIndex paramIndex, ShowLabel showLabel = ShowLabel::yes, float height = 0);

/**
 * Displays the value of a meter as text. Not editable
 * */
void MeterValueLabel(MeterIndex meterIndex,
                     std::function<std::string(float)> const& toString,
                     float fallbackValue = 0.f);

/**
 * Displays the value of a meter as text, centered in the rectangle between the current position and size. Not editable
 * */
void MeterValueLabelCentered(MeterIndex meterIndex,
                             std::string const& prefix = "",
                             std::function<std::string(float)> const& toString = unplug::linearToDBAsText,
                             float height = 0.f);

/**
 * Style for level meter filling
 * */
enum class FillStyle
{
  gradient,
  solid
};

/**
 * Alignment options for level meters.
 * */
enum class LevelMeterAlign
{
  toMinValue,
  toMaxValue
};

/**
 * Settings for a level meter
 * */
struct LevelMeterSettings
{
  float minValue = -90.0;
  float maxValue = 12.0;
  ImVec4 minValueColor = { 0.f, 1.f, 0.f, 1.f };
  ImVec4 maxValueColor = { 1.f, 0.f, 0.f, 1.f };
  ImVec4 intermediateColor = { 1.f, 1.f, 0.f, 1.f };
  float relativePositionOfIntermediateColor = 0.5f;
  FillStyle fillStyle = FillStyle::gradient;
  float thicknessAtZero = 1.f;
  std::function<float(float)> scaling = linearToDB<float>;
};

struct DifferenceLevelMeterSettings : LevelMeterSettings
{
  float centerValue = 0.0f;
  DifferenceLevelMeterSettings()
  {
    minValue = -36.f;
    maxValue = 36.f;
  }
};

/**
 * Level meter.
 * */
void LevelMeter(MeterIndex meterIndex,
                std::string const& name,
                ImVec2 size,
                LevelMeterSettings const& settings = {},
                LevelMeterAlign alignment = LevelMeterAlign::toMinValue);

/**
 * Level meter taking an explicit value
 * */
void LevelMeterRaw(float value,
                   std::string const& name,
                   ImVec2 size,
                   LevelMeterSettings const& settings = {},
                   LevelMeterAlign alignment = LevelMeterAlign::toMinValue);

/**
 * Difference level meter.
 * */
void DifferenceLevelMeter(MeterIndex meterIndex,
                          std::string const& name,
                          ImVec2 size,
                          DifferenceLevelMeterSettings settings);

/**
 * Difference level taking an explicit value.
 * */
void DifferenceLevelMeterRaw(float value, std::string const& name, ImVec2 size, DifferenceLevelMeterSettings settings);

/**
 * Displays the value of a parameter as text, allowing user input upon click or double click, centered in the rectangle
 * between the current position and size
 * */
bool ValueAsText(ParamIndex paramIndex,
                 ShowLabel showLabel = ShowLabel::yes,
                 const char* format = "%.1f",
                 bool noHighlight = true);

/**
 * Data to characterize the state of a parameter
 * */

bool DragFloat(ParamIndex paramIndex,
               ShowLabel showLabel = ShowLabel::no,
               float speedCoef = 1.f,
               const char* format = "%.1f",
               ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * SliderFloat ImGui control associated with a plugin parameter
 * */
bool SliderFloat(ParamIndex paramIndex,
                 ShowLabel showLabel = ShowLabel::no,
                 const char* format = "%.1f",
                 ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * Vertical SliderFloat ImGui control associated with a plugin parameter
 * */
bool VSliderFloat(ParamIndex paramIndex,
                  ImVec2 size,
                  ShowLabel showLabel = ShowLabel::no,
                  const char* format = "%.1f",
                  ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * SliderInt ImGui control associated with a plugin parameter
 * */
bool SliderInt(ParamIndex paramIndex,
               ShowLabel showLabel = ShowLabel::no,
               const char* format = "%d",
               ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * Vertical SliderInt ImGui control associated with a plugin parameter
 * */
bool VSliderInt(ParamIndex paramIndex,
                ImVec2 size,
                ShowLabel showLabel = ShowLabel::no,
                const char* format = "%d",
                ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * Knob control stuff
 * */

/**
 * Data needed to draw a knob. An object of this type is passed to the function that draws the knob.
 * */
struct KnobDrawData
{
  float angleOffset = pi<float> / 4;
  ImVec2 center;
  ImVec2 pointerPosition;
  bool isActive = false;
  bool isHovered = false;
};

/**
 * Minimalistic knob drwaer
 * */
void DrawSimpleKnob(KnobDrawData const& knob);

/**
 * Knob control associated with a plugin parameter
 * */
bool Knob(ParamIndex paramIndex,
          float power = 1.f,
          float angleOffset = pi<float> / 4,
          std::function<void(KnobDrawData const&)> const& drawer = DrawSimpleKnob);

/**
 * Knob control associated with a plugin parameter, which also display the name and value of the parameter
 * */
bool KnobWithLabels(ParamIndex paramIndex,
                    float power = 1.f,
                    float angleOffset = pi<float> / 4,
                    std::function<void(KnobDrawData const&)> const& drawer = DrawSimpleKnob);

struct ParameterData
{
  std::string name;
  float valueNormalized;
  float value;
  float minValue;
  float maxValue;
  std::string measureUnit;
  bool isBeingEdited;

  ParameterData(ParameterAccess& parameters, ParamIndex paramIndex);
};

/**
 * Data to characterize the state of a control
 * */
struct ControlOutput
{
  std::string controlName;
  float value = 0.f;
  bool isActive = false;
};

/**
 * Controls a parameter with a custom control, used internally by most control that allows to edit a parameter
 * */
bool Control(ParamIndex paramIndex, std::function<ControlOutput(ParameterData const& parameter)> const& control);

/**
 * implementation details that can be useful to implement custom controls
 * */
namespace detail {
struct EditingState
{
  bool isParameterBeingEdited;
  bool isControlActive;
  std::string controlName;

  EditingState(ParameterData const& parameterData, bool isControlActive, std::string controlName);
};

void applyRangedParameters(ParameterAccess& parameters,
                           ParamIndex paramIndex,
                           EditingState editingState,
                           float valueNormalized);

struct KnobOutput
{
  KnobDrawData drawData;
  double value = 0.f;
  bool isActive = false;
};

/**
 * An ImGui Knob control
 * */
KnobOutput Knob(const char* name, float inputValue, float angleOffset = pi<float> / 4);

/**
 * An ImGui control that display a scalar value as text and allows for user input on double click
 * */
bool EditableScalar(const char* label,
                    ImGuiDataType data_type,
                    void* p_data,
                    void* p_min,
                    void* p_max,
                    const char* format,
                    bool noHighlight = true);

/**
 * An ImGui control that display a float value as text and allows for user input on double click
 * */
bool EditableFloat(const char* label,
                   float* value,
                   float min,
                   float max,
                   const char* format = "%.1f",
                   bool noHighlight = true);

/**
 * An ImGui control that display an int value as text and allows for user input on double click.
 * */
bool EditableInt(const char* label, int* value, int min, int max, const char* format = "%d", bool noHighlight = true);

/**
 * utilities to register what areas in the ui controls which parameters and report it to the host
 */
using BeginRegisteAreaInfo = std::array<int, 3>;

BeginRegisteAreaInfo beginRegisterArea();

void endRegisterArea(ParameterAccess& parameters, ParamIndex paramIndex, BeginRegisteAreaInfo const& area);

} // namespace detail

} // namespace unplug
