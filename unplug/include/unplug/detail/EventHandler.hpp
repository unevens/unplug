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
#include "pluginterfaces/base/keycodes.h"
#include "unplug/detail/View.hpp"
#include <array>
#include <chrono>

namespace unplug {

struct ModifierKeys
{
  bool shift = false;
  bool alt = false;
  bool control = false;
};

namespace detail {

/**
 * The EventHandler class handles the events generated by the plugin view or the host using Dear ImGui.
 * If you are familiar with Dear ImGui, think of this class as a Dear ImGui backend.
 * The specific UI code of your plugin should be implemented by a Painter class, which will then by injected here as a
 * template argument (see the files unplug/PluginView.hpp and unplug/DemoView.hpp).
 */

template<class Painter>
class EventHandler final
{
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;
  using char16 = Steinberg::char16;
  using int16 = Steinberg::int16;
  using tresult = Steinberg::tresult;
  static constexpr auto kResultTrue = Steinberg::kResultTrue;
  static constexpr auto kResultFalse = Steinberg::kResultFalse;

public:
  explicit EventHandler(View<EventHandler<Painter>>& vstView)
    : vstView(vstView)
    , painter(vstView)
  {}

  static void adjustSize(int& width, int& height) { return Painter::adjustSize(width, height); }

  static bool isResizingAllowed() { return Painter::isResizingAllowed(); }

  static std::array<int, 2> getDefaultSize() { return Painter::getDefaultSize(); }

  void SetCurrentContext()
  {
    assert(imguiContext);
    ImGui::SetCurrentContext(imguiContext);
  }

  pugl::Status onEvent(const pugl::TimerEvent& event)
  {
    if (event.id == redrawTimerId) {
      getPuglView().postRedisplay();
      return pugl::Status::success;
    }
    else {
      SetCurrentContext();
      return painter.onEvent(event);
    }
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
    io.ImeWindowHandle = (void*)getPuglView().nativeWindow();
    io.ClipboardUserData = io.ImeWindowHandle;
#endif

    // todo maybe put a wrapper around puglSetClipboard in io.SetClipboardTextFn. (and same thing for the getter)

    ImGui_ImplOpenGL2_Init();

    prevFrameTime = clock::now();
    lastCursor = -1;

    getPuglView().startTimer(redrawTimerId, 1.0 / 60.0);

    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::DestroyEvent& event)
  {
    SetCurrentContext();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui::DestroyContext();
    getPuglView().stopTimer(redrawTimerId);
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
    getPuglView().postRedisplay();
    return pugl::Status::success;
  }

  void SetCursor(ImGuiIO const& io)
  {
    ImGuiMouseCursor cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (lastCursor == cursor)
      return;
    lastCursor = cursor;
    switch (cursor) {
      case ImGuiMouseCursor_None:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_Arrow:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_ARROW);
        break;
      case ImGuiMouseCursor_TextInput:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_CARET);
        break;
      case ImGuiMouseCursor_ResizeAll:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_ResizeNS:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_UP_DOWN);
        break;
      case ImGuiMouseCursor_ResizeEW:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_LEFT_RIGHT);
        break;
      case ImGuiMouseCursor_ResizeNESW:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_ResizeNWSE:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_CROSSHAIR);
        break;
      case ImGuiMouseCursor_Hand:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_HAND);
        break;
      case ImGuiMouseCursor_NotAllowed:
        puglSetCursor(getPuglView().cobj(), PUGL_CURSOR_NO);
        break;
      default:
        break;
    }
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
    painter.paint();
    ImGui::Render();
    glViewport(vstView.getRect().left,
               vstView.getRect().top,
               vstView.getRect().right - vstView.getRect().left,
               vstView.getRect().bottom - vstView.getRect().top);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
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
    getPuglView().postRedisplay();
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
    getPuglView().postRedisplay();
  }

  tresult onKeyEvent(char16 key, int16 keyMsg, int16 modifiersMask, bool isDown)
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard)
      return kResultFalse;

    auto const modifiers = modifierKeysFromBitmask(modifiersMask);
    bool const isAscii = key > 0;
    if (isAscii) {
      io.KeysDown[key] = isDown;
      if (isDown)
        io.AddInputCharacterUTF16(key);
      handleModifierKeys(modifiers);
      getPuglView().postRedisplay();
      return kResultTrue;
    }
    else { // not ascii
      auto const numPadKeyCode = convertNumPadKeyCode(keyMsg);
      if (numPadKeyCode > -1) {
        io.KeysDown[numPadKeyCode] = isDown;
        if (isDown)
          io.AddInputCharacter(numPadKeyCode);
        handleModifierKeys(modifiers);
        getPuglView().postRedisplay();
        return kResultTrue;
      }
      else { // not ASCII, not num pad
        auto const virtualKeyCode = convertVirtualKeyCode(keyMsg);
        if (virtualKeyCode > -1) {
          io.KeysDown[virtualKeyCode + 128] = isDown;
          if (virtualKeyCode == ImGuiKey_Space && isDown) {
            io.AddInputCharacter(' ');
          }
          handleModifierKeys(modifiers);
          getPuglView().postRedisplay();
          return kResultTrue;
        }
        else { // not ASCII, not num pad, not special key
          handleModifierKeys(modifiers);
          getPuglView().postRedisplay();
          return kResultTrue;
        }
      }
    }
  }

