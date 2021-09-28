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
#include "CustomData.hpp"
#include "Meters.hpp"
#include "Parameters.hpp"
#include "unplug/Automation.hpp"
#include "unplug/IO.hpp"
#include "unplug/Math.hpp"
#include "unplug/PluginState.hpp"
#include <numeric>

namespace GainDsp {

using Index = unplug::Index;

template<class SampleType>
using IO = unplug::IO<SampleType>;

template<class SampleType>
using Automation = unplug::LinearAutomation<SampleType>;

struct MeteringCache final
{
  std::vector<float> levels;
  float levelSmoothingAlpha = 0.0;
  float invNumChannels = 1.f;

  void setSampleRate(double sampleRate)
  {
    auto const levelSmoothingTime = 1.0;
    levelSmoothingAlpha = 1.f - static_cast<float>(std::exp(-2 * M_PI / (sampleRate * levelSmoothingTime)));
  }

  void setNumChannels(Index outputChannels)
  {
    levels.resize(outputChannels, 0.f);
    invNumChannels = 1.f / static_cast<float>(outputChannels);
  }

  void reset()
  {
    std::fill(levels.begin(), levels.end(), 0.f);
  }
};

struct State final
{
  unplug::PluginState& pluginState;
  MeteringCache metering;

  explicit State(unplug::PluginState& pluginState)
    : pluginState{ pluginState }
  {}
};

template<class SampleType>
void levelMetering(State& state, IO<SampleType> io, Index numSamples)
{
  auto outputs = io.getOut(0).buffers;
  auto numOutputChannels = io.getOut(0).numChannels;
  bool const wantsLevelMetering = state.pluginState.meters && state.pluginState.isUserInterfaceOpen;
  if (wantsLevelMetering) {
    assert(state.metering.levels.size() == numOutputChannels);
    state.metering.levels.resize(numOutputChannels);
    auto& customData = state.pluginState.customData->get();
    auto levelRingBuffer = customData.levelRingBuffer.getFromAudioThread();
    if (levelRingBuffer) {
      unplug::sendToRingBuffer(
        *levelRingBuffer,
        outputs,
        numOutputChannels,
        0,
        numSamples,
        [&](auto sampleValue, Index channel) {
          auto memory = state.metering.levels[channel];
          memory += state.metering.levelSmoothingAlpha * static_cast<float>(std::abs(sampleValue) - memory);
          state.metering.levels[channel] = memory;
          return memory;
        },
        [](auto x, float weight) { return static_cast<float>(x) * weight; },
        [&](auto accumulatedValue, auto elementValue) { return accumulatedValue + elementValue; },
        [](auto weightedValue) { return std::max(-90.f, unplug::linearToDB(weightedValue)); });
    }
    auto waveformRingBuffer = customData.waveformRingBuffer.getFromAudioThread();
    if (waveformRingBuffer) {
      unplug::sendToWaveformRingBuffer(*waveformRingBuffer, outputs, numOutputChannels, 0, numSamples);
    }
  }
  auto const level =
    std::reduce(state.metering.levels.begin(), state.metering.levels.end()) * state.metering.invNumChannels;
  state.pluginState.meters->set(Meter::level, level);
}

template<class SampleType>
void staticProcessing(State& state, IO<SampleType> io, Index numSamples)
{
  auto const gain = state.pluginState.parameters.get(Param::gain);
  bool const bypass = state.pluginState.parameters.get(Param::bypass) > 0.0;
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
      for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        outputBuffer[sampleIndex] = gain * inputBuffer[sampleIndex];
      }
    }
  }
  for (Index channelIndex = sharedChannels; channelIndex < numOutputChannels; ++channelIndex) {
    auto outputBuffer = out.buffers[channelIndex];
    std::fill(outputBuffer, outputBuffer + numSamples, 0.0);
  }
}

template<class SampleType>
void automatedProcessing(State& state,
                         Automation<SampleType>& automation,
                         IO<SampleType> io,
                         Index startSample,
                         Index endSample)
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
    for (Index sampleIndex = startSample; sampleIndex < endSample; ++sampleIndex) {
      for (Index channelIndex = 0; channelIndex < sharedChannels; ++channelIndex) {
        auto const gain = automation.increment(Param::gain);
        out.buffers[channelIndex][sampleIndex] = gain * in.buffers[channelIndex][sampleIndex];
      }
    }
  }
  for (Index channelIndex = sharedChannels; channelIndex < numOutputChannels; ++channelIndex) {
    auto outputBuffer = out.buffers[channelIndex];
    std::fill(outputBuffer + startSample, outputBuffer + endSample, 0.0);
  }
}

template<class SampleType>
unplug::LinearAutomation<SampleType> prepareAutomation(State& state)
{
  return unplug::LinearAutomation<SampleType>(state.pluginState.parameters);
}
} // namespace GainDsp