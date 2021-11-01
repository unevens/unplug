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
#include "unplug/ContextInfo.hpp"
#include "unplug/Index.hpp"
#include "unplug/Math.hpp"
#include "unplug/Serialization.hpp"
#include <atomic>
#include <vector>

namespace unplug {

struct RingBufferSettings final
{
  ContextInfo context;
  float pointsPerSecond = 128;
  float durationInSeconds = 1.f;
  bool operator==(RingBufferSettings const&) const noexcept = default;
};

/**
 * A ring buffer to send continuous data from the dsp to the user interface.
 * */
template<class ElementType, class Allocator = std::allocator<ElementType>>
class RingBuffer final
{
public:
  using Buffer = std::vector<ElementType, Allocator>;

  Buffer& getBuffer()
  {
    return buffer;
  }

  ElementType& at(Index channel, Index pointIndex)
  {
    return buffer[numChannels * pointIndex + channel];
  }

  Index wrapIndex(Index index) const
  {
    auto const bufferSize = static_cast<int>(bufferCapacity);
    auto const wrapped = (((index) % bufferSize) + bufferSize) % bufferSize;
    assert(wrapped >= 0);
    assert(wrapped < bufferSize);
    return wrapped;
  }

  Index getWritePosition() const
  {
    return static_cast<Index>(writePosition().load(std::memory_order_acquire));
  }

  Index getReadPosition() const
  {
    return wrapIndex(getWritePosition() - readBlockSize);
  }

  Index getReadBlockSize() const
  {
    return readBlockSize;
  }

  float getPointsPerSample() const
  {
    return pointsPerSample;
  }

  float getSamplesPerPoint() const
  {
    return samplesPerPoint;
  }

  Index getNumChannels() const
  {
    return numChannels;
  }

  void setWritePosition(Index newWritePosition)
  {
    writePosition().store(static_cast<unsigned int>(newWritePosition), std::memory_order_release);
  }

  Index getBufferCapacity() const
  {
    return bufferCapacity;
  }

  template<class T>
  void reset(T valueToResetTo)
  {
    std::fill(std::begin(buffer), std::end(buffer), valueToResetTo);
    std::fill(std::begin(accumulator), std::end(accumulator), valueToResetTo);
  }

  float getPointsPerSecond() const
  {
    return settings.pointsPerSecond;
  }

  float getSecondsPerPoint() const
  {
    return secondsPerPoint;
  }

  float getDurationInSeconds() const
  {
    return settings.durationInSeconds;
  }

  void setContext(ContextInfo const& context)
  {
    settings.context = context;
    resize();
  }

  void setResolution(float pointsPerSecond, float durationInSeconds)
  {
    settings.pointsPerSecond = pointsPerSecond;
    settings.durationInSeconds = durationInSeconds;
    resize();
  }

  void setSettings(RingBufferSettings settings_)
  {
    settings = settings_;
    resize();
  }

  ContextInfo const& getContext() const
  {
    return settings.context;
  }

  void setUseOversampledSampleRate(bool value)
  {
    useOversampledSampleRate = value;
    resize();
  }

  explicit RingBuffer(RingBufferSettings settings = {})
    : settings{ settings }
  {
    secondsPerPoint = 1.f / settings.pointsPerSecond;
    resize();
  }

  RingBufferSettings const& getSettings() const
  {
    return settings;
  }

  std::vector<ElementType, Allocator> accumulator;
  float accumulatedSamples = 0.f;

private:
  virtual Index choseNumChannels(NumIO numIO) const
  {
    return numIO.numOuts;
  }

