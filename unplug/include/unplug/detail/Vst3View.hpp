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
#include "Vst3Keycodes.hpp"
#include "Vst3Parameters.hpp"
#include "public.sdk/source/common/pluginview.h"
#include "pugl/gl.hpp"
#include <memory>
#include <string>

namespace unplug {
namespace vst3 {
namespace detail {

using FIDString = Steinberg::FIDString;
using ViewRect = Steinberg::ViewRect;
using char16 = Steinberg::char16;
using int16 = Steinberg::int16;

/**
 * The View class implements the Steinberg::IPluginView class that the plugin controller returns to the host
 * as a result of a call to createView.
 * We want the UserInterface class of your plugin to be completely decoupled from the VST3 SDK. In order to achieve
 * this, we use the Pugl library as a crossplatform window system upon which to build user interfaces. Two template
 * classes are implemented: The View class, which takes care of interfacing VST3 SDK with the Pugl window sytstem, and
 * more specifically
 * - manages the lifecycle of the child window
 * - handles VST3 callbacks related to the window, such as "onSize" and "onKeydown"
 * - exposes VST3 API to set and get the plugin parameter to the user interface
 * And an EventHandler class, which takes care of interfacing the UserInterface class of your plugin with the Pugl
 * window system. In this way, you can write the UserInterface class of your plugin against the API of the EventHandler
 * class, without depending on the VST3 SDK. The UserInterface class is then supplied to the EventHandler class and then
 * to the View class through dependency injection - see the files unplug/PluginView.hpp and unplug/DemoView.hpp.
 * */

template<class EventHandler>
class View final : public Steinberg::CPluginView
{
public:
  explicit View(EditControllerEx1* controller, const char* name)
    : world{ pugl::WorldType::module }
    , name{ name }
    , parameters{ controller }
  {
    controller->addRef();
    world.setClassName(name);
  }

  ~View() = default;

  tresult PLUGIN_API attached(void* pParent, FIDString type) override
  {
    CPluginView::attached(pParent, type);
    puglView = std::make_unique<pugl::View>(world);
    eventHandler = std::make_unique<EventHandler>(*puglView, parameters);
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
    if (plugFrame) {
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
    return onKeyEvent(key, keyMsg, modifiers, true);
  }

  tresult PLUGIN_API onKeyUp(char16 key, int16 keyMsg, int16 modifiers) override
  {
    return onKeyEvent(key, keyMsg, modifiers, false);
  }

private:
  Steinberg::tresult onKeyEvent(Steinberg::char16 key,
                                Steinberg::int16 keyMsg,
                                Steinberg::int16 modifiersMask,
                                bool isDown)
  {
    if (!eventHandler->wantsCaptureKeyboard())
      return Steinberg::kResultFalse;

    auto const modifiers = modifierKeysFromBitmask(modifiersMask);
    bool const isAscii = key > 0;
    if (isAscii) {
      eventHandler->onAsciiKeyEvent(key, isDown);
      eventHandler->handleModifierKeys(modifiers);
      return Steinberg::kResultTrue;
    }
    else { // not ascii
      auto const numPadKeyCode = convertNumPadKeyCode(keyMsg);
      if (numPadKeyCode > -1) {
        eventHandler->onAsciiKeyEvent(numPadKeyCode, isDown);
        eventHandler->handleModifierKeys(modifiers);
        return Steinberg::kResultTrue;
      }
      else { // not ASCII, not num pad
        auto const virtualKeyCode = convertVirtualKeyCode(keyMsg);
        if (virtualKeyCode > -1) {
          eventHandler->onNonAsciiKeyEvent(virtualKeyCode, isDown);
          eventHandler->handleModifierKeys(modifiers);
          return Steinberg::kResultTrue;
        }
        else { // not ASCII, not num pad, not special key
          eventHandler->handleModifierKeys(modifiers);
          return Steinberg::kResultTrue;
        }
      }
    }
  }

private:
  pugl::World world;
  std::unique_ptr<pugl::View> puglView;
  std::unique_ptr<EventHandler> eventHandler;
  std::string name;
  Parameters parameters;
};

} // namespace detail
} // namespace vst3
} // namespace unplug