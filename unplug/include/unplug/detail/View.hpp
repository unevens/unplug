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
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pugl/gl.hpp"
#include <cassert>
#include <functional>
#include <memory>
#include <string>

namespace unplug {
namespace detail {

/**
 * The View class implements a Steinberg::CPluginView using a pugl::View.
 * It manages the lifecycle of the pugl::View has translates VST3 callbacks such as "onSize" and "onKeyDown" to Pugl
 * events.
 * The logic to handle the Pugl events is not implemented by this class, but by an EventHandler class, which is then
 * injected as a template argument (see the files unplug/PluginView.hpp and unplug/DemoView.hpp).
 * */

template<class EventHandler>
class View final : public Steinberg::CPluginView
{
public:
  using tresult = Steinberg::tresult;
  using FIDString = Steinberg::FIDString;
  using ViewRect = Steinberg::ViewRect;
  using char16 = Steinberg::char16;
  using int16 = Steinberg::int16;
  static constexpr auto kResultTrue = Steinberg::kResultTrue;
  static constexpr auto kResultFalse = Steinberg::kResultFalse;

  explicit View(const char* name)
    : world{ pugl::WorldType::module }
    , name(name)
  {
    world.setClassName(name);
  }

  ~View() = default;

  pugl::View* getPuglView() { return puglView.get(); }

  tresult PLUGIN_API attached(void* pParent, FIDString type) override
  {
    CPluginView::attached(pParent, type);
    puglView = std::make_unique<pugl::View>(world);
    eventHandler = std::make_unique<EventHandler>(*this);
    puglView->setEventHandler(*eventHandler);
    puglView->setParentWindow((pugl::NativeView)pParent);
    puglView->setWindowTitle(name.c_str());
    auto const defaultSize = EventHandler::getDefaultSize();
    puglView->setDefaultSize(defaultSize[0], defaultSize[1]);
    puglView->setAspectRatio(0, 0, 0, 0);
    puglView->setBackend(pugl::glBackend());
    puglView->setHint(pugl::ViewHint::resizable, true);
    puglView->setHint(pugl::ViewHint::samples, 0);
    puglView->setHint(pugl::ViewHint::doubleBuffer, true);
    puglView->setHint(pugl::ViewHint::ignoreKeyRepeat, true);
#ifdef _NDEBUG
    puglView->setHint(pugl::ViewHint::useDebugContext, false);
#else
    puglView->setHint(pugl::ViewHint::useDebugContext, true);
#endif
    puglView->setHint(pugl::ViewHint::contextVersionMajor, 2);
    puglView->setHint(pugl::ViewHint::contextVersionMinor, 0);
    puglView->setHint(pugl::ViewHint::useCompatProfile, true);
    pugl::Status status = puglView->realize();
    if (status != pugl::Status::success) {
      assert(false);
      return kResultFalse;
    }
    puglView->show();
    if (plugFrame){
      auto viewRect = ViewRect{ 0, 0, defaultSize[0], defaultSize[1] };
      plugFrame->resizeView(this, &viewRect);
    }
    return kResultTrue;
  }

  tresult PLUGIN_API removed() override
  {
    puglView.reset(nullptr);
    eventHandler.reset(nullptr);
    return CPluginView::removed();
  }

  tresult PLUGIN_API onSize(ViewRect* r) override
  {
    if (puglView) {
      puglView->setFrame({ (double)r->left, (double)r->top, (double)r->getWidth(), (double)r->getHeight() });
      puglView->postRedisplay();
    }
    return CPluginView::onSize(r);
  }

  tresult PLUGIN_API isPlatformTypeSupported(FIDString type) override
  {
    using namespace Steinberg;

    if (strcmp(type, kPlatformTypeHWND) == 0)
      return kResultTrue;

    if (strcmp(type, kPlatformTypeHIView) == 0)
      return kResultTrue;

    if (strcmp(type, kPlatformTypeNSView) == 0)
      return kResultTrue;

    if (strcmp(type, kPlatformTypeX11EmbedWindowID) == 0)
      return kResultTrue;

    return kResultFalse;
  }

  tresult PLUGIN_API canResize() override
  {
    bool const isResizable = EventHandler::isResizingAllowed();
    return isResizable ? kResultTrue : kResultFalse;
  }

  tresult PLUGIN_API checkSizeConstraint(ViewRect* rect) override
  {
    int width = rect->getWidth();
    int height = rect->getHeight();
    EventHandler::adjustSize(width, height);
    rect->right = rect->left + width;
    rect->bottom = rect->top + height;
    return kResultTrue;
  }

  tresult PLUGIN_API onWheel(float distance) override
  {
    eventHandler->handleScroll(0, distance);
    return kResultFalse;
  }

  tresult PLUGIN_API onKeyDown(char16 key, int16 keyMsg, int16 modifiers) override
  {
    return eventHandler->onKeyEvent(key, keyMsg, modifiers, true);
  }

  tresult PLUGIN_API onKeyUp(char16 key, int16 keyMsg, int16 modifiers) override
  {
    return eventHandler->onKeyEvent(key, keyMsg, modifiers, false);
  }

private:
  pugl::World world;
  std::unique_ptr<pugl::View> puglView;
  std::unique_ptr<EventHandler> eventHandler;
  std::string name;
};

} // namespace detail
} // namespace unplug