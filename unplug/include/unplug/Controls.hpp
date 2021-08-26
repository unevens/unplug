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

/**
 * Combo ImGui control associated with a plugin parameter
 * */
bool
Combo(int parameterTag);

/**
 * Checkbox ImGui control associated with a plugin parameter
 * */
bool
Checkbox(int parameterTag);

void
TextCentered(std::string const& text, ImVec2 size);

void
Label(int parameterTag);

void
LabelCentered(int parameterTag, ImVec2 size);

void
ValueAsText(int parameterTag);

void
ValueAsTextCentered(int parameterTag, ImVec2 size);

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
void
DrawSimpleKnob(KnobDrawData const& knob);

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

KnobOutput
Knob(const char* name, float inputValue, KnobLayout layout);
} // namespace detail

/**
 * Knob control associated with a plugin parameter
 * */
bool
Knob(int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> drawer = DrawSimpleKnob);

bool
KnobWithLabels(int parameterTag, KnobLayout layout, std::function<void(KnobDrawData const&)> drawer = DrawSimpleKnob);

} // namespace unplug
