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
#include "unplug/detail/View.hpp"

namespace unplug {
namespace detail {

template<class Painter>
class EventHandler
{

public:
  EventHandler(View<EventHandler<Painter>>& vstView)
    : vstView(vstView)
    , painter(vstView)
  {}

  template<PuglEventType t, class Base>
  pugl::Status onEvent(const pugl::Event<t, Base>&) noexcept
  {
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::CreateEvent& event) noexcept
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    // todo: smt as ImGui_ImplGlfw_InitForOpenGL(getPuglView(), true);
    ImGui_ImplOpenGL2_Init();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::DestroyEvent& event) noexcept
  {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui::DestroyContext();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ConfigureEvent& event) noexcept
  {
    using namespace Steinberg;
    auto newSize = ViewRect{ 0, 0, static_cast<int32>(event.width), static_cast<int32>(event.height) };
    vstView.setRect(newSize);
    // maybe more?
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::UpdateEvent& event) noexcept
  {
    getPuglView().postRedisplay();
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::ExposeEvent& event) noexcept
  {

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

  pugl::Status onEvent(const pugl::KeyPressEvent& event) noexcept
  {
    /**
     * todo in vst3 key events should be left to the host.
     * gotta see how that works with Pugl
     * */
    return pugl::Status::success;
  }

  pugl::Status onEvent(const pugl::CloseEvent& event) noexcept
  {
    //?
    return pugl::Status::success;
  }

protected:
  View<EventHandler<Painter>>& vstView;
  pugl::View& getPuglView() { return *vstView.getPuglView(); }
  Painter painter;
};

} // namespace detail
} // namespace unplug