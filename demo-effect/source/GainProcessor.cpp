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

#include "GainProcessor.hpp"
#include "Id.hpp"

namespace Steinberg::Vst {

GainProcessor::GainProcessor()
  : dsp{ processingData }
{
  setControllerClass(kUnplugDemoEffectControllerUID);
}

GainProcessor::~GainProcessor() = default;

tresult PLUGIN_API GainProcessor::process(ProcessData& data)
{
  if (data.symbolicSampleSize == kSample64) {
    TProcess<double>(data);
  }
  else {
    TProcess<float>(data);
  }
  return kResultOk;
}

Steinberg::tresult GainProcessor::setupProcessing(ProcessSetup& newSetup)
{
  tresult result = UnplugProcessor::setupProcessing(newSetup);
  if (result == kResultFalse) {
    return kResultFalse;
  }
  dsp.setup(newSetup.sampleRate, newSetup.maxSamplesPerBlock);
  return kResultOk;
}

tresult GainProcessor::setActive(TBool state)
{
  auto const numIO = getNumIO();
  dsp.setNumChannels(numIO.numIns, numIO.numOuts);
  return UnplugProcessor::setActive(state);
}

Steinberg::tresult GainProcessor::setProcessing(Steinberg::TBool state)
{
  dsp.reset();
  return UnplugProcessor::setProcessing(state);
}

template<class SampleType>
void GainProcessor::TProcess(ProcessData& data)
{
  using namespace unplug;
  auto automationCache = AutomationCache<SampleType>{ processingData.parameters };

  processWithSamplePreciseAutomation<SampleType>(
    data,
    [this](IO<SampleType> io, Index numSamples) { return dsp.staticProcessing(io, numSamples); },
    [&](IO<SampleType> io, Index startSample, Index endSample) {
      return dsp.template automatedProcessing(automationCache, io, startSample, endSample);
    },
    [&](unplug::AutomationEvent<SampleType> automationEvent) {
      setParameterAutomation(automationCache, automationEvent);
    });
}

Steinberg::tresult GainProcessor::setBusArrangements(SpeakerArrangement* inputs,
                                                     int32 numIns,
                                                     SpeakerArrangement* outputs,
                                                     int32 numOuts)
{
  UnplugProcessor::setBusArrangements(inputs, numIns, outputs, numOuts);
  bool const hasSidechain = false;
  return acceptSimpleBusArrangement(inputs,
                                    numIns,
                                    outputs,
                                    numOuts,
                                    hasSidechain,
                                    [](int numInputChannels, int numOutputChannels, int numSidechainChannnels) {
                                      return numInputChannels == numOutputChannels;
                                    });
}

} // namespace Steinberg::Vst