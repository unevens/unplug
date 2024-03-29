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
#include "Meters.hpp"
#include "Parameters.hpp"
#include "SharedData.hpp"
#include "unplug/Plot.hpp"
#include "unplug/Widgets.hpp"

namespace unplug::UserInterface {

void paint()
{
  auto const main_viewport = ImGui::GetMainViewport();
  auto const viewWidth = main_viewport->Size.x - 2 * ImGui::GetStyle().ItemSpacing.x;
  auto const levelMeterHeight = 24.f;
  auto const widgetWidth = std::min(viewWidth / 2, 300.f);
  auto& sharedData = SharedDataWrapped::getCurrent();

  ImGui::BeginGroup();
  ImGui::PushItemWidth(widgetWidth);
  KnobWithLabels(Param::gain);
  DragFloat(Param::gain);
  SliderFloat(Param::gain);
  MeterValueLabelCentered(Meter::level, "Level: ");
  LevelMeter(Meter::level, "LevelMeter", { widgetWidth, levelMeterHeight });
  Combo(Param::oversamplingOrder);
  Checkbox(Param::oversamplingLinearPhase);
  ImGui::EndGroup();

  ImGui::SameLine();

  ImGui::BeginGroup();
  ImGui::TableNextColumn();
  PlotRingBuffer("Level", sharedData.levelRingBuffer);
  PlotWaveformRingBuffer("Waveform", sharedData.waveformRingBuffer);
  ImGui::EndGroup();
}

std::array<int, 2> getDefaultSize()
{
  return { { 800, 620 } };
}

bool isResizingAllowed()
{
  return true;
}

float getMinZoomWithFixedRatio()
{
  return 1.f;
}

bool hasFixedRatio()
{
  return false;
}

const char* getWindowName()
{
  return "Unplug Gain Example";
}

std::array<float, 3> getBackgroundColor()
{
  return { { 0, 0, 0 } };
}

void setupStyle() {}

void adjustSize(int& width, int& height, int prevWidth, int prevHeight)
{
  width = std::max(width, 400);
  height = std::max(height, 400);
}

float getRefreshRate()
{
  return 1.f / 30.f;
}

} // namespace unplug::UserInterface