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

#include "unplug/UnplugProcessor.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "unplug/detail/GetSortedParameterDescriptions.hpp"
#include "unplug/detail/Vst3MessageIds.hpp"

namespace Steinberg::Vst {

void UnplugProcessor::onInitialization()
{
  //--- create Audio IO ------
  addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);
}

tresult PLUGIN_API UnplugProcessor::initialize(FUnknown* context)
{
  using namespace Steinberg;
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  auto const parameterDescriptions = unplug::detail::getSortedParameterDescriptions();
  parameterStorage.initialize(parameterDescriptions);

  onInitialization();

  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::terminate()
{
  onTermination();
  return AudioEffect::terminate();
}

void UnplugProcessor::UpdateParametersToLastPoint(ProcessData& data)
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
          auto const parameterTag = paramQueue->getParameterId();
          parameterStorage.setNormalized(parameterTag, value);
        }
      }
    }
  }
}

tresult PLUGIN_API UnplugProcessor::setupProcessing(ProcessSetup& newSetup)
{
  using namespace Steinberg;
  tresult result = AudioEffect::setupProcessing(newSetup);
  if (result != kResultOk) {
    return result;
  }
  onSetupProcessing(newSetup);
  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::setState(IBStream* state)
{
  using namespace Steinberg;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < unplug::NumParameters::value; ++i) {
    double value;
    if (!streamer.readDouble(value)) {
      return kResultFalse;
    }
    parameterStorage.set(i, value);
  }
  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::getState(IBStream* state)
{
  using namespace Steinberg;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < unplug::NumParameters::value; ++i) {
    double const value = parameterStorage.get(i);
    if (!streamer.writeDouble(value)) {
      return kResultFalse;
    }
  }
  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return supportsDoublePrecision();

  return kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::notify(IMessage* message)
{
  if (!message)
    return kInvalidArgument;

  if (FIDStringsEqual(message->getMessageID(), unplug::vst3::messaageIds::programChange)) {
    void const* binary = nullptr;
    uint32 size = 0;
    message->getAttributes()->getBinary(unplug::vst3::messaageIds::parameterValues, binary, size);
    assert(binary);
    if (binary) {
      auto values = static_cast<double const*>(binary);
      for (int i = 0; i < unplug::NumParameters::value; ++i) {
        parameterStorage.set(i, values[i]);
      }
    }
    return kResultOk;
  }

  return AudioEffect::notify(message);
}

} // namespace Steinberg::Vst

namespace unplug {

using UnplugProcessor = Steinberg::Vst::UnplugProcessor;
}