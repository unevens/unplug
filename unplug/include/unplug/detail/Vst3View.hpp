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
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/common/pluginview.h"
#include "pugl/pugl.hpp"
#include "unplug/MidiMapping.hpp"
#include "unplug/ViewPersistentData.hpp"
#include "unplug/detail/EventHandler.hpp"
#include "unplug/detail/Vst3Keycodes.hpp"
#include "unplug/detail/Vst3ParameterAccess.hpp"
#include <memory>
#include <string>

namespace unplug::vst3::detail {

using FIDString = Steinberg::FIDString;
using ViewRect = Steinberg::ViewRect;
using char16 = Steinberg::char16;
using int16 = Steinberg::int16;
using int32 = Steinberg::int32;
using ParamID = Steinberg::Vst::ParamID;
using EventHandler = unplug::detail::EventHandler;

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
 * to the View class through dependency injection.
 * */

class Vst3View final
  : public Steinberg::CPluginView
  , public Steinberg::Vst::IParameterFinder
{
public:
  DELEGATE_REFCOUNT(Steinberg::CPluginView)

  tresult queryInterface(const char* iid, void** obj) override;

  Vst3View(EditControllerEx1& controller,
           ViewPersistentData& persistentData,
           MidiMapping& midiMapping,
           std::array<int, 2>& lastViewSize);

  ~Vst3View() override = default;

  tresult PLUGIN_API findParameter(int32 xPos, int32 yPos, ParamID& resultTag) override;

  tresult PLUGIN_API attached(void* pParent, FIDString type) override;

  tresult PLUGIN_API removed() override;

  tresult PLUGIN_API onSize(ViewRect* r) override;

  tresult PLUGIN_API isPlatformTypeSupported(FIDString type) override;

  tresult PLUGIN_API canResize() override;

  tresult PLUGIN_API checkSizeConstraint(ViewRect* rect) override;

  tresult PLUGIN_API onWheel(float distance) override;

  tresult PLUGIN_API onKeyDown(char16 key, int16 keyMsg, int16 modifiers) override;

  tresult PLUGIN_API onKeyUp(char16 key, int16 keyMsg, int16 modifiers) override;

private:
  std::array<int, 2> getDefaultSize() const;

  Steinberg::tresult onKeyEvent(Steinberg::char16 key,
                                Steinberg::int16 keyMsg,
                                Steinberg::int16 modifiersMask,
                                bool isDown);

private:
  pugl::World world;
  std::unique_ptr<pugl::View> puglView;
  std::unique_ptr<EventHandler> eventHandler;
  ParameterAccess parameters;
  ViewPersistentData& persistentData;
  std::array<int, 2>& lastViewSize;
};

} // namespace unplug::vst3::detail