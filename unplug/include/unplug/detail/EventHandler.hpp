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
#include "imgui_impl_opengl2.h"
#include "pugl/gl.hpp"
#include "Vst3Keycodes.hpp" //todo move the code that use this to Vst3View
#include <array>
#include <chrono>
#include "OpaqueGl.h"

namespace unplug {
namespace detail {

/**
 * The EventHandler class handles the events generated by the View class either through the Pugl view system or from
 * the VST3 SDK and expose the parameter API to the UserInterface - see the doc of the View class for a general
 * discussion. The EventHandler class implemented here assumes that the UserInterface uses the Dear Imgui library. If
 * you are familiar with Dear ImGui, think of the EventHandler class as a Dear ImGui backend.
 */

template<class UserInterface, class Parameters>
class EventHandler final
{
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

public:
  explicit EventHandler(pugl::View& view, Parameters& parameters)
    : view(view)
    , parameters(parameters)
    , ui(parameters)
  {}

  Parameters& getParameters() { return parameters; }

  static void adjustSize(int& width, int& height) { return UserInterface::adjustSize(width, height); }

  static bool isResizingAllowed() { return UserInterface::isResizingAllowed(); }

  static std::array<int, 2> getDefaultSize() { return UserInterface::getDefaultSize(); }

  void SetCurrentContext()
  {
    assert(imguiContext);
    ImGui::SetCurrentContext(imguiContext);
  }

  pugl::Status onEvent(const pugl::CreateEvent& event)
  {
    IMGUI_CHECKVERSION();
    if (imguiContext == nullptr) {
      imguiContext = ImGui::CreateContext();
      SetCurrentContext();
    }
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();

    io.BackendPlatformName = "imgui_impl_unplug_pugl";

    // the first 128 keys are ASCII, then the special characters from the ImGuiKey_ enum
    for (auto i = 0; i < ImGuiKey_COUNT; ++i) {
      io.KeyMap[i] = i + 128;
    }
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

#if defined(_WIN32)
    io.ImeWindowHandle = (void*)view.nativeWindow();
    io.ClipboardUserData = io.ImeWindowHandle;
#endif

    // todo maybe put a wrapper around puglSetClipboard in io.SetClipboardTextFn. (and same thing for the getter)

    ImGui_ImplOpenGL2_Init();

    prevFrameTime = clock::now();
    lastCursor = -1;

    view.startTimer(redrawTimerId, 1.0 / 60.0);

    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::DestroyEvent& event)
  {
    SetCurrentContext();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui::DestroyContext();
    view.stopTimer(redrawTimerId);
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ConfigureEvent& event)
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = { (float)event.width, (float)event.height };
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::UpdateEvent& event)
  {
    view.postRedisplay();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ExposeEvent& event)
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();

    // time
    using namespace std::chrono;
    auto time = clock::now();
    io.DeltaTime = duration_cast<duration<float>>(time - prevFrameTime).count();
    prevFrameTime = time;

    SetCursor(io);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui::NewFrame();
    ui.paint();
    ImGui::Render();
    resizeAndClearViewport(io.DisplaySize.x, io.DisplaySize.y);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ButtonPressEvent& event)
  {
    if (!isMouseCursorIn)
      return pugl::Status::failure;
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    auto imguiButtonCode = convertButtonCode(event.button);
    io.MouseDown[imguiButtonCode] = true;
    view.postRedisplay();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ButtonReleaseEvent& event)
  {
    if (!isMouseCursorIn)
      return pugl::Status::failure;
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    auto imguiButtonCode = convertButtonCode(event.button);
    io.MouseDown[imguiButtonCode] = false;
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::MotionEvent& event)
  {
    if (!isMouseCursorIn)
      return pugl::Status::failure;
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = { (float)event.x, (float)event.y };
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ScrollEvent& event)
  {
    // vertical scroll should be handled by the host, horizontal scroll is handled here
    handleScroll(static_cast<float>(event.dx), static_cast<float>(event.dy));
    return pugl::Status::success;
  }

