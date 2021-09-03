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
#include "unplug/Parameters.hpp"
#include "NumParameters.hpp"

namespace Steinberg::Vst {

class UnplugProcessor : public AudioEffect
{
public:
  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  tresult PLUGIN_API initialize(FUnknown* context) override;

  tresult PLUGIN_API terminate() override;

  /** Will be called before any process call */
  tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup) override;

  /** For persistence */
  tresult PLUGIN_API setState(IBStream* state) override;
  tresult PLUGIN_API getState(IBStream* state) override;

protected:
  void UpdateParametersToLastPoint(ProcessData& data);

private:
  /** Called from initialize, at first after constructor */
  virtual void onInitialization();

  /** Called at the end before destructor, by terminate */
  virtual void onTermination() {}

  /**
   * Called by setupProcessing on the UI Thread, before the processing is started. The newSetup object contains
   * information such as the maximum number of samples per audio block and the sample rate, so this is the right place
   * where to allocate resources that depend on those.
   * */
  virtual void onSetupProcessing(ProcessSetup& newSetup) {}

protected:
  using ParameterStorage = unplug::ParameterStorage<unplug::NumParameters::value>;
  ParameterStorage parameterStorage;
};

} // namespace Steinberg::Vst

namespace unplug {

using UnplugProcessor = Steinberg::Vst::UnplugProcessor;
}