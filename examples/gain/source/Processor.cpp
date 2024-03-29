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

namespace Steinberg::Vst {

Processor::Processor()
  : dspState{ pluginState }
{
  setControllerClass(kControllerUID);
}

bool Processor::onSetup(ContextInfo const& context)
{
  dspState.metering.setNumChannels(context.numIO.numOuts);
  dspState.metering.setSampleRate(context.sampleRate);
  return true;
}

tresult PLUGIN_API Processor::setProcessing(TBool state)
{
  dspState.metering.reset();
  sharedDataWrapped->get().oversampling.reset();
  return kResultOk;
}

void Processor::updateLatency(unplug::ParamIndex paramIndex, ParamValue value)
{
  auto oversamplingOrder = pluginState.parameters.get(Param::oversamplingOrder);
  auto oversamplingLinearPhase = pluginState.parameters.get(Param::oversamplingLinearPhase);
  switch (paramIndex) {
    case Param::oversamplingOrder:
      oversamplingOrder = value;
      break;
    case Param::oversamplingLinearPhase:
      oversamplingLinearPhase = value;
      break;
    default:
      return;
  }
  if (oversamplingLinearPhase > 0.5) {
    auto const latency = sharedDataWrapped->get().oversampling.getLatency(
      static_cast<uint32_t>(oversamplingOrder), static_cast<uint32_t>(oversamplingLinearPhase));
    setLatency(static_cast<uint32_t>(latency));
  }
  else {
    setLatency(0);
  }
}

} // namespace Steinberg::Vst