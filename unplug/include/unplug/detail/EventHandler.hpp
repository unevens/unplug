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
#include "View.hpp"
#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "unplug/detail/View.hpp"
#include <chrono>

namespace unplug {
namespace detail {

/**
 * The EventHandler class handles the events generated by the plugin view or the host using Dear ImGui.
 * If you are familiar with Dear ImGui, think of this class as a Dear ImGui backend.
 * The specific UI code of your plugin should be implemented by a Painter class, which will then by injected here as a
 * template argument (see the files unplug/PluginView.hpp and unplug/DemoView.hpp).
 */

template<class Painter>
class EventHandler
{
protected:
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

public:
  explicit EventHandler(View<EventHandler<Painter>>& vstView)
    : vstView(vstView)
    , painter(vstView)
  {}

  static bool isSizeSupported(int width, int height) {
    return Painter::isSizeSupported(width, height);
  }

  static bool isResizingAllowed() {
    return Painter::isResizingAllowed();
  }

  void SetCurrentContext()
  {
    assert(imguiContext);
    ImGui::SetCurrentContext(imguiContext);
  }

  pugl::Status onEvent(const pugl::CreateEvent& event) noexcept
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

    io.KeyMap[ImGuiKey_LeftArrow] = PUGL_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = PUGL_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = PUGL_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = PUGL_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = PUGL_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = PUGL_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = PUGL_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = PUGL_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = PUGL_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = PUGL_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = PUGL_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Escape] = PUGL_KEY_ESCAPE;
    //missing from pugl:
#if defined _WIN32
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
#else
// todo
#endif

//    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
//    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
//    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
//    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
//    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
//    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
//    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
//    io.KeyMap[ImGuiKey_Home] = VK_HOME;
//    io.KeyMap[ImGuiKey_End] = VK_END;
//    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
//    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
//    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
//    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
//    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
//    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
//    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
//    io.KeyMap[ImGuiKey_A] = 'A';
//    io.KeyMap[ImGuiKey_C] = 'C';
//    io.KeyMap[ImGuiKey_V] = 'V';
//    io.KeyMap[ImGuiKey_X] = 'X';
//    io.KeyMap[ImGuiKey_Y] = 'Y';
//    io.KeyMap[ImGuiKey_Z] = 'Z';


#if defined(_WIN32)
    io.ImeWindowHandle = (void*)getPuglView().nativeWindow();
    io.ClipboardUserData = io.ImeWindowHandle;
#endif

    // todo maybe put a wrapper around puglSetClipboard in io.SetClipboardTextFn. (and same thing for the getter)

    ImGui_ImplOpenGL2_Init();

    prevFrameTime = clock::now();
    lastCursor = -1;

    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::DestroyEvent& event) noexcept
  {
    SetCurrentContext();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui::DestroyContext();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ConfigureEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = { (float)event.width, (float)event.height };
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::UpdateEvent& event) noexcept
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

  pugl::Status onEvent(const pugl::ExposeEvent& event) noexcept
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

  void UpdateModifierKeys(ImGuiIO& io)
  {
    io.KeyCtrl = io.KeysDown[PUGL_KEY_CTRL_L] || io.KeysDown[PUGL_KEY_CTRL_R];
    io.KeyShift = io.KeysDown[PUGL_KEY_SHIFT_L] || io.KeysDown[PUGL_KEY_SHIFT_R];
    io.KeyAlt = io.KeysDown[PUGL_KEY_MENU];
  }

  pugl::Status onEvent(const pugl::KeyPressEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[event.key] = true;
    UpdateModifierKeys(io);
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::KeyReleaseEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[event.key] = false;
    UpdateModifierKeys(io);
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::TextEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(event.character);
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ButtonPressEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[event.button] = true;
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ButtonReleaseEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[event.button] = false;
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::MotionEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = { (float)event.x, (float)event.y };
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ScrollEvent& event) noexcept
  {
    SetCurrentContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)event.dx;
    io.MouseWheel += (float)event.dy;
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::PointerInEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::PointerOutEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::CloseEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::MapEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::UnmapEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::FocusInEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::FocusOutEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::TimerEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::LoopEnterEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::LoopLeaveEvent& event) noexcept { return pugl::Status::success; }

  pugl::Status onEvent(const pugl::ClientEvent& event) noexcept { return pugl::Status::success; }

protected:
  View<EventHandler<Painter>>& vstView;
  pugl::View& getPuglView() { return *vstView.getPuglView(); }
  ImGuiContext* imguiContext = nullptr;
  Painter painter;
  time_point prevFrameTime;
  ImGuiMouseCursor lastCursor = -1;
};

} // namespace detail
} // namespace unplug