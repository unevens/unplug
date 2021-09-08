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
#include "Meters.hpp"
#include "Parameters.hpp"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "unplug/GetParameterDescriptions.hpp"
#include "unplug/MeterStorage.hpp"
#include "unplug/ParameterStorage.hpp"
#include <atomic>
#include <memory>

namespace Steinberg::Vst {

class UnplugProcessor : public AudioEffect
{
public:
  using ParamId = unplug::ParamIndex;
  using MeterId = unplug::MeterIndex;

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

  /** Reports if the plugin supports 32/64 bit floating point audio */
  Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

  tresult PLUGIN_API notify(IMessage* message) override;

  tresult PLUGIN_API setActive(TBool state) override;

  tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs,
                                        int32 numIns,
                                        SpeakerArrangement* outputs,
                                        int32 numOuts) override;

protected:
  void updateParametersToLastPoint(ProcessData& data);

  tresult acceptBusArrangement(
    SpeakerArrangement* inputs,
    int32 numIns,
    SpeakerArrangement* outputs,
    int32 numOuts,
    bool acceptSidechain,
    const std::function<bool(int numInputs, int numOutputs, int numSidechain)>& acceptNumChannels);

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

  virtual bool supportsDoublePrecision() { return true; }

protected:
  unplug::ParameterStorage parameterStorage;
  std::shared_ptr<unplug::MeterStorage> meterStorage;
  std::shared_ptr<unplug::CircularBufferStorage> circularBufferStorage;
  std::atomic<bool> isUserInterfaceOpen{ false };
};

} // namespace Steinberg::Vst

namespace unplug {
using UnplugProcessor = Steinberg::Vst::UnplugProcessor;
}