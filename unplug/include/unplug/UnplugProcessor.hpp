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

#include "Id.hpp"
#include "Parameters.hpp"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace unplug {

template<class PluginParameters>
class UnPlugProcessor : public Steinberg::Vst::AudioEffect
{
public:
  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

  Steinberg::tresult PLUGIN_API terminate() override;

  /** Will be called before any process call */
  Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) override;

  /** For persistence */
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

protected:
  void UpdateParametersToLastPoint(Steinberg::Vst::ProcessData& data);

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
  virtual void onSetupProcessing(Steinberg::Vst::ProcessSetup& newSetup) {}

protected:
  unplug::ParameterStorage<PluginParameters::numParameters> parameterStorage;
};

template<class PluginParameters>
void
UnPlugProcessor<PluginParameters>::onInitialization()
{
  //--- create Audio IO ------
  addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);
}

template<class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugProcessor<PluginParameters>::initialize(FUnknown* context)
{
  using namespace Steinberg;
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  PluginParameters::getParameterInitializer().initializeStorage(parameterStorage);

  onInitialization();

  return kResultOk;
}

template<class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugProcessor<PluginParameters>::terminate()
{
  onTermination();
  return AudioEffect::terminate();
}

template<class PluginParameters>
void
UnPlugProcessor<PluginParameters>::UpdateParametersToLastPoint(Steinberg::Vst::ProcessData& data)
{
  using namespace Steinberg;
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
        int32 numPoints = paramQueue->getPointCount();
        if (numPoints > 0) {
          Vst::ParamValue value;
          int32 sampleOffset;
          paramQueue->getPoint(numPoints - 1, sampleOffset, value);
          parameterStorage.setNormalized(index, value);
        }
      }
    }
  }
}

template<class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugProcessor<PluginParameters>::setupProcessing(Steinberg::Vst::ProcessSetup& newSetup)
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
template<class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugProcessor<PluginParameters>::setState(Steinberg::IBStream* state)
{
  using namespace Steinberg;

  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < PluginParameters::numParameters; ++i) {
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
template<class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugProcessor<PluginParameters>::getState(Steinberg::IBStream* state)
{
  using namespace Steinberg;
  // saves the state
  //
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < PluginParameters::numParameters; ++i) {
    double const value = parameterStorage.get(i);
    if (!streamer.writeDouble(value)) {
      return kResultFalse;
    }
  }
  return kResultOk;
}

} // namespace unplug
