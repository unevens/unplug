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
#include "unplug/PluginState.hpp"
#include <numeric>

namespace GainDsp {
using Index = unplug::Index;
template<class SampleType>
using IO = unplug::IO<SampleType>;
template<class SampleType>
using Automation = unplug::LinearAutomation<SampleType>;

struct MeteringCache
{
  std::vector<float> levels;
  float levelSmoothingAlpha = 0.0;

  void setSampleRate(double sampleRate)
  {
    auto const levelSmoothingTime = 1.0;
    levelSmoothingAlpha = static_cast<float>(1.0 - std::exp(-2 * M_PI / (sampleRate * levelSmoothingTime)));
  }

  void setNumChannels(Index outputChannels)
  {
    levels.resize(outputChannels, 0.f);
  }

  void reset()
  {
    std::fill(levels.begin(), levels.end(), 0.f);
  }
};

struct State
{
  unplug::PluginState& pluginState;
  MeteringCache metering;

  State(unplug::PluginState& pluginState)
    : pluginState{ pluginState }
  {}
};

namespace detail {
template<class SampleType>
void levelMetering(State& data, SampleType** outputs, Index numOutputChannels, Index startSample, Index endSample)
{
  bool const wantsLevelMetering = data.pluginState.meters && data.pluginState.isUserInterfaceOpen;
  if (wantsLevelMetering) {
    assert(data.metering.levels.size() == numOutputChannels);
    data.metering.levels.resize(numOutputChannels);
    for (Index channel = 0; channel < numOutputChannels; ++channel) {
      for (Index sample = startSample; sample < endSample; ++sample) {
        data.metering.levels[channel] +=
          data.metering.levelSmoothingAlpha *
          static_cast<float>(std::abs(outputs[channel][sample]) - data.metering.levels[channel]);
      }
    }
    auto const level = std::reduce(data.metering.levels.begin(), data.metering.levels.end()) /
                       static_cast<float>(data.metering.levels.size());
    data.pluginState.meters->set(Meter::level, level);
  }
}
} // namespace detail

template<class SampleType>
void staticProcessing(State& data, IO<SampleType> io, Index numSamples)
{
  auto const gain = data.pluginState.parameters.get(Param::gain);
  bool const bypass = data.pluginState.parameters.get(Param::bypass) > 0.0;
  auto in = io.getIn(0);
  auto out = io.getOut(0);
  auto const numInputChannels = in.numChannels;
  auto const numOutputChannels = out.numChannels;
  auto const sharedChannels = std::min(numOutputChannels, numInputChannels);
  for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
    auto inputBuffer = in.buffers[channelIndex];
    auto outputBuffer = out.buffers[channelIndex];
    if (bypass) {
      std::copy(inputBuffer, inputBuffer + numSamples, outputBuffer);
    }
    else {
      for (int s = 0; s < numSamples; ++s) {
        outputBuffer[s] = gain * inputBuffer[s];
      }
    }
  }
  for (Index channelIndex = sharedChannels; channelIndex < numOutputChannels; ++channelIndex) {
    auto outputBuffer = out.buffers[channelIndex];
    std::fill(outputBuffer, outputBuffer + numSamples, 0.0);
  }
  detail::levelMetering(data, out.buffers, out.numChannels, 0, numSamples);
}

template<class SampleType>
void automatedProcessing(State& data, Automation<SampleType>& automation, IO<SampleType> io, Index startSample, Index endSample)
{
  bool const bypass = automation.parameters[Param::bypass].currentValue > 0.0;
  auto in = io.getIn(0);
  auto out = io.getOut(0);
  auto const numInputChannels = in.numChannels;
  auto const numOutputChannels = out.numChannels;
  auto const sharedChannels = std::min(numOutputChannels, numInputChannels);
  if (bypass) {
    for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
      auto inputBuffer = in.buffers[channelIndex];
      auto outputBuffer = out.buffers[channelIndex];
      std::copy(inputBuffer + startSample, inputBuffer + endSample, outputBuffer + startSample);
    }
  }
  else {
    for (Index s = startSample; s < endSample; ++s) {
      for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
        auto const gain = automation.increment(Param::gain);
        out.buffers[channelIndex][s] = gain * in.buffers[channelIndex][s];
      }
    }
  }
  for (Index channelIndex = sharedChannels; channelIndex < numOutputChannels; ++channelIndex) {
    auto outputBuffer = out.buffers[channelIndex];
    std::fill(outputBuffer + startSample, outputBuffer + endSample, 0.0);
  }
  detail::levelMetering(data, out.buffers, out.numChannels, startSample, endSample);
}

template<class SampleType>
unplug::LinearAutomation<SampleType> prepareAutomation(State& data)
{
  return unplug::LinearAutomation<SampleType>(data.pluginState.parameters);
};

}; // namespace GainDsp