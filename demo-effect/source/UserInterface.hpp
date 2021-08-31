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
#include "pugl/pugl.hpp"
#include "unplug/ViewPersistentData.hpp"
#include <array>

class DemoEffectUserInterface final
{
public:
  void paint();

  static void adjustSize(int& width, int& height, int prevWidth, int prevHeight);

  static bool isResizingAllowed() { return true; }

  static std::array<int, 2> getDefaultSize() { return { { 200, 300 } }; }

  static float getMinZoom() { return 0.5f; }

  static void initializePersistentData(unplug::ViewPersistentData& presistentData) {}

  static std::string getWindowName() { return "Demo"; }

  std::array<float, 4> getBackgroundColor() const { return { { 0, 0, 0, 1 } }; }

  bool getParameterAtCoordinates(int x, int y, int& parameterTag) const { return false; }

  // for pugl events whose responses may be implemented here
  template<class EventType>
  pugl::Status onEvent(EventType const& event) const noexcept
  {
    return pugl::Status::success;
  }
};