  void handleScroll(float dx, float dy)
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += dx;
    io.MouseWheel += dy;
    view.postRedisplay();
  }

  // todo parts of this method should be moved into the View class to decouple this code form the VST3 SDK
  Steinberg::tresult onKeyEvent(Steinberg::char16 key,
                                Steinberg::int16 keyMsg,
                                Steinberg::int16 modifiersMask,
                                bool isDown)
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard)
      return Steinberg::kResultFalse;

    auto const modifiers = modifierKeysFromBitmask(modifiersMask);
    bool const isAscii = key > 0;
    if (isAscii) {
      io.KeysDown[key] = isDown;
      if (isDown)
        io.AddInputCharacterUTF16(key);
      handleModifierKeys(modifiers);
      view.postRedisplay();
      return Steinberg::kResultTrue;
    }
    else { // not ascii
      auto const numPadKeyCode = convertNumPadKeyCode(keyMsg);
      if (numPadKeyCode > -1) {
        io.KeysDown[numPadKeyCode] = isDown;
        if (isDown)
          io.AddInputCharacter(numPadKeyCode);
        handleModifierKeys(modifiers);
        view.postRedisplay();
        return Steinberg::kResultTrue;
      }
      else { // not ASCII, not num pad
        auto const virtualKeyCode = convertVirtualKeyCode(keyMsg);
        if (virtualKeyCode > -1) {
          io.KeysDown[virtualKeyCode + 128] = isDown;
          if (virtualKeyCode == ImGuiKey_Space && isDown) {
            io.AddInputCharacter(' ');
          }
          handleModifierKeys(modifiers);
          view.postRedisplay();
          return Steinberg::kResultTrue;
        }
        else { // not ASCII, not num pad, not special key
          handleModifierKeys(modifiers);
          view.postRedisplay();
          return Steinberg::kResultTrue;
        }
      }
    }
  }

  // Pugl events that may be handled by the UserInterface

  pugl::Status onEvent(const pugl::TimerEvent& event)
  {
    if (event.id == redrawTimerId) {
      view.postRedisplay();
      return pugl::Status::success;
    }
    else {
      SetCurrentContext();
      return ui.onEvent(event);
    }
  }

  pugl::Status onEvent(const pugl::PointerInEvent& event)
  {
    isMouseCursorIn = true;
    SetCurrentContext();
    view.postRedisplay();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::PointerOutEvent& event)
  {
    isMouseCursorIn = false;
    SetCurrentContext();
    view.postRedisplay();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::CloseEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::MapEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::UnmapEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::FocusInEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::FocusOutEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::LoopEnterEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::LoopLeaveEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  pugl::Status onEvent(const pugl::ClientEvent& event)
  {
    SetCurrentContext();
    return ui.onEvent(event);
  }

  // Pugl events that should not be dispatched

  pugl::Status onEvent(const pugl::KeyPressEvent& event)
  {
    // this event should be left to the host, which will call IPluginView::onKeyDown
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::KeyReleaseEvent& event)
  {
    // this event should be left to the host, which will call IPluginView::onKeyDown
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::TextEvent& event)
  {
    // this event should be left to the host, which will call IPluginView::onKeyUp and IPluginView::onKeyUp
    return pugl::Status::success;
  }

private:
  void handleModifierKeys(ModifierKeys modifiers)
  {
    using namespace Steinberg;
    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = modifiers.control;
    io.KeyShift = modifiers.shift;
    io.KeyAlt = modifiers.alt;
  }

  int convertButtonCode(int code)
  {
    switch (code) {
      case 1: // left
        return 0;
      case 2: // center
        return 2;
      case 3: // right
        return 1;
      default: // extra buttons, unused
        return code - 1;
    }
  }

  void SetCursor(ImGuiIO const& io)
  {
    ImGuiMouseCursor cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (lastCursor == cursor)
      return;
    lastCursor = cursor;
    switch (cursor) {
      case ImGuiMouseCursor_None:
        puglSetCursor(view.cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_Arrow:
        puglSetCursor(view.cobj(), PUGL_CURSOR_ARROW);
        break;
      case ImGuiMouseCursor_TextInput:
        puglSetCursor(view.cobj(), PUGL_CURSOR_CARET);
        break;
      case ImGuiMouseCursor_ResizeAll:
        puglSetCursor(view.cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_ResizeNS:
        puglSetCursor(view.cobj(), PUGL_CURSOR_UP_DOWN);
        break;
      case ImGuiMouseCursor_ResizeEW:
        puglSetCursor(view.cobj(), PUGL_CURSOR_LEFT_RIGHT);
        break;
      case ImGuiMouseCursor_ResizeNESW:
        puglSetCursor(view.cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_ResizeNWSE:
        puglSetCursor(view.cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_Hand:
        puglSetCursor(view.cobj(), PUGL_CURSOR_HAND);
        break;
      case ImGuiMouseCursor_NotAllowed:
        puglSetCursor(view.cobj(), PUGL_CURSOR_NO);
        break;
      default:
        break;
    }
  }

private:
  Parameters& parameters;
  pugl::View& view;
  ImGuiContext* imguiContext = nullptr;
  UserInterface ui;
  time_point prevFrameTime;
  ImGuiMouseCursor lastCursor = -1;
  bool isMouseCursorIn = false;
  static constexpr uintptr_t redrawTimerId = 1;
};

} // namespace detail
} // namespace unplug