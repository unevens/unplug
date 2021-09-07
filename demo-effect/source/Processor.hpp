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

#include "unplug/UnplugProcessor.hpp"

class UnplugDemoEffectProcessor final : public unplug::UnplugProcessor
{
public:
  UnplugDemoEffectProcessor();
  ~UnplugDemoEffectProcessor() override;

  static Steinberg::FUnknown* createInstance(void* /*context*/)
  {
    return (Steinberg::Vst::IAudioProcessor*)new UnplugDemoEffectProcessor;
  }

  Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;

  Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) override;

private:
  float level = 0.0;
  float levelSmooothingAlpha = 0.0;
};
