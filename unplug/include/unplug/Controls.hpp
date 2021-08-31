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
#include <functional>
#include <utility>

namespace unplug {

inline constexpr auto pi = (float)M_PI;

enum class ShowLabel
{
  no,
  yes
};

/**
 * Combo ImGui control associated with a plugin parameter
 * */
bool
Combo(int parameterTag, ShowLabel showLabel = ShowLabel::yes);

/**
 * Checkbox ImGui control associated with a plugin parameter
 * */
bool
Checkbox(int parameterTag, ShowLabel showLabel = ShowLabel::yes);

/**
 * Displays some text centered in the rectangle between the current position and size
 * */
void
TextCentered(std::string const& text, ImVec2 size);

/**
 * Displays the name of a parameter
 * */
void
Label(int parameterTag);

/**
 * Displays the name of a parameter, centered in the rectangle between the current position and size
 * */
void
LabelCentered(int parameterTag, ImVec2 size);

/**
 * Displays the value of a parameter as text
 * */
void
ValueAsText(int parameterTag);

/**
 * Displays the value of a parameter as text, centered in the rectangle between the current position and size
 * */
void
ValueAsTextCentered(int parameterTag, ImVec2 size, ShowLabel showLabel = ShowLabel::yes);

/**
 * Data to characterize the state of a parameter
 * */
struct ParameterData
{
  std::string name;
  float valueNormalized;
  float value;
  float minValue;
  float maxValue;
  std::string valueAsText;
  bool isBeingEdited;

  ParameterData(ParameterAccess& parameters, int parameterTag);
};

bool
DragFloat(int parameterTag,
          ShowLabel showLabel = ShowLabel::no,
          float speed = 0.01f,
          const char* format = "%.3f",
          ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * SliderFloat ImGui control associated with a plugin parameter
 * */
bool
SliderFloat(int parameterTag,
            ShowLabel showLabel = ShowLabel::no,
            const char* format = "%.3f",
            ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * SliderInt ImGui control associated with a plugin parameter
 * */
bool
SliderInt(int parameterTag,
          ShowLabel showLabel = ShowLabel::no,
          const char* format = "%df",
          ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

/**
 * Knob control stuff, originally based on https://github.com/ocornut/imgui/issues/942
 * */

/**
 * Describes the layout of the knob control
 * */
struct KnobLayout
{
  float radius = 20.f;
  float angleOffset = pi / 4;
};

/**
 * Data needed to draw a knob. An object of this type is passed to the function that draws the knob.
 * */
struct KnobDrawData
{
  KnobLayout layout;
  ImVec2 center;
  ImVec2 pointerPosition;
  bool isActive = false;
  bool isHovered = false;
};

/**
 * Minimalistic knob drwaer
 * */
void
DrawSimpleKnob(KnobDrawData const& knob);

/**
 * Knob control associated with a plugin parameter
 * */
bool
Knob(int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> const& drawer = DrawSimpleKnob);

/**
 * Knob control associated with a plugin parameter, which also display the name and value of the parameter
 * */

bool
KnobWithLabels(int parameterTag,
               KnobLayout layout,
               std::function<void(KnobDrawData const&)> const& drawer = DrawSimpleKnob);

/**
 * Data to characterize the state of a control
 * */
struct ControlOutput
{
  float value = 0.f;
  bool isActive = false;
};

/**
 * Controls a parameter with a custom control, used internally by most control that allows to edit a parameter
 * */
bool
Control(int parameterTag, std::function<ControlOutput(ParameterData const& parameter)> const& control);

/**
 * implementation details that can be useful to implement custom controls
 * */
namespace detail {

struct EditingState
{
  bool isParameterBeingEdited;
  bool isControlActive;

  EditingState(ParameterData const& parameterData, bool isControlActive);
};

void
applyRangedParameters(ParameterAccess& parameters, int parameterTag, EditingState editingState, float valueNormalized);

struct KnobOutput
{
  KnobDrawData drawData;
  ControlOutput output;
  double value = 0.f;
  bool isActive = false;
};

/**
 * ImGui Knob control, not assocciated with any parameter*/
KnobOutput
Knob(const char* name, float inputValue, KnobLayout layout);

} // namespace detail

} // namespace unplug
