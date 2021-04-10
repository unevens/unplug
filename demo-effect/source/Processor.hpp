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

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace unplug {

class UnPlugDemoEffectProcessor final : public Steinberg::Vst::AudioEffect
{
public:
  UnPlugDemoEffectProcessor();
  ~UnPlugDemoEffectProcessor() SMTG_OVERRIDE;

  static Steinberg::FUnknown* createInstance(void* /*context*/)
  {
    return (Steinberg::Vst::IAudioProcessor*)new UnPlugDemoEffectProcessor;
  }

  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  /** Called at first after constructor */
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;

  /** Called at the end before destructor */
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  /** Switch the Plug-in on/off */
  Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

  /** Will be called before any process call */
  Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;

  /** Will be called before and after any process call */
  Steinberg::tresult PLUGIN_API setProcessing (Steinberg::TBool state) SMTG_OVERRIDE;

  /** Asks if a given sample size is supported see SymbolicSampleSizes. */
  Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

  /** Here we go...the process call */
  Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;

  /** For persistence */
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;

};

} // namespace unplug
