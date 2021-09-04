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

using namespace Steinberg;

UnplugDemoEffectProcessor::UnplugDemoEffectProcessor()
{
  setControllerClass(kUnplugDemoEffectControllerUID);
}

UnplugDemoEffectProcessor::~UnplugDemoEffectProcessor() = default;

tresult PLUGIN_API UnplugDemoEffectProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return kResultTrue;

  return kResultFalse;
}

tresult PLUGIN_API UnplugDemoEffectProcessor::process(Vst::ProcessData& data)
{
  UpdateParametersToLastPoint(data);

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
