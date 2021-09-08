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

#include "unplug/detail/Vst3View.hpp"
#include "pugl/gl.hpp"
#include "unplug/UnplugController.hpp"
#include "unplug/UserInterface.hpp"

namespace unplug::vst3::detail {

detail::Vst3View::Vst3View(UnplugController& controller)
  : world{ pugl::WorldType::module }
  , controller{ controller }
  , parameters{ controller, controller.midiMapping } {
  world.setClassName(UserInterface::getWindowName());
}

tresult Vst3View::queryInterface(const char* iid, void** obj) {
  QUERY_INTERFACE(iid, obj, Steinberg::Vst::IParameterFinder::iid, Steinberg::Vst::IParameterFinder)
  return Steinberg::CPluginView::queryInterface(iid, obj);
}

tresult Vst3View::findParameter(int32 xPos, int32 yPos, ParamID& resultTag) {
  int tag = 0;
  if (parameters.findParameterFromUserInterfaceCoordinates(xPos, yPos, tag)) {
    resultTag = static_cast<ParamID>(tag);
    return kResultTrue;
  }
  else {
    return kResultFalse;
  }
}

tresult Vst3View::attached(void* pParent, FIDString type) {
  CPluginView::attached(pParent, type);
  puglView = std::make_unique<pugl::View>(world);
  eventHandler = std::make_unique<EventHandler>(*puglView, parameters, controller.meters, controller.circularBuffers);
  puglView->setEventHandler(*eventHandler);
  puglView->setParentWindow((pugl::NativeView)pParent);
  puglView->setWindowTitle(UserInterface::getWindowName());
  auto const defaultSize =
    (controller.lastViewSize[0] > -1 && controller.lastViewSize[1] > -1) ? controller.lastViewSize : getDefaultSize();
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
#if (UNPLUG_OPENGL_VERSION == 3)
  puglView->setHint(pugl::ViewHint::contextVersionMajor, 3);
  puglView->setHint(pugl::ViewHint::contextVersionMinor, 0);
  puglView->setHint(pugl::ViewHint::useCompatProfile, false);
#endif
#if (UNPLUG_OPENGL_VERSION == 2)
  puglView->setHint(pugl::ViewHint::contextVersionMajor, 2);
  puglView->setHint(pugl::ViewHint::contextVersionMinor, 0);
  puglView->setHint(pugl::ViewHint::useCompatProfile, true);
#endif
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

tresult Vst3View::removed() {
  puglView.reset(nullptr);
  eventHandler.reset(nullptr);

  return CPluginView::removed();
}

tresult Vst3View::onSize(ViewRect* r) {
  if (puglView) {
    puglView->setFrame({ (double)r->left, (double)r->top, (double)r->getWidth(), (double)r->getHeight() });
    puglView->postRedisplay();
  }
  if (puglView) {
    controller.lastViewSize[0] = r->getWidth();
    controller.lastViewSize[1] = r->getHeight();
  }
  return CPluginView::onSize(r);
}

tresult Vst3View::isPlatformTypeSupported(FIDString type) {
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

tresult Vst3View::canResize() {
  bool const isResizable = UserInterface::isResizingAllowed();
  return isResizable ? kResultTrue : kResultFalse;
}

static void adjustSizeToDefaultRatio(int& width, int& height) {
  auto const referenceSize = UserInterface::getDefaultSize();
  auto const referenceWidth = static_cast<float>(referenceSize[0]);
  auto const referenceHeight = static_cast<float>(referenceSize[1]);
  auto const widthRatio = static_cast<float>(width) / referenceWidth;
  auto const heightRatio = static_cast<float>(height) / referenceHeight;
  auto const ratio = std::max(UserInterface::getMinZoom(), std::min(widthRatio, heightRatio));
  width = static_cast<int>(ratio * referenceWidth);
  height = static_cast<int>(ratio * referenceHeight);
}

tresult Vst3View::checkSizeConstraint(ViewRect* rect) {
  int requestedWidth = rect->getWidth();
  int requestedHeight = rect->getHeight();
  UserInterface::adjustSize(requestedWidth, requestedHeight, controller.lastViewSize[0], controller.lastViewSize[1]);
  if (UserInterface::keepDefaultRatio()) {
    adjustSizeToDefaultRatio(requestedWidth, requestedHeight);
  }
  rect->right = rect->left + requestedWidth;
  rect->bottom = rect->top + requestedHeight;
  return kResultTrue;
}

tresult Vst3View::onWheel(float distance) {
  eventHandler->handleScroll(0, distance);
  return kResultFalse;
}

tresult Vst3View::onKeyDown(char16 key, int16 keyMsg, int16 modifiers) {
  return onKeyEvent(key, keyMsg, modifiers, true);
}

tresult Vst3View::onKeyUp(char16 key, int16 keyMsg, int16 modifiers) {
  return onKeyEvent(key, keyMsg, modifiers, false);
}

std::array<int, 2> Vst3View::getDefaultSize() const {
  bool const hasLastViewSize = controller.lastViewSize[0] > -1 && controller.lastViewSize[1] > -1;
  return hasLastViewSize ? controller.lastViewSize : UserInterface::getDefaultSize();
}

Steinberg::tresult Vst3View::onKeyEvent(Steinberg::char16 key,
                                        Steinberg::int16 keyMsg,
                                        Steinberg::int16 modifiersMask,
                                        bool isDown) {
  if (!eventHandler->wantsCaptureKeyboard())
    return Steinberg::kResultFalse;

  auto const modifiers = modifierKeysFromBitmask(modifiersMask);
  bool const isAscii = key > 0;
  if (isAscii) {
    eventHandler->onAsciiKeyEvent(key, isDown);
    eventHandler->handleModifierKeys(modifiers);
  }
  else { // not ASCII
    auto const numPadKeyCode = convertNumPadKeyCode(keyMsg);
    if (numPadKeyCode > -1) {
      eventHandler->onAsciiKeyEvent(numPadKeyCode, isDown);
      eventHandler->handleModifierKeys(modifiers);
    }
    else { // not ASCII, not num pad
      auto const virtualKeyCode = convertVirtualKeyCode(keyMsg);
      if (virtualKeyCode > -1) {
        eventHandler->onNonAsciiKeyEvent(virtualKeyCode, isDown);
        eventHandler->handleModifierKeys(modifiers);
      }
      else { // not ASCII, not num pad, not special key
        eventHandler->handleModifierKeys(modifiers);
      }
    }
  }
  return Steinberg::kResultTrue;
}
} // namespace unplug::vst3::detail