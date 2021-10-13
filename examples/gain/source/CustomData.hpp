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
#include "lockfree/RealtimeObject.hpp"
#include "unplug/BlockSizeInfo.hpp"
#include "unplug/CustomDataWrapper.hpp"
#include "unplug/IO.hpp"
#include "unplug/NumIO.hpp"
#include "unplug/Oversampling.hpp"
#include "unplug/RingBuffer.hpp"
#include "unplug/Serialization.hpp"

struct PluginCustomData final
{
  lockfree::RealtimeObject<unplug::RingBuffer<float>> levelRingBuffer;
  lockfree::RealtimeObject<unplug::WaveformRingBuffer<float>> waveformRingBuffer;
  lockfree::RealtimeObject<unplug::Oversampling<double>> oversampling64;
  lockfree::RealtimeObject<unplug::Oversampling<float>> oversampling32;

  PluginCustomData()
    : levelRingBuffer{ std::make_unique<unplug::RingBuffer<float>>() }
    , waveformRingBuffer{ std::make_unique<unplug::WaveformRingBuffer<float>>() }
    , oversampling64{ std::unique_ptr<unplug::Oversampling<double>>{nullptr} }
    , oversampling32{ std::unique_ptr<unplug::Oversampling<float>>{nullptr} }
  {}

  void setFloatingPointPrecision(bool doublePrecision)
  {
    if (doublePrecision) {
      if (!oversampling64.getFromNonRealtimeThread()) {
        auto const oversampling32_ = oversampling32.getFromNonRealtimeThread();
        auto settings = oversampling32_ ? oversampling32_->getSettings() : oversimple::OversamplingSettings{};
        oversampling64.set(std::make_unique<unplug::Oversampling<double>>(settings));
        oversampling32.set({});
      }
    }
    else {
      if (!oversampling32.getFromNonRealtimeThread()) {
        auto const oversampling64_ = oversampling64.getFromNonRealtimeThread();
        auto settings = oversampling64_ ? oversampling64_->getSettings() : oversimple::OversamplingSettings{};
        oversampling32.set(std::make_unique<unplug::Oversampling<float>>(settings));
        oversampling64.set({});
      }
    }
  }

  void setBlockSizeInfo(unplug::BlockSizeInfo const& blockSizeInfo,
                        std::function<void(uint64_t newLatency)> const& checkLatency)
  {
    unplug::setBlockSizeInfo(
      levelRingBuffer, blockSizeInfo, [](unplug::RingBuffer<float>& ringBuffer) { ringBuffer.reset(0.f); });
    unplug::setBlockSizeInfo(waveformRingBuffer, blockSizeInfo, [](unplug::WaveformRingBuffer<float>& ringBuffer) {
      ringBuffer.reset(unplug::WaveformElement<float>{ 0.f, 0.f });
    });
    unplug::setBlockSizeInfo(
      oversampling64, blockSizeInfo.numIO.numOuts, blockSizeInfo.maxAudioBlockSize, checkLatency);
    unplug::setBlockSizeInfo(
      oversampling32, blockSizeInfo.numIO.numOuts, blockSizeInfo.maxAudioBlockSize, checkLatency);
  }

  template<unplug::Serialization::Action action>
  bool serialization(unplug::Serialization::Streamer<action>& streamer,
                     std::function<void(uint64_t newLatency)> const& checkLatency)
  {
    using namespace unplug;
    if (!ringBufferSettingsSerialization(levelRingBuffer, streamer))
      return false;
    if (!ringBufferSettingsSerialization(waveformRingBuffer, streamer))
      return false;
    bool const hasSerializedFromOversampling64 =
      oversamplingSettingsSerialization(oversampling64, streamer, checkLatency);
    if (hasSerializedFromOversampling64) {
      return true;
    }
    else {
      return oversamplingSettingsSerialization(oversampling32, streamer, checkLatency);
    }
  }
};

namespace unplug {
using CustomData = CustomDataWrapper<PluginCustomData>;
}