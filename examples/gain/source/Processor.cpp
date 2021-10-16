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
  auto const oversampledSampleRate = context.getOversampledSampleRate();
  dspState.metering.setSampleRate(oversampledSampleRate);
  return true;
}

tresult PLUGIN_API Processor::setProcessing(TBool state)
{
  dspState.metering.reset();
  return kResultOk;
}

UnplugProcessor::Index Processor::getOversamplingRate() const
{
  return sharedDataWrapped->get().oversampling.getProcessorOnUiThread().getRate();
}

} // namespace Steinberg::Vst