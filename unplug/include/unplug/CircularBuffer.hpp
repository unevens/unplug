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
#include <algorithm>
#include <atomic>
#include <cmath>
#include <vector>

namespace unplug {

template<class Buffer>
class CircularBuffer
{
public:
  virtual float getPointsPerSecond() = 0;

  virtual float getDurationInSeconds() = 0;

  int getCircularIndex(int index) const
  {
    auto const bufferSize = static_cast<int>(buffer.size());
    return (((index) % bufferSize) + bufferSize) % bufferSize;
  }

  int getWritePosition() const
  {
    return writePosition.load(std::memory_order_acquire);
  }

  int getReadPosition() const
  {
    return getCircularIndex(getWritePosition() - readBlockSize);
  }

  int getReadBlockSize() const
  {
    return readBlockSize;
  }

  void incrementWritePosition(int amount)
  {
    writePosition.fetch_add(amount, std::memory_order_release);
  }

  void resize(float sampleRate, float refreshRate, int maxAudioBlockSize)
  {
    auto const pointsPerSecond = getPointsPerSecond();
    auto const durationInSeconds = getDurationInSeconds();
    auto const pointPerSample = pointsPerSecond / sampleRate;
    auto const maxWriteIncrementPerAudioBlock = pointPerSample * static_cast<float>(maxAudioBlockSize);
    readBlockSize = static_cast<int>(std::ceil(durationInSeconds * pointsPerSecond));
    auto const audioBlockDuration = static_cast<float>(maxAudioBlockSize) / sampleRate;
    auto const refreshTime = 1.f / refreshRate;
    auto const audioBlocksPerUserInterfaceRefreshTime = refreshTime / audioBlockDuration;
    resize(static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock)), audioBlocksPerUserInterfaceRefreshTime);
  }

  Buffer& getBuffer()
  {
    return buffer;
  }

  template<class T>
  void reset(T valueToResetTo)
  {
    std::fill(std::begin(buffer), std::end(buffer), valueToResetTo);
  }

private:
  void resize(int maxWriteIncrementPerAudioBlock, float audioBlocksPerUserInterfaceRefreshTime)
  {
    auto const bufferForProduction =
      static_cast<int>(std::ceil(maxWriteIncrementPerAudioBlock * audioBlocksPerUserInterfaceRefreshTime));
    auto const newSize = readBlockSize + bufferForProduction;
    resize(newSize);
  }

  void resize(int newSize)
  {
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

  std::atomic<int> writePosition{ 0 };
  int readBlockSize = 0;
  Buffer buffer;
};

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

private:
  CircularBuffersClass circularBuffers;
  static inline thread_local CircularBuffersClass* currentInstance = nullptr;
};

}; // namespace unplug