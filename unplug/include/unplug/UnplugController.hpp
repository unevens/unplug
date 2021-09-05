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

#include "CircularBuffers.hpp"
#include "GetParameterDescriptions.hpp"
#include "Meters.hpp"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "unplug/MeterStorage.hpp"
#include "unplug/MidiMapping.hpp"
#include "unplug/ViewPersistentData.hpp"
#include "unplug/detail/Vst3View.hpp"
#include <memory>

namespace Steinberg::Vst {

class UnplugController
  : public EditControllerEx1
  , public IMidiMapping
{
public:
  using FUnknown = FUnknown;
  using IEditController = IEditController;
  using MidiMapping = unplug::MidiMapping;
  using View = unplug::vst3::detail::Vst3View;

  UnplugController() = default;
  ~UnplugController() override = default;

  // IPluginBase
  tresult PLUGIN_API initialize(FUnknown* context) override;

  // EditController
  tresult PLUGIN_API setComponentState(IBStream* state) override;

  IPlugView* PLUGIN_API createView(FIDString name) override;

  tresult PLUGIN_API setState(IBStream* state) override;

  tresult PLUGIN_API getState(IBStream* state) override;

  tresult PLUGIN_API getMidiControllerAssignment(int32 busIndex,
                                                 int16 channel,
                                                 CtrlNumber midiControllerNumber,
                                                 ParamID& tag) override;

  tresult PLUGIN_API setParamNormalized(ParamID tag, ParamValue value) override;

  tresult PLUGIN_API notify(IMessage* message) override;

  void onViewClosed();

  unplug::MidiMapping midiMapping;
  unplug::ViewPersistentData persistentData;
  std::array<int, 2> lastViewSize{ { -1, -1 } };
  std::shared_ptr<unplug::MeterStorage> meters;
  std::shared_ptr<unplug::CircularBufferStorage> circularBuffers;

private:
  void applyPreset(int presetIndex);

  DEFINE_INTERFACES
  DEF_INTERFACE(IMidiMapping);
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)
};

} // namespace Steinberg::Vst

namespace unplug {

using UnplugController = Steinberg::Vst::UnplugController;
}
