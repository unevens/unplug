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
#include "unplug/PluginView.hpp"
#include <array>

namespace unplug {

class ImguiDemoPainter
{
public:
  explicit ImguiDemoPainter(PluginView<ImguiDemoPainter>& unplugView)
    : unplugView(unplugView)
  {}

  void paint()
  {
    bool keep_open = true;
    ImGui::ShowDemoWindow(&keep_open);
    // will not handle keep open
  }

  static void adjustSize(int& width, int& height) {}

  static bool isResizingAllowed() { return true; }

  static std::array<int, 2> getDefaultSize() { return { { 900, 700 } }; }

  // for pugl events whose responses may be implemented here
  template<class EventType>
  pugl::Status onEvent(EventType const& event) const noexcept
  {
    return pugl::Status::success;
  }

private:
  PluginView<ImguiDemoPainter>& unplugView;
};

using DemoView = PluginView<ImguiDemoPainter>;

} // namespace unplug