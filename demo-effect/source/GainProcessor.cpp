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
  : dspState{ pluginState }
{
  setControllerClass(kUnplugDemoEffectControllerUID);
}

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

void GainProcessor::onSetActive(bool isActive)
{
  auto const numIO = getNumIO();
  dspState.metering.setNumChannels(numIO.numOuts);
  dspState.metering.setSampleRate(processSetup.sampleRate);
}

tresult PLUGIN_API GainProcessor::setProcessing(TBool state)
{
  dspState.metering.reset();
  return kResultOk;
}

template<class SampleType>
void GainProcessor::TProcess(ProcessData& data)
{
  constexpr auto useSamplePreciseAutomation = true;
  if constexpr (useSamplePreciseAutomation) {
    processWithSamplePreciseAutomation<SampleType>(
      data,
      [this](IO<SampleType> io, Index numSamples) { GainDsp::staticProcessing(dspState, io, numSamples); },
      [this]() { return GainDsp::prepareAutomation<SampleType>(dspState); },
      [&](auto& automation, IO<SampleType> io, Index startSample, Index endSample) {
        GainDsp::automatedProcessing(dspState, automation, io, startSample, endSample);
      },
      [&](auto& automation, auto automationEvent) { unplug::setParameterAutomation(automation, automationEvent); });
  }
  else {
    staticProcessing<SampleType>(
      data, [this](IO<SampleType> io, Index numSamples) { GainDsp::staticProcessing(dspState, io, numSamples); });
  }
}

} // namespace Steinberg::Vst