  void resize()
  {
    auto const& contextInfo = settings.context;
    numChannels = choseNumChannels(contextInfo.numIO);
    accumulator.resize(numChannels);
    auto actualSampleRate = useOversampledSampleRate ? contextInfo.getOversampledSampleRate() : contextInfo.sampleRate;
    samplesPerPoint = actualSampleRate / settings.pointsPerSecond;
    pointsPerSample = 1.f / samplesPerPoint;
    auto const maxWriteIncrementPerAudioBlock = pointsPerSample * static_cast<float>(contextInfo.maxAudioBlockSize);
    readBlockSize = static_cast<int>(std::ceil(settings.durationInSeconds * settings.pointsPerSecond));
    auto const audioBlockDuration = static_cast<float>(contextInfo.maxAudioBlockSize) / contextInfo.sampleRate;
    auto const refreshTime = 1.f / contextInfo.userInterfaceRefreshRate;
    auto const audioBlocksPerUserInterfaceRefreshTime = refreshTime / audioBlockDuration;
    resize(static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock)), audioBlocksPerUserInterfaceRefreshTime);
  }

  void resize(Index maxWriteIncrementPerAudioBlock, float audioBlocksPerUserInterfaceRefreshTime)
  {
    auto const bufferForProduction = static_cast<int>(
      std::ceil(static_cast<float>(maxWriteIncrementPerAudioBlock) * audioBlocksPerUserInterfaceRefreshTime));
    auto const newSize = readBlockSize + bufferForProduction;
    resize(newSize);
  }

  void resize(Index newSize)
  {
    auto const currentWritePosition = getWritePosition();
    if (newSize <= currentWritePosition) {
      setWritePosition(0);
      auto const delta = currentWritePosition - newSize;
      if (delta > 0) {
        std::copy(std::begin(buffer) + delta * numChannels,
                  std::begin(buffer) + currentWritePosition * numChannels,
                  std::begin(buffer));
      }
    }
    bufferCapacity = newSize;
    buffer.resize(newSize * numChannels);
  }

  template<class T>
  struct MovableAtomic
  {
    std::atomic<T> value;

    std::atomic<T> const& operator()() const
    {
      return value;
    }

    std::atomic<T>& operator()()
    {
      return value;
    }

    friend void swap(MovableAtomic& first, MovableAtomic& second) noexcept
    {
      auto const secondValue = second.value.load(std::memory_order_acquire);
      auto const firstValue = first.value.load(std::memory_order_acquire);
      first.value.store(secondValue, std::memory_order_release);
      second.value.store(firstValue, std::memory_order_release);
    }

    MovableAtomic() = default;

    explicit MovableAtomic(T value)
      : value{ value }
    {}

    MovableAtomic(MovableAtomic&& other) noexcept
      : MovableAtomic()
    {
      swap(*this, other);
    }

    MovableAtomic& operator=(MovableAtomic other) noexcept
    {
      swap(*this, other);
      return *this;
    }
  };

  Index numChannels = 1;
  Index bufferCapacity = 0;
  MovableAtomic<int> writePosition{ 0 };
  Index readBlockSize = 0;
  float pointsPerSample = 1.f;
  float samplesPerPoint = 1;
  RingBufferSettings settings;
  float secondsPerPoint;
  bool useOversampledSampleRate = false;
  Buffer buffer;
};

template<Serialization::Action action, class ElementType, class Allocator = std::allocator<ElementType>>
bool serialization(RingBuffer<ElementType, Allocator>& ringBuffer, Serialization::Streamer<action>& streamer)
{
  auto settings = ringBuffer.getSettings();
  if (!streamer(settings.pointsPerSecond))
    return false;
  if (!streamer(settings.durationInSeconds))
    return false;

  if constexpr (action == Serialization::load) {
    ringBuffer.setSettings(settings);
  }

  return true;
}

/**
 * Sends data to a ring buffer, with custom logic to average it
 * */
template<class SampleType,
         class Preprocess,
         class Weight,
         class Accumulate,
         class Postprocess,
         class ElementType = float,
         class Allocator = std::allocator<ElementType>>
