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

#include "Parameters.hpp"
#include "unplug/UnplugProcessor.hpp"

using BaseProcessor = unplug::UnplugProcessor<DemoEffectParameters>;

class UnplugDemoEffectProcessor final : public BaseProcessor
{
public:
  UnplugDemoEffectProcessor();
  ~UnplugDemoEffectProcessor() override;

  static Steinberg::FUnknown* createInstance(void* /*context*/)
  {
    return (Steinberg::Vst::IAudioProcessor*)new UnplugDemoEffectProcessor;
  }

  /** Reports if the plugin supports 32/64 bit floating point audio */
  Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

  Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;

};
