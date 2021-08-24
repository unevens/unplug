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
    //    bool keep_open = true;
    //    ImGui::ShowDemoWindow(&keep_open);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui Demo", NULL, 0)) {
      // Early out if the window is collapsed, as an optimization.
      ImGui::End();
      return;
    }

    KnobLayout knobLayout;
    knobLayout.radius = 40;

    Knob(parameters, ParamTag::gain, knobLayout);

    ImGui::End();
  }

  static void adjustSize(int& width, int& height) {}

  static bool isResizingAllowed() { return true; }

  static std::array<int, 2> getDefaultSize() { return { { 900, 700 } }; }

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