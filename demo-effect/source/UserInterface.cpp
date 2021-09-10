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
#include "CircularBuffers.hpp"
#include "Meters.hpp"
#include "Parameters.hpp"
#include "unplug/Widgets.hpp"

namespace unplug::UserInterface {

void paint()
{
  auto const main_viewport = ImGui::GetMainViewport();
  auto const width = main_viewport->Size.x - 2 * ImGui::GetStyle().ItemSpacing.x;
  ImGui::PushItemWidth(width);

  KnobWithLabels(Param::gain);
  DragFloat(Param::gain);
  SliderFloat(Param::gain);
  MeterValueLabelCentered(Meter::level);

  auto const differenceLevelMeterSettings = DifferenceLevelMeterSettings{};
  DifferenceLevelMeter(Meter::level, "LevelMeter", { width, 50.f }, differenceLevelMeterSettings);

  auto levelMeterSettings = LevelMeterSettings{};
  LevelMeter(Meter::level, "LevelMeter1", { width, 50.f }, levelMeterSettings, LevelMeterAlign::toMinValue);
  levelMeterSettings.maxValueColor = ImVec4{ 0.f, 0.f, 1.f, 1.f };
  LevelMeter(Meter::level, "LevelMeter2", { width, 50.f }, levelMeterSettings, LevelMeterAlign::toMaxValue);
}

std::array<int, 2> getDefaultSize()
{
  return { { 300, 700 } };
}

bool isResizingAllowed()
{
  return true;
}

float getMinZoom()
{
  return 1.f;
}

bool keepDefaultRatio()
{
  return true;
}

const char* getWindowName()
{
  return "Unplug Demo Gain";
}

std::array<float, 3> getBackgroundColor()
{
  return { { 0, 0, 0 } };
}

void setupStyle() {}

void adjustSize(int& width, int& height, int prevWidth, int prevHeight) {}

void initializePersistentData(unplug::ViewPersistentData& persistentData) {}

float getRefreshRate()
{
  return 1.f / 30.f;
}

} // namespace unplug::UserInterface