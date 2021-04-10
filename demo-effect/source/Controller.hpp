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
#include "unplug/ViewPersistentData.hpp"
#include <memory>

namespace unplug {

class UnPlugDemoEffectController final : public Steinberg::Vst::EditControllerEx1
{
public:
  UnPlugDemoEffectController() = default;
  ~UnPlugDemoEffectController() SMTG_OVERRIDE = default;

  static Steinberg::FUnknown* createInstance(void* /*context*/)
  {
    return (Steinberg::Vst::IEditController*)new UnPlugDemoEffectController;
  }

  // IPluginBase
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  // EditController
  Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) SMTG_OVERRIDE;

  Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;

  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;

  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;

  Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID tag,
                                                   Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;

  Steinberg::tresult PLUGIN_API getParamStringByValue(Steinberg::Vst::ParamID tag,
                                                      Steinberg::Vst::ParamValue valueNormalized,
                                                      Steinberg::Vst::String128 string) SMTG_OVERRIDE;

  Steinberg::tresult PLUGIN_API getParamValueByString(Steinberg::Vst::ParamID tag,
                                                      Steinberg::Vst::TChar* string,
                                                      Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;

private:
  ViewPersistentData persistentData;

  //---Interface---------
  DEFINE_INTERFACES
  // Here you can add more supported VST3 interfaces
  // DEF_INTERFACE (Vst::IXXX)
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)
};

} // namespace unplug