void sendToRingBuffer(RingBuffer<ElementType, Allocator>& ringBuffer,
                      SampleType** buffers,
                      Index numChannels,
                      Index startSample,
                      Index endSample,
                      Preprocess preprocess,
                      Weight weight,
                      Accumulate accumulate,
                      Postprocess postprocess)
{
  auto const currentWritePosition = ringBuffer.getWritePosition();
  auto const samplesPerPoint = ringBuffer.getSamplesPerPoint();
  auto const pointsPerSample = ringBuffer.getPointsPerSample();
  auto pointIndex = currentWritePosition;
  auto fistSampleOfPoint = startSample;
  while (true) {
    auto const numSamplesNeededForNextPoint = samplesPerPoint - ringBuffer.accumulatedSamples;
    auto const lastSampleOfPoint = static_cast<float>(fistSampleOfPoint) + numSamplesNeededForNextPoint;
    auto const lastSampleToAccumulate = FractionalIndex(std::min(static_cast<float>(endSample), lastSampleOfPoint));
    for (Index sample = fistSampleOfPoint; sample < lastSampleToAccumulate.integer; ++sample) {
      for (Index channel = 0; channel < numChannels; ++channel) {
        auto const sampleValue = preprocess(buffers[channel][sample], channel);
        ringBuffer.accumulator[channel] = accumulate(ringBuffer.accumulator[channel], sampleValue);
      }
    }
    bool const accumulationIsCompleted = lastSampleOfPoint == lastSampleToAccumulate.value;
    if (accumulationIsCompleted) {
      auto const nextSample = static_cast<Index>(std::ceil(lastSampleToAccumulate.value));
      if (nextSample < endSample && lastSampleToAccumulate.fractional > 0.f) {
        for (Index channel = 0; channel < numChannels; ++channel) {
          auto const sampleValue = preprocess(buffers[channel][nextSample], channel);
          auto const sampleValueWeighted = weight(buffers[channel][nextSample], lastSampleToAccumulate.fractional);
          auto const accumulatedValue = accumulate(ringBuffer.accumulator[channel], sampleValueWeighted);
          auto const accumulatedValueWeighted = weight(accumulatedValue, pointsPerSample);
          auto pointValue = postprocess(accumulatedValueWeighted);
          ringBuffer.at(channel, pointIndex) = pointValue;
          auto const firstValue = weight(ElementType(sampleValue), 1.f - lastSampleToAccumulate.fractional);
          ringBuffer.accumulator[channel] = firstValue;
        }
      }
      else {
        for (Index channel = 0; channel < numChannels; ++channel) {
          auto const pointValueWeighted = weight(ringBuffer.accumulator[channel], pointsPerSample);
          auto const pointValue = postprocess(pointValueWeighted);
          ringBuffer.at(channel, pointIndex) = pointValue;
          ringBuffer.accumulator[channel] = ElementType(0.f);
        }
      }
      ++pointIndex;
      pointIndex = ringBuffer.wrapIndex(pointIndex);
      fistSampleOfPoint = nextSample + 1;
      ringBuffer.accumulatedSamples = 0;
    }
    else {
      ringBuffer.accumulatedSamples += lastSampleToAccumulate.value - static_cast<float>(fistSampleOfPoint);
      break;
    }
  }
  ringBuffer.setWritePosition(pointIndex);
}

/**
 * Sends data to a ring buffer, using simple averaging
 * */
template<class SampleType, class ElementType = float, class Allocator = std::allocator<ElementType>>
void sendToRingBuffer(RingBuffer<ElementType, Allocator>& ringBuffer,
                      SampleType** buffers,
                      Index numChannels,
                      Index startSample,
                      Index endSample)
{
  sendToRingBuffer(
    ringBuffer,
    buffers,
    numChannels,
    startSample,
    endSample,
    [](ElementType value, Index channel) { return value; },
    [](ElementType value, float weight) { return value * weight; },
    [](ElementType accumulatedValue, SampleType value) { return accumulatedValue + static_cast<ElementType>(value); },
    [](ElementType value) { return value; });
}

/**
 * A struct to hold the elements of a waveform ring buffer, used to pass the averaged waveform profile to the user
 * interface.
 * */
template<class SampleType>
struct WaveformElement final
{
  float negative;
  float positive;
  explicit WaveformElement(float negative = 0.f, float positive = 0.f)
    : negative(negative)
    , positive(positive)
  {}
};

/**
 * A ring buffer to send the waveform profile to the user interface.
 * */
template<class SampleType, class Allocator = std::allocator<WaveformElement<SampleType>>>
using WaveformRingBuffer = RingBuffer<WaveformElement<SampleType>, Allocator>;

/**
 * Sends the waveform profile to a ring buffer.
 * */
template<class SampleType,
         class WaveformSampleType = float,
         class Allocator = std::allocator<WaveformElement<WaveformSampleType>>>
void sendToWaveformRingBuffer(WaveformRingBuffer<WaveformSampleType, Allocator>& ringBuffer,
                              SampleType** buffers,
                              Index numChannels,
                              Index startSample,
                              Index endSample)
{
  sendToRingBuffer(
    ringBuffer,
    buffers,
    numChannels,
    startSample,
    endSample,
    [](SampleType value, Index channel) { return value; },
    [](auto value, float weight) { return value; },
    [](WaveformElement<WaveformSampleType> accumulatedValue, SampleType value) {
      accumulatedValue.positive = std::max(accumulatedValue.positive, static_cast<WaveformSampleType>(value));
      accumulatedValue.negative = std::min(accumulatedValue.negative, static_cast<WaveformSampleType>(value));
      return accumulatedValue;
    },
    [](WaveformElement<WaveformSampleType> value) { return value; });
}

} // namespace unplug