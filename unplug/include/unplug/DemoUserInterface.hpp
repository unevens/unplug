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
#include "pugl/pugl.hpp"
#include "unplug/Controls.hpp"
#include "unplug/ParameterAccess.hpp"
#include <array>

namespace unplug {

/**
 * Just an hello world user interface that shows the Dear ImGui demo
 * */

class DemoUserInterface final
{
public:
  using ParameterAccess = unplug::ParameterAccess;

  explicit DemoUserInterface(ParameterAccess& parameters)
    : parameters(parameters)
  {}

  void paint()
  {

    // ImGui default style can be editer directly before calling ImGui::NewFrame();

    ImGui::NewFrame();

    // use PushStyleColor or PushStyleVar (and the corresponding Pop calls) to edit the ImGui style after
    // ImGui::NewFrame() has been called

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_None);
    ImGui::SetNextWindowSize(main_viewport->Size, ImGuiCond_None);

    if (!ImGui::Begin("UnPlugDemo", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
      // Early out if the window is collapsed, as an optimization.
      ImGui::End();
      return;
    }

    KnobLayout knobLayout;
    knobLayout.radius = 40;

    KnobWithLabels(parameters, ParamTag::gain, knobLayout);

    ImGui::End();
  }

  static void adjustSize(int& width, int& height) {
    width = std::max(width, 100);
    height = std::max(height, 200);
  }

  static bool isResizingAllowed() { return true; }

  static std::array<int, 2> getDefaultSize() { return { { 300, 300 } }; }

  static void initializePersistentData(ViewPersistentData& presistentData) {}

  static std::string getWindowName() { return "Demo"; }

  std::array<float, 4> getBackgroundColor() const { return { { 0, 0, 0, 1 } }; }

  bool getParameterAtCoordinates(int x, int y, int& parameterTag) const { return false; }

  // for pugl events whose responses may be implemented here
  template<class EventType>
  pugl::Status onEvent(EventType const& event) const noexcept
  {
    return pugl::Status::success;
  }

private:
  ParameterAccess& parameters;
};

} // namespace unplug