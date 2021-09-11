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
#include "CircularBuffers.hpp"
#include "Meters.hpp"
#include "Parameters.hpp"
#include "unplug/Automation.hpp"
#include "unplug/IO.hpp"
#include "unplug/ProcessingData.hpp"
#include <numeric>

class GainDsp final
{
public:
  using Index = unplug::Index;
  using ProcessingData = unplug::ProcessingData;
  template<class SampleType>
  using IO = unplug::IO<SampleType>;

  GainDsp(ProcessingData& pluginData)
    : pluginData{ pluginData }
  {}

  template<class SampleType>
  void staticProcessing(IO<SampleType> io, Index numSamples)
  {
    auto const gain = pluginData.parameters.get(Param::gain);
    bool const bypass = pluginData.parameters.get(Param::bypass) > 0.0;
    auto const sharedChannels = std::min(io.numOutputs, io.numInputs);
    for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
      auto& in = io.inputs[channelIndex];
      auto& out = io.outputs[channelIndex];
      if (bypass) {
        std::copy(&in[0], &in[0] + numSamples, &out[0]);
      }
      else {
        for (int s = 0; s < numSamples; ++s) {
          out[s] = gain * in[s];
        }
      }
    }
    for (Index channelIndex = sharedChannels; channelIndex < io.numOutputs; ++channelIndex) {
      auto& out = io.outputs[channelIndex];
      std::fill(&out[0], &out[0] + numSamples, 0.0);
    }
    levelMetering(io.outputs, io.numOutputs, numSamples);
  }

  template<class SampleType>
  void automatedProcessing(unplug::AutomationCache<SampleType>& automation,
                           IO<SampleType> io,
                           Index startSample,
                           Index endSample)
  {
    bool const bypass = automation.parameters[Param::bypass].currentValue > 0.0;
    auto const sharedChannels = std::min(io.numOutputs, io.numInputs);
    if (bypass) {
      for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
        auto& in = io.inputs[channelIndex];
        auto& out = io.outputs[channelIndex];
        std::copy(in + startSample, in + endSample, out);
      }
    }
    else {
      for (Index s = startSample; s < endSample; ++s) {
        for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
          auto const gain = automation.increment(Param::gain);
          io.outputs[channelIndex][s] = gain * io.inputs[channelIndex][s];
        }
      }
    }
    for (Index channelIndex = sharedChannels; channelIndex < io.numOutputs; ++channelIndex) {
      auto& out = io.outputs[channelIndex];
      std::fill(out + startSample, out + endSample, 0.0);
    }
  }

  void setup(double sampleRate, Index maxBlockSize)
  {
    auto const levelSmoothingTime = 1.0;
    levelSmoothingAlpha = static_cast<float>(1.0 - std::exp(-2 * M_PI / (sampleRate * levelSmoothingTime)));
  }

  void setNumChannels(Index inputChannels, Index outputChannels)
  {
    levels.resize(outputChannels, 0.f);
  }

  void reset()
  {
    std::fill(levels.begin(), levels.end(), 0.f);
  }

private:
  template<class SampleType>
  void levelMetering(SampleType** outputs, Index numOutputChannels, int numSamples)
  {
    bool const wantsLevelMetering = pluginData.meters && pluginData.isUserInterfaceOpen;
    if (wantsLevelMetering) {
      assert(levels.size() == numOutputChannels);
      levels.resize(numOutputChannels);
      for (Index channel = 0; channel < numOutputChannels; ++channel) {
        for (Index sample = 0; sample < numSamples; ++sample) {
          levels[channel] +=
            levelSmoothingAlpha * static_cast<float>(std::abs(outputs[channel][sample]) - levels[channel]);
        }
      }
      auto const level = std::reduce(levels.begin(), levels.end()) / static_cast<float>(levels.size());
      pluginData.meters->set(Meter::level, level);
    }
  }

  ProcessingData& pluginData;

  std::vector<float> levels;
  float levelSmoothingAlpha = 0.0;
};