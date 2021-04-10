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

#include "Processor.hpp"
#include "Id.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace unplug {

UnPlugDemoEffectProcessor::UnPlugDemoEffectProcessor()
{
  setControllerClass(kUnPlugDemoEffectControllerUID);
}

UnPlugDemoEffectProcessor::~UnPlugDemoEffectProcessor() {}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::initialize(FUnknown* context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  //--- create Audio IO ------
  addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);

  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::terminate()
{
  // Here the Plug-in will be de-instanciated, last possibility to remove some
  // memory!

  //---do not forget to call parent ------
  return AudioEffect::terminate();
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::setActive(TBool state)
{
  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::process(Vst::ProcessData& data)
{
  //--- First : Read inputs parameter changes-----------

  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
        Vst::ParamValue value;
        int32 sampleOffset;
        int32 numPoints = paramQueue->getPointCount();
        switch (paramQueue->getParameterId()) {}
      }
    }
  }

  //--- Here you have to implement your processing

  auto in = data.inputs[0].channelBuffers32;
  auto out = data.outputs[0].channelBuffers32;

  for (int c = 0; c < data.inputs[0].numChannels; ++c)
    std::copy(in[c], in[c] + data.numSamples, out[c]);

  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
{
  // called from the non-realtime thread.
  // the newSetup object contains information such as the maximum number of samples per audio block and the sample rate
  tresult result = AudioEffect::setupProcessing(newSetup);
  if (result != kResultOk) {
    return result;
  }

  return kResultOk;
}

Steinberg::tresult
UnPlugDemoEffectProcessor::setProcessing(Steinberg::TBool state)
{
  // may be called by both the realtime thread and the non realtime thread
  // it is called before the processing starts with state = true, and after it stops with state = false
  // it is used to reset the internal state of the plugin, as in cleaning any delay buffer and any filter memory
  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  // by default kSample32 is supported
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return kResultTrue;

  return kResultFalse;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::setState(IBStream* state)
{
  // called when we load a preset, the model has to be reloaded
  IBStreamer streamer(state, kLittleEndian);

  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::getState(IBStream* state)
{
  // here we need to save the model
  IBStreamer streamer(state, kLittleEndian);

  return kResultOk;
}

} // namespace unplug
