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

UnPlugDemoEffectProcessor::UnPlugDemoEffectProcessor()
{
  setControllerClass(kUnPlugDemoEffectControllerUID);
}

UnPlugDemoEffectProcessor::~UnPlugDemoEffectProcessor() = default;

tresult PLUGIN_API
UnPlugDemoEffectProcessor::initialize(FUnknown* context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  getParameterInitializer().initializeStorage(parameterStorage);

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

  auto const gain = parameterStorage.get(ParamTag::gain);
  bool const bypass = parameterStorage.get(ParamTag::bypass) > 0.0;

  for (int o = 0; o < data.numOutputs; ++o) {
    auto out = data.outputs[o];
    if (o < data.numInputs) {
      auto in = data.inputs[o];
      for (int c = 0; c < out.numChannels; ++c) {
        if (c < in.numChannels) {
          if (bypass) {
            if (data.symbolicSampleSize == Steinberg::Vst::kSample64) {
              std::copy(in.channelBuffers64[c], in.channelBuffers64[c] + data.numSamples, out.channelBuffers64[c]);
            }
            else {
              std::copy(in.channelBuffers32[c], in.channelBuffers32[c] + data.numSamples, out.channelBuffers32[c]);
            }
          }
          else {
            if (data.symbolicSampleSize == Steinberg::Vst::kSample64) {
              for (int s = 0; s < data.numSamples; ++s) {
                out.channelBuffers64[c][s] = gain * in.channelBuffers64[c][s];
              }
            }
            else {
              for (int s = 0; s < data.numSamples; ++s) {
                out.channelBuffers32[c][s] = gain * in.channelBuffers32[c][s];
              }
            }
          }
        }
      }
    }
    else {
      for (int c = 0; c < out.numChannels; ++c) {
        if (data.symbolicSampleSize == Steinberg::Vst::kSample64) {
          std::fill(out.channelBuffers64[c], out.channelBuffers64[c] + data.numSamples, 0.0);
        }
        else {
          std::fill(out.channelBuffers32[c], out.channelBuffers32[c] + data.numSamples, 0.f);
        }
      }
    }
  }

  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
{
  // called from the UI Thread.
  // the newSetup object contains information such as the maximum number of samples per audio block and the sample
  // rate
  tresult result = AudioEffect::setupProcessing(newSetup);
  if (result != kResultOk) {
    return result;
  }

  return kResultOk;
}

Steinberg::tresult
UnPlugDemoEffectProcessor::setProcessing(Steinberg::TBool state)
{
  // may be called by both the Processing Thread and the UI thread
  // it is called before the processing starts with state = true, and after it stops with state = false
  // it is used to reset the internal state of the plugin, as in cleaning any delay buffer and any filter memory
  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return kResultTrue;

  return kResultFalse;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::setState(IBStream* state)
{
  // loads the state
  // may be called by either the Processing Thread or the UI Thread
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < ParamTag::numParams; ++i) {
    double value;
    if (!streamer.readDouble(value)) {
      return kResultFalse;
    }
    parameterStorage.set(i, value);
  }
  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectProcessor::getState(IBStream* state)
{
  // saves the state
  // may be called by either the Processing Thread or the UI Thread
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < ParamTag::numParams; ++i) {
    double const value = parameterStorage.get(i);
    if (!streamer.writeDouble(value)) {
      return kResultFalse;
    }
  }
  return kResultOk;
}
