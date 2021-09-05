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

#include "unplug/detail/EventHandler.hpp"
#include "imgui_impl_opengl2.h"
#include "pugl/gl.hpp"
#include "unplug/UserInterface.hpp"
#include "unplug/detail/OpaqueGl.hpp"

namespace unplug::detail {

EventHandler::EventHandler(pugl::View& view, ParameterAccess& parameters, std::shared_ptr<MeterStorage>& meters)
  : view(view)
  , parameters(parameters)
  , meters(meters)
{}

void EventHandler::handleScroll(float dx, float dy)
{
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  io.MouseWheelH += dx;
  io.MouseWheel += dy;
  view.postRedisplay();
}

bool EventHandler::wantsCaptureKeyboard()
{
  setCurrentContext();
  return ImGui::GetIO().WantCaptureKeyboard;
}

void EventHandler::onAsciiKeyEvent(int key, bool isDown)
{
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  io.KeysDown[key] = isDown;
  if (isDown)
    io.AddInputCharacterUTF16(key);
}

void EventHandler::onNonAsciiKeyEvent(int virtualKeyCode, bool isDown)
{
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  io.KeysDown[virtualKeyCode + 128] = isDown;
  if (virtualKeyCode == ImGuiKey_Space && isDown)
    io.AddInputCharacter(' ');
}

void EventHandler::handleModifierKeys(ModifierKeys modifiers)
{
  ImGuiIO& io = ImGui::GetIO();
  io.KeyCtrl = modifiers.control;
  io.KeyShift = modifiers.shift;
  io.KeyAlt = modifiers.alt;
  io.KeySuper = modifiers.command;
}

pugl::Status EventHandler::onEvent(const pugl::CreateEvent& event)
{
  IMGUI_CHECKVERSION();
  if (imguiContext == nullptr) {
    imguiContext = ImGui::CreateContext();
    implotContext = ImPlot::CreateContext();
    setCurrentContext();
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

pugl::Status EventHandler::onEvent(const pugl::DestroyEvent& event)
{
  setCurrentContext();
  ImGui_ImplOpenGL2_Shutdown();
  ImGui::DestroyContext();
  view.stopTimer(redrawTimerId);
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::ConfigureEvent& event)
{
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = { (float)event.width, (float)event.height };
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::UpdateEvent& event)
{
  view.postRedisplay();
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::ExposeEvent& event)
{
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();

  // time
  using namespace std::chrono;
  auto time = clock::now();
  io.DeltaTime = duration_cast<duration<float>>(time - prevFrameTime).count();
  prevFrameTime = time;

  setCursor(io);

  ImGui_ImplOpenGL2_NewFrame();
  UserInterface::setupStyle();
  ImGui::NewFrame();
  parameters.clearParameterRectangles();
  const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_None);
  ImGui::SetNextWindowSize(main_viewport->Size, ImGuiCond_None);
  if (ImGui::Begin(UserInterface::getWindowName(), NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
    // do not paint if the windows is collapsed - will probably never happen with plugins
    UserInterface::paint();
  }
  ImGui::End();

  ImGui::Render();

  resizeAndClearViewport(io.DisplaySize.x, io.DisplaySize.y, UserInterface::getBackgroundColor());
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

  resetKeys();

  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::ButtonPressEvent& event)
{
  view.grabFocus();
  if (!isMouseCursorIn)
    return pugl::Status::failure;
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  auto imguiButtonCode = convertButtonCode(event.button);
  io.MouseDown[imguiButtonCode] = true;
  view.postRedisplay();
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::ButtonReleaseEvent& event)
{
  if (!isMouseCursorIn)
    return pugl::Status::failure;
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  auto imguiButtonCode = convertButtonCode(event.button);
  io.MouseDown[imguiButtonCode] = false;
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::MotionEvent& event)
{
  if (!isMouseCursorIn)
    return pugl::Status::failure;
  setCurrentContext();
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = { (float)event.x, (float)event.y };
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::ScrollEvent& event)
{
  handleScroll(static_cast<float>(event.dx), static_cast<float>(event.dy));
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::TimerEvent& event)
{
  if (event.id == redrawTimerId) {
    view.postRedisplay();
    return pugl::Status::success;
  }
  else {
    setCurrentContext();
    return pugl::Status::success;
  }
}

pugl::Status EventHandler::onEvent(const pugl::PointerInEvent& event)
{
  isMouseCursorIn = true;
  setCurrentContext();
  view.postRedisplay();
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::PointerOutEvent& event)
{
  isMouseCursorIn = false;
  setCurrentContext();
  view.postRedisplay();
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::KeyPressEvent& event)
{
  // this event should be left to the host, which will call IPluginView::onKeyDown
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::KeyReleaseEvent& event)
{
  // this event should be left to the host, which will call IPluginView::onKeyDown
  return pugl::Status::success;
}

pugl::Status EventHandler::onEvent(const pugl::TextEvent& event)
{
  // this event should be left to the host, which will call IPluginView::onKeyUp and IPluginView::onKeyUp
  return pugl::Status::success;
}

void EventHandler::setCurrentContext()
{
  assert(imguiContext);
  ImGui::SetCurrentContext(imguiContext);
  ImPlot::SetCurrentContext(implotContext);
  detail::setParameters(&parameters);
  if constexpr (NumMeters::value > 0) {
    detail::setMeters(meters.get());
  }
}

int EventHandler::convertButtonCode(int code)
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

void EventHandler::setCursor(const ImGuiIO& io)
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

void EventHandler::resetKeys()
{
  ImGuiIO& io = ImGui::GetIO();
  std::fill(std::begin(io.KeysDown), std::end(io.KeysDown), false);
}

} // namespace unplug::detail