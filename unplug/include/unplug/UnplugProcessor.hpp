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

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

namespace Steinberg::Vst {

template<class Parameters>
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
  unplug::ParameterStorage<Parameters::numParameters> parameterStorage;
};

template<class Parameters>
void
UnplugProcessor<Parameters>::onInitialization()
{
  //--- create Audio IO ------
  addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);
}

template<class Parameters>
tresult PLUGIN_API
UnplugProcessor<Parameters>::initialize(FUnknown* context)
{
  using namespace Steinberg;
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  Parameters::getParameterInitializer().initializeStorage(parameterStorage);

  onInitialization();

  return kResultOk;
}

template<class Parameters>
tresult PLUGIN_API
UnplugProcessor<Parameters>::terminate()
{
  onTermination();
  return AudioEffect::terminate();
}

template<class Parameters>
void
UnplugProcessor<Parameters>::UpdateParametersToLastPoint(ProcessData& data)
{
  using namespace Steinberg;
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
        int32 numPoints = paramQueue->getPointCount();
        if (numPoints > 0) {
          ParamValue value;
          int32 sampleOffset;
          paramQueue->getPoint(numPoints - 1, sampleOffset, value);
          parameterStorage.setNormalized(index, value);
        }
      }
    }
  }
}

template<class Parameters>
tresult PLUGIN_API
UnplugProcessor<Parameters>::setupProcessing(ProcessSetup& newSetup)
{
  using namespace Steinberg;
  tresult result = AudioEffect::setupProcessing(newSetup);
  if (result != kResultOk) {
    return result;
  }
  onSetupProcessing(newSetup);
  return kResultOk;
}
/**
 * loads the state. may be called by either the Processing Thread or the UI Thread
 * */
template<class Parameters>
tresult PLUGIN_API
UnplugProcessor<Parameters>::setState(IBStream* state)
{
  using namespace Steinberg;

  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < Parameters::numParameters; ++i) {
    double value;
    if (!streamer.readDouble(value)) {
      return kResultFalse;
    }
    parameterStorage.set(i, value);
  }
  return kResultOk;
}

/**
 * saves the state. may be called by either the Processing Thread or the UI Thread
 * */
template<class Parameters>
tresult PLUGIN_API
UnplugProcessor<Parameters>::getState(IBStream* state)
{
  using namespace Steinberg;
  // saves the state
  //
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < Parameters::numParameters; ++i) {
    double const value = parameterStorage.get(i);
    if (!streamer.writeDouble(value)) {
      return kResultFalse;
    }
  }
  return kResultOk;
}

} // namespace Steinberg::Vst

namespace unplug {
template<class Parameters>
using UnplugProcessor = Steinberg::Vst::UnplugProcessor<Parameters>;
}