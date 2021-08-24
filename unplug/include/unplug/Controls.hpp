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
bool
Combo(ParameterAccess& parameters, int parameterTag);

/**
 * Checkbox ImGui control associated with a plugin parameter
 * */
bool
Checkbox(ParameterAccess& parameters, int parameterTag);

void
Label(ParameterAccess& parameters, int parameterTag);

void
ValueAsText(ParameterAccess& parameters, int parameterTag);

/**
 * Knob control stuff, originally based on https://github.com/ocornut/imgui/issues/942
 * The Knob function is templated in order to support customizable drawing. A minimalist one is provided as default.
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
  bool isActive;
  bool isHovered;
};

/**
 * Minimalistic knob drwaer
 * */
template<int numSegments>
inline void
DrawSimpleKnob(KnobDrawData const& knob)
{
  ImU32 col32 = ImGui::GetColorU32(knob.isActive    ? ImGuiCol_FrameBgActive
                                   : knob.isHovered ? ImGuiCol_FrameBgHovered
                                                    : ImGuiCol_FrameBg);
  ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddCircleFilled(knob.center, knob.layout.radius, col32, numSegments);
  draw_list->AddLine(knob.center, knob.pointerPosition, col32line, 1);
}

/**
 * implementation details for the Knob control
 * */
namespace detail {

struct KnobOutput
{
  KnobDrawData drawData;
  double outputValue;
  bool isActive;
};

inline KnobOutput
Knob(const char* name, const float inputValue, KnobLayout layout);
} // namespace detail

/**
 * Knob control associated with a plugin parameter
 * */
template<class Drawer>
inline bool
Knob(ParameterAccess& parameters, int parameterTag, KnobLayout layout, Drawer drawer)
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

  auto const output = detail::Knob(parameterName.c_str(), normalizedValue, layout);

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

/**
 * Knob control associated with a plugin parameter using the minimalistic drawer
 * */
bool
Knob(ParameterAccess& parameters, int parameterTag, KnobLayout layout);

} // namespace unplug