private:
  ModifierKeys modifierKeysFromBitmask(int16 mask)
  {
    ModifierKeys modifierKeys;
    modifierKeys.shift = mask & Steinberg::kShiftKey;
    modifierKeys.alt = mask & Steinberg::kAlternateKey;
    modifierKeys.control = (mask & Steinberg::kCommandKey) || (mask & Steinberg::kControlKey);
    return modifierKeys;
  }

  void handleModifierKeys(ModifierKeys modifiers)
  {
    using namespace Steinberg;
    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = modifiers.control;
    io.KeyShift = modifiers.shift;
    io.KeyAlt = modifiers.alt;
  }

  int convertVirtualKeyCode(int16 virtualKeyCode)
  {
    using namespace Steinberg;
    switch (virtualKeyCode) {
      case KEY_BACK:
        return ImGuiKey_Backspace;
      case KEY_TAB:
        return ImGuiKey_Tab;
      case KEY_RETURN:
        return ImGuiKey_Enter;
      case KEY_ESCAPE:
        return ImGuiKey_Escape;
      case KEY_SPACE:
        return ImGuiKey_Space;
      case KEY_END:
        return ImGuiKey_End;
      case KEY_HOME:
        return ImGuiKey_Home;
      case KEY_LEFT:
        return ImGuiKey_LeftArrow;
      case KEY_UP:
        return ImGuiKey_UpArrow;
      case KEY_RIGHT:
        return ImGuiKey_RightArrow;
      case KEY_DOWN:
        return ImGuiKey_DownArrow;
      case KEY_PAGEUP:
        return ImGuiKey_PageUp;
      case KEY_PAGEDOWN:
        return ImGuiKey_PageDown;
      case KEY_ENTER:
        return ImGuiKey_KeyPadEnter;
      case KEY_INSERT:
        return ImGuiKey_Insert;
      case KEY_DELETE:
        return ImGuiKey_Delete;
      default:
        return -1;
    }
  }

  int convertNumPadKeyCode(int16 virtualKeyCode)
  {
    using namespace Steinberg;
    switch (virtualKeyCode) {
      case KEY_NUMPAD0:
        return '0';
      case KEY_NUMPAD1:
        return '1';
      case KEY_NUMPAD2:
        return '2';
      case KEY_NUMPAD3:
        return '3';
      case KEY_NUMPAD4:
        return '4';
      case KEY_NUMPAD5:
        return '5';
      case KEY_NUMPAD6:
        return '6';
      case KEY_NUMPAD7:
        return '7';
      case KEY_NUMPAD8:
        return '8';
      case KEY_NUMPAD9:
        return '9';
      case KEY_MULTIPLY:
        return '*';
      case KEY_ADD:
        return '+';
      case KEY_SUBTRACT:
        return '-';
      case KEY_DIVIDE:
        return '/';
      case KEY_DECIMAL:
        return '.';
      default:
        return -1;
    }
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

public:
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

  // Pugl events that may be handled by the Painter

  pugl::Status onEvent(const pugl::PointerInEvent& event)
  {
    SetCurrentContext();
    isMouseCursorIn = true;
    getPuglView().postRedisplay();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::PointerOutEvent& event)
  {
    SetCurrentContext();
    isMouseCursorIn = false;
    getPuglView().postRedisplay();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::CloseEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::MapEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::UnmapEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::FocusInEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::FocusOutEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::LoopEnterEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::LoopLeaveEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }

  pugl::Status onEvent(const pugl::ClientEvent& event)
  {
    SetCurrentContext();
    return painter.onEvent(event);
  }
  
private:
  View<EventHandler<Painter>>& vstView;
  pugl::View& getPuglView() { return *vstView.getPuglView(); }
  ImGuiContext* imguiContext = nullptr;
  Painter painter;
  time_point prevFrameTime;
  ImGuiMouseCursor lastCursor = -1;
  bool isMouseCursorIn = false;
  static constexpr uintptr_t redrawTimerId = 1;
};

} // namespace detail
} // namespace unplug