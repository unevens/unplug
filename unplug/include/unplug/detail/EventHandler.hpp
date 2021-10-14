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
#include "SharedData.hpp"
#include "imgui.h"
#include "implot.h"
#include "pugl/pugl.hpp"
#include "unplug/MeterStorage.hpp"
#include "unplug/ParameterAccess.hpp"
#include "unplug/detail/ModifierKeys.hpp"
#include <array>
#include <chrono>
#include <memory>

namespace unplug::detail {

/**
 * The EventHandler class handles the events generated by the View class either through the Pugl view system or from
 * the VST3 SDK and expose the parameter API to the UserInterface - see the doc of the View class for a general
 * discussion. The EventHandler class implemented here assumes that the UserInterface uses the Dear ImGui library. If
 * you are familiar with Dear ImGui, think of the EventHandler class as a Dear ImGui backend.
 */

class EventHandler final
{
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

public:
  EventHandler(pugl::View& view,
               ParameterAccess& parameters,
               std::shared_ptr<MeterStorage>& meters,
               std::shared_ptr<SharedDataWrapped>& custom);

  void handleScroll(float dx, float dy);

  bool wantsCaptureKeyboard();

  void onAsciiKeyEvent(int key, bool isDown);

  void onNonAsciiKeyEvent(int virtualKeyCode, bool isDown);

  void handleModifierKeys(ModifierKeys modifiers);

  pugl::Status onEvent(const pugl::CreateEvent& event);

  pugl::Status onEvent(const pugl::DestroyEvent& event);

  pugl::Status onEvent(const pugl::ConfigureEvent& event);

  pugl::Status onEvent(const pugl::UpdateEvent& event);

  pugl::Status onEvent(const pugl::ExposeEvent& event);

  pugl::Status onEvent(const pugl::ButtonPressEvent& event);

  pugl::Status onEvent(const pugl::ButtonReleaseEvent& event);

  pugl::Status onEvent(const pugl::MotionEvent& event);

  pugl::Status onEvent(const pugl::ScrollEvent& event);

  // Pugl events that may be handled by the UserInterface

  pugl::Status onEvent(const pugl::TimerEvent& event);

  pugl::Status onEvent(const pugl::PointerInEvent& event);

  pugl::Status onEvent(const pugl::PointerOutEvent& event);

  // these text key related Pugl events are left to the host on Windows, and are never called on macOS

  pugl::Status onEvent(const pugl::KeyPressEvent& event);

  pugl::Status onEvent(const pugl::KeyReleaseEvent& event);

  pugl::Status onEvent(const pugl::TextEvent& event);

  // for pugl events whose responses are not implemented
  template<class EventType>
  pugl::Status onEvent(EventType const& event) const noexcept
  {
    return pugl::Status::success;
  }

private:
  void setCurrentContext();

  int convertButtonCode(int code);

  void setCursor(ImGuiIO const& io);

  void resetKeys();

private:
  ParameterAccess& parameters;
  std::shared_ptr<MeterStorage>& meters;
  std::shared_ptr<SharedDataWrapped>& custom;
  pugl::View& view;
  ImGuiContext* imguiContext = nullptr;
  ImPlotContext* implotContext = nullptr;
  time_point prevFrameTime;
  ImGuiMouseCursor lastCursor = -1;
  bool isMouseCursorIn = false;
  static constexpr uintptr_t redrawTimerId = 1;
};

} // namespace unplug::detail