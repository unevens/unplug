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

#include "GainDsp.hpp"
#include "unplug/UnplugProcessor.hpp"

namespace Steinberg::Vst {

class Processor final : public UnplugProcessor
{
public:
  Processor();

  static FUnknown* createInstance(void* /*context*/)
  {
    return (IAudioProcessor*)new Processor;
  }

  tresult PLUGIN_API process(ProcessData& data) override;

  tresult PLUGIN_API setProcessing(TBool state) override;

  bool onSetup(ContextInfo const& context) override;

private:

  template<class SampleType>
  void TProcess(ProcessData& data);

  void updateLatency(unplug::ParamIndex paramIndex, ParamValue value) override;

  GainDsp::State dspState;
};
} // namespace Steinberg::Vst