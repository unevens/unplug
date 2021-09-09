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
#include <numeric>

using namespace Steinberg;

GainProcessor::GainProcessor() {
  setControllerClass(kUnplugDemoEffectControllerUID);
}

GainProcessor::~GainProcessor() = default;

template<class Sample, class Inputs, class Outputs>
void process(Inputs const& inputs, Outputs& outputs, int numInputs, int numOutputs, int numSamples) {}

tresult PLUGIN_API GainProcessor::process(Vst::ProcessData& data) {

  updateParametersToLastPoint(data);

  auto const gain = parameterStorage.get(Param::gain);
  bool const bypass = parameterStorage.get(Param::bypass) > 0.0;

  bool const wantsLevelMetering = meterStorage && isUserInterfaceOpen;

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
          if (wantsLevelMetering && c == 0) {
            if (data.symbolicSampleSize == Steinberg::Vst::kSample64) {
              for (int s = 0; s < data.numSamples; ++s) {
                levels[c] +=
                  levelSmooothingAlpha * static_cast<float>(std::abs(out.channelBuffers64[c][s]) - levels[c]);
              }
            }
            else {
              for (int s = 0; s < data.numSamples; ++s) {
                levels[c] +=
                  levelSmooothingAlpha * static_cast<float>(std::abs(out.channelBuffers32[c][s]) - levels[c]);
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

  if (wantsLevelMetering) {
    auto const level = std::reduce(levels.begin(), levels.end()) / static_cast<float>(levels.size());
    meterStorage->set(Meter::level, level);
  }

  return kResultOk;
}

Steinberg::tresult GainProcessor::setupProcessing(Vst::ProcessSetup& newSetup) {
  tresult result = UnplugProcessor::setupProcessing(newSetup);
  if (result == kResultFalse) {
    return kResultFalse;
  }
  auto const levelSmoothingTime = 1.0;
  levelSmooothingAlpha = 1.f - std::exp(-2 * M_PI / (newSetup.sampleRate * levelSmoothingTime));
  return kResultOk;
}

tresult GainProcessor::setActive(TBool state) {
  auto const input = getAudioInput(0);
  assert(input);
  if (input) {
    Vst::BusInfo info{};
    bool const gotInfoOk = input->getInfo(info);
    assert(gotInfoOk);
    if (gotInfoOk) {
      levels.resize(info.channelCount, 0.f);
    }
  }
  return UnplugProcessor::setActive(state);
}
