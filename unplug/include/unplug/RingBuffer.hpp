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
#include "unplug/Index.hpp"
#include "unplug/Math.hpp"
#include <vector>

namespace unplug {
template<class ElementType, class Allocator = std::allocator<ElementType>>
class RingBuffer
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
    return (((index) % bufferSize) + bufferSize) % bufferSize;
  }

  Index getWritePosition() const
  {
    return static_cast<Index>(writePosition.load(std::memory_order_acquire));
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

  float getSecondsPerPoint() const
  {
    return 1.f / getPointsPerSecond();
  }

  Index getNumChannels() const
  {
    return numChannels;
  }

  void setWritePosition(Index newWritePosition)
  {
    writePosition.store(static_cast<unsigned int>(newWritePosition), std::memory_order_release);
  }

  Index getBufferCapaccity() const
  {
    return bufferCapacity;
  }

  template<class T>
  void reset(T valueToResetTo)
  {
    std::fill(std::begin(buffer), std::end(buffer), valueToResetTo);
    std::fill(std::begin(accumulator), std::end(accumulator), valueToResetTo);
  }

  void resize(float sampleRate, float refreshRate, Index maxAudioBlockSize, NumIO numIO)
  {
    numChannels = choseNumChannels(numIO);
    accumulator.resize(numChannels);
    auto const pointsPerSecond = getPointsPerSecond();
    auto const durationInSeconds = getDurationInSeconds();
    samplesPerPoint = sampleRate / pointsPerSecond;
    pointsPerSample = 1.f / samplesPerPoint;
    auto const maxWriteIncrementPerAudioBlock = pointsPerSample * static_cast<float>(maxAudioBlockSize);
    readBlockSize = static_cast<int>(std::ceil(durationInSeconds * pointsPerSecond));
    auto const audioBlockDuration = static_cast<float>(maxAudioBlockSize) / sampleRate;
    auto const refreshTime = 1.f / refreshRate;
    auto const audioBlocksPerUserInterfaceRefreshTime = refreshTime / audioBlockDuration;
    resize(static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock)), audioBlocksPerUserInterfaceRefreshTime);
  }

  std::vector<ElementType, Allocator> accumulator;
  float accumulatedSamples = 0.f;

private:
  // todo: make these editable at runtime (allocating and deallocating on ui and sending buffer to audio thread)
  virtual float getPointsPerSecond() const
  {
    return 128;
  }

  virtual float getDurationInSeconds() const
  {
    return 8;
  }

  virtual Index choseNumChannels(NumIO numIO) const
  {
    return numIO.numOuts;
  }

  void resize(Index maxWriteIncrementPerAudioBlock, float audioBlocksPerUserInterfaceRefreshTime)
  {
    auto const bufferForProduction =
      static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock * audioBlocksPerUserInterfaceRefreshTime));
    auto const newSize = readBlockSize + bufferForProduction;
    resize(newSize);
  }

  void resize(Index newSize)
  {
    assert(newSize <= std::numeric_limits<unsigned int>::max());
    auto const currentWritePosition = writePosition.load(std::memory_order_acquire);
    if (newSize <= currentWritePosition) {
      writePosition.store(0, std::memory_order_release);
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

  Index numChannels = 1;
  Index bufferCapacity = 0;
  std::atomic<unsigned int> writePosition{ 0 };
  Index readBlockSize = 0;
  float pointsPerSample = 1.f;
  float samplesPerPoint = 1;
  Buffer buffer;
};

template<class SampleType,
         class Accumulate,
         class Scale,
         class ElementType = float,
         class Allocator = std::allocator<ElementType>>
void sendToRingBuffer(unplug::RingBuffer<ElementType, Allocator>& circularBuffer,
                      SampleType** buffers,
                      Index numChannels,
                      Index startSample,
                      Index endSample,
                      Accumulate accumulate,
                      Scale scale)
{
  auto const currentWritePosition = circularBuffer.getWritePosition();
  auto const samplesPerPoint = circularBuffer.getSamplesPerPoint();
  auto const pointsPerSample = circularBuffer.getPointsPerSample();
  auto pointIndex = currentWritePosition;
  auto fistSampleOfPoint = startSample;
  while (true) {
    auto const numSamplesNeededForNextPoint = samplesPerPoint - circularBuffer.accumulatedSamples;
    auto const lastSampleOfPoint = static_cast<float>(fistSampleOfPoint) + numSamplesNeededForNextPoint;
    auto const lastSampleToAccumulate = FractionalIndex(std::min(static_cast<float>(endSample), lastSampleOfPoint));
    for (Index sample = fistSampleOfPoint; sample < lastSampleToAccumulate.integer; ++sample) {
      for (Index channel = 0; channel < numChannels; ++channel) {
        auto const sampleValue = buffers[channel][sample];
        circularBuffer.accumulator[channel] = accumulate(circularBuffer.accumulator[channel], sampleValue, channel);
      }
    }
    bool const accumulationIsCompleted = lastSampleOfPoint == lastSampleToAccumulate.value;
    if (accumulationIsCompleted) {
      auto const nextSample = static_cast<Index>(std::ceil(lastSampleToAccumulate.value));
      if (nextSample < endSample && lastSampleToAccumulate.fractional > 0.f) {
        for (Index channel = 0; channel < numChannels; ++channel) {
          auto const sampleValue = buffers[channel][nextSample];
          auto const accumulatedValue =
            accumulate(circularBuffer.accumulator[channel], sampleValue * lastSampleToAccumulate.fractional, channel);
          auto pointValue = scale(pointsPerSample * accumulatedValue);
          circularBuffer.at(channel, pointIndex) = pointValue;
          circularBuffer.accumulator[channel] = sampleValue * (1.f - lastSampleToAccumulate.fractional);
        }
      }
      else {
        for (Index channel = 0; channel < numChannels; ++channel) {
          auto pointValue = scale(pointsPerSample * circularBuffer.accumulator[channel]);
          circularBuffer.at(channel, pointIndex) = pointValue;
          circularBuffer.accumulator[channel] = ElementType(0.f);
        }
      }
      ++pointIndex;
      pointIndex = circularBuffer.wrapIndex(pointIndex);
      fistSampleOfPoint = nextSample + 1;
      circularBuffer.accumulatedSamples = 0;
    }
    else {
      circularBuffer.accumulatedSamples += lastSampleToAccumulate.value - static_cast<float>(fistSampleOfPoint);
      break;
    }
  }
  circularBuffer.setWritePosition(pointIndex);
}

template<class SampleType, class ElementType = float, class Allocator = std::allocator<ElementType>>
void sendToRingBuffer(unplug::RingBuffer<ElementType, Allocator>& circularBuffer,
                      SampleType** buffers,
                      Index numChannels,
                      Index startSample,
                      Index endSample)
{
  sendToRingBuffer(
    circularBuffer,
    buffers,
    numChannels,
    startSample,
    endSample,
    [](ElementType accumulatedValue, SampleType value, Index channel) {
      return accumulatedValue + static_cast<ElementType>(value);
    },
    [](SampleType value) { static_cast<ElementType>(value); });
}

} // namespace unplug