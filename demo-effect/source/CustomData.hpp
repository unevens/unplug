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
#include "lockfree/PreAllocated.hpp"
#include "unplug/BlockSizeInfo.hpp"
#include "unplug/CustomDataWrapper.hpp"
#include "unplug/IO.hpp"
#include "unplug/NumIO.hpp"
#include "unplug/RingBuffer.hpp"
#include "unplug/Serialization.hpp"

struct PluginCustomData final
{
  lockfree::PreAllocated<unplug::RingBuffer<float>> levelRingBuffer;
  lockfree::PreAllocated<unplug::WaveformRingBuffer<float>> waveformRingBuffer;

  PluginCustomData()
    : levelRingBuffer{ std::make_unique<unplug::RingBuffer<float>>() }
    , waveformRingBuffer{ std::make_unique<unplug::WaveformRingBuffer<float>>() }
  {}

  void setBlockSizeInfo(unplug::BlockSizeInfo const& blockSizeInfo)
  {
    unplug::setBlockSizeInfo(
      levelRingBuffer, blockSizeInfo, [](unplug::RingBuffer<float>& ringBuffer) { ringBuffer.reset(0.f); });
    unplug::setBlockSizeInfo(waveformRingBuffer, blockSizeInfo, [](unplug::WaveformRingBuffer<float>& ringBuffer) {
      ringBuffer.reset(unplug::WaveformElement<float>{ 0.f, 0.f });
    });
  }

  template<unplug::Serialization::Action action>
  bool serialization(unplug::Serialization::Streamer<action>& streamer)
  {
    levelRingBuffer.getFromNonRealtimeThread()->settingsSerialization(streamer);
    waveformRingBuffer.getFromNonRealtimeThread()->settingsSerialization(streamer);
    return true;
  }
};

namespace unplug {
using CustomData = CustomDataWrapper<PluginCustomData>;
}