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
#include "imgui_impl_opengl2.h" //todo or maybe 3?
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pugl/gl.hpp"
#include "pugl/pugl.h"
#include "pugl/pugl.hpp"
#include <cassert>
#include <functional>
#include <memory>

namespace unplug {
namespace detail {

template<class EventHandler>
class View final : public Steinberg::CPluginView
{
public:
  using tresult = Steinberg::tresult;
  using FIDString = Steinberg::FIDString;
  using ViewRect = Steinberg::ViewRect;
  using char16 = Steinberg::char16;
  using int16 = Steinberg::int16;

  explicit View(const char* className)
    : world{ pugl::WorldType::module }
  {
    world.setClassName(className);
  }

  ~View() {}

  pugl::View* getPuglView() { return puglView.get(); }

  tresult PLUGIN_API attached(void* pParent, FIDString type) override
  {
    CPluginView::attached(pParent, type);
    puglView = std::make_unique<pugl::View>(world);
    eventHandler = std::make_unique<EventHandler>(*this);
    puglView->setEventHandler(*eventHandler);
    puglView->setParentWindow((pugl::NativeView)pParent);
    puglView->setWindowTitle("Pugl C++ Test");
    puglView->setDefaultSize(512, 512);
    puglView->setMinSize(64, 64);
    puglView->setMaxSize(1024, 1024);
    puglView->setAspectRatio(1, 1, 16, 9);
    puglView->setBackend(pugl::glBackend());
    puglView->setHint(pugl::ViewHint::resizable, true);
    pugl::Status status = puglView->realize();
    if (status != pugl::Status::success) {
      assert(false);
      return kResultFalse;
    }
    puglView->show();
    return kResultOk;
  }

  tresult PLUGIN_API removed() override
  {
    puglView.reset(nullptr);
    eventHandler.reset(nullptr);
    return CPluginView::removed();
  }

  tresult PLUGIN_API onSize(ViewRect* newSize) override
  {
    CPluginView::onSize(newSize);
    // todo inform puglView? ... emit event?
    return kResultOk;
  }

  tresult PLUGIN_API isPlatformTypeSupported(FIDString type) override
  {
    if (strcmp(type, kPlatformTypeHWND) == 0) {
      return kResultTrue;
    }
    if (strcmp(type, kPlatformTypeHIView) == 0) {
      // todo The parent is a WindowRef, should attach a HIViewRef to it.
      return kResultTrue;
    }
    if (strcmp(type, kPlatformTypeNSView) == 0) {
      // todo The parent is a NSView pointer. should attach a NSView to it.
      return kResultTrue;
    }
    if (strcmp(type, kPlatformTypeX11EmbedWindowID) == 0) {
      // todo The parent is a X11 Window supporting XEmbed.
      return kResultTrue;
    }

    return kResultFalse;
  }

  tresult PLUGIN_API canResize() override { return kResultTrue; }

  tresult PLUGIN_API checkSizeConstraint(ViewRect* /*rect*/) override
  {
    // todo, ask puglView?
    return kResultTrue;
  }

  // todo: send Pugl events?
  tresult PLUGIN_API onWheel(float /*distance*/) override { return kResultFalse; }
  tresult PLUGIN_API onKeyDown(char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) override { return kResultFalse; }
  tresult PLUGIN_API onKeyUp(char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) override { return kResultFalse; }

private:
  pugl::World world;
  std::unique_ptr<pugl::View> puglView;
  std::unique_ptr<EventHandler> eventHandler;
};

} // namespace detail
} // namespace unplug