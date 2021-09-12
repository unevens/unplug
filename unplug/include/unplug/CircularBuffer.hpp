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
#include "unplug/NumIO.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <vector>

namespace unplug {

template<class ElementType, class Allocator = std::allocator<ElementType>>
class CircularBuffer
{
public:
  using Buffer = std::vector<ElementType, Allocator>;

  Buffer& getBuffer()
  {
    return buffer;
  }

  auto& at(Index channel, Index pointIndex)
  {
    return buffer[numChannels * pointIndex + channel];
  }

  Index getCircularIndex(Index index) const
  {
    auto const bufferSize = static_cast<int>(buffer.size());
    return (((index) % bufferSize) + bufferSize) % bufferSize;
  }

  Index getWritePosition() const
  {
    return static_cast<Index>(writePosition.load(std::memory_order_acquire));
  }

  Index getReadPosition() const
  {
    return getCircularIndex(getWritePosition() - readBlockSize);
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

  void incrementWritePosition(Index amount)
  {
    writePosition.fetch_add(static_cast<int>(amount), std::memory_order_release);
  }

  template<class T>
  void reset(T valueToResetTo)
  {
    std::fill(std::begin(buffer), std::end(buffer), valueToResetTo);
  }

  void resize(float sampleRate, float refreshRate, Index maxAudioBlockSize, NumIO numIO)
  {
    numChannels = choseNumChannels(numIO);
    auto const pointsPerSecond = getPointsPerSecond();
    auto const durationInSeconds = getDurationInSeconds();
    pointsPerSample = pointsPerSecond / sampleRate;
    samplesPerPoint = 1.f / pointsPerSample;
    auto const maxWriteIncrementPerAudioBlock = numChannels * pointsPerSample * static_cast<float>(maxAudioBlockSize);
    readBlockSize = numChannels * static_cast<int>(std::ceil(durationInSeconds * pointsPerSecond));
    auto const audioBlockDuration = static_cast<float>(maxAudioBlockSize) / sampleRate;
    auto const refreshTime = 1.f / refreshRate;
    auto const audioBlocksPerUserInterfaceRefreshTime = refreshTime / audioBlockDuration;
    resize(static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock)), audioBlocksPerUserInterfaceRefreshTime);
  }

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
        std::copy(std::begin(buffer) + delta, std::begin(buffer) + currentWritePosition, std::begin(buffer));
      }
    }
    buffer.resize(newSize);
  }

  Index numChannels = 1;
  std::atomic<unsigned int> writePosition{ 0 };
  Index readBlockSize = 0;
  float pointsPerSample = 1.f;
  float samplesPerPoint = 1.f;
  Buffer buffer;
};

template<class SampleType,
         class PreprocessValue,
         class PostprocessValue,
         class ElementType = float,
         class Allocator = std::allocator<ElementType>>
void sendToCircularBuffer(unplug::CircularBuffer<ElementType, Allocator>& circularBuffer,
                          SampleType** buffers,
                          Index numChannels,
                          Index startSample,
                          Index endSample,
                          PreprocessValue preprocessValue,
                          PostprocessValue postprocessValue)
{
  using FractionalIndex = unplug::FractionalIndex;
  auto currentWritePosition = circularBuffer.getWritePosition();
  auto const samplesPerPoint = FractionalIndex(circularBuffer.getSamplesPerPoint());
  auto const invSamplePerPoint = 1.f / samplesPerPoint.value;
  auto const numPoints =
    FractionalIndex(static_cast<float>(endSample - startSample) * circularBuffer.getPointsPerSample());
  for (Index channel = 0; channel < numChannels; ++channel) {
    for (Index pointIndex = currentWritePosition; pointIndex < currentWritePosition + numPoints.integer; ++pointIndex) {
      auto const firstSampleIndex = FractionalIndex(static_cast<float>(pointIndex) * samplesPerPoint.value);
      auto pointValue = ElementType(0.f);
      auto firstSampleValue = buffers[channel][firstSampleIndex.integer];
      pointValue += (1.f - firstSampleIndex.fractional) * preprocessValue(firstSampleValue);
      for (Index offset = 1; offset <= samplesPerPoint.integer; ++offset) {
        auto sampleValue = buffers[channel][firstSampleIndex.integer + offset];
        pointValue += preprocessValue(sampleValue);
      }
      auto const lastSampleIndex = FractionalIndex(firstSampleIndex.value + samplesPerPoint.value);
      assert(lastSampleIndex.integer < endSample);
      auto const lastSampleValue = buffers[channel][std::min(lastSampleIndex.integer, endSample - 1)];
      pointValue += lastSampleIndex.fractional * preprocessValue(lastSampleValue);
      circularBuffer.at(channel, pointIndex) = postprocessValue(pointValue * invSamplePerPoint);
    }
  }
  circularBuffer.incrementWritePosition(numPoints.integer);
}

template<class SampleType, class ElementType = float, class Allocator = std::allocator<ElementType>>
void sendToCircularBuffer(unplug::CircularBuffer<ElementType, Allocator>& circularBuffer,
                          SampleType** buffers,
                          Index numChannels,
                          Index startSample,
                          Index endSample)
{
  sendToCircularBuffer(
    circularBuffer,
    buffers,
    numChannels,
    startSample,
    endSample,
    [](SampleType value) { return static_cast<ElementType>(value); },
    [](SampleType value) { return static_cast<ElementType>(value); });
}

template<class CircularBuffersClass>
class TCircularBufferStorage
{
public:
  void setCurrent()
  {
    currentInstance = &circularBuffers;
  }

  static CircularBuffersClass* getCurrent()
  {
    return currentInstance;
  }

  CircularBuffersClass& get()
  {
    return circularBuffers;
  }

  TCircularBufferStorage() = default;

  TCircularBufferStorage(TCircularBufferStorage const&) = delete;

  TCircularBufferStorage& operator=(TCircularBufferStorage const&) = delete;

private:
  CircularBuffersClass circularBuffers;
  static inline thread_local CircularBuffersClass* currentInstance = nullptr;
};
}; // namespace unplug