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

#include "unplug/UserInterface.hpp"
#include "Parameters.hpp"
#include "imgui.h"
#include "unplug/Controls.hpp"

namespace unplug::UserInterface {

void
paint()
{
  using namespace unplug;

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

  ImGui::PushItemWidth(main_viewport->Size.x - 2 * ImGui::GetStyle().ItemSpacing.x);

  KnobWithLabels(ParamTag::gain, 2.f);

  DragFloat(ParamTag::gain);
  SliderFloat(ParamTag::gain);
  //    auto c = false;
  //    ImGui::ShowDemoWindow(&c);

  ImGui::End();
}

std::array<int, 2>
adjustSize(int width, int height, int prevWidth, int prevHeight)
{
  auto const referenceSize = getDefaultSize();
  auto const referenceWidth = static_cast<float>(referenceSize[0]);
  auto const referenceHeight = static_cast<float>(referenceSize[1]);
  auto const widthRatio = static_cast<float>(width) / referenceWidth;
  auto const heightRatio = static_cast<float>(height) / referenceHeight;
  auto const ratio = std::max(getMinZoom(), std::min(widthRatio, heightRatio));
  auto const adjustedWidth = static_cast<int>(ratio * referenceWidth);
  auto const adjustedHeight = static_cast<int>(ratio * referenceHeight);
  return { { adjustedWidth, adjustedHeight } };
}

bool
isResizingAllowed()
{
  return true;
}

std::array<int, 2>
getDefaultSize()
{
  return { { 200, 300 } };
}

float
getMinZoom()
{
  return 0.5f;
}

void
initializePersistentData(unplug::ViewPersistentData& persistentData)
{}

std::string
getWindowName()
{
  return "Demo";
}

std::array<float, 4>
getBackgroundColor()
{
  return { { 0, 0, 0, 1 } };
}

bool
getParameterAtCoordinates(int x, int y, int& parameterTag)
{
  return false;
}

} // namespace unplug::ui