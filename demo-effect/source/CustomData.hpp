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
#include "unplug/BlockSizeInfo.hpp"
#include "unplug/CustomDataWrapper.hpp"
#include "unplug/IO.hpp"
#include "unplug/NumIO.hpp"
#include "lockfree/PreAllocated.hpp"
#include "unplug/RingBuffer.hpp"

struct PluginCustomData final
{
  lockfree::PreAllocated<unplug::RingBuffer<float>> levelRingBuffer;
  lockfree::PreAllocated<unplug::WaveformRingBuffer<float>> waveformRingBuffer;

  PluginCustomData(){
    levelRingBuffer.set(std::make_unique<unplug::RingBuffer<float>>());
    waveformRingBuffer.set(std::make_unique<unplug::WaveformRingBuffer<float>>());
  }

  void setBlockSizeInfo(unplug::BlockSizeInfo const& blockSizeInfo)
  {
    unplug::setBlockSizeInfo(levelRingBuffer, blockSizeInfo, [](unplug::RingBuffer<float>& ringBuffer) { ringBuffer.reset(0.f); });
    unplug::setBlockSizeInfo(waveformRingBuffer, blockSizeInfo, [](unplug::WaveformRingBuffer<float>& ringBuffer) {
      ringBuffer.reset(unplug::WaveformElement<float>{ 0.f, 0.f });
    });
  }
};

namespace unplug {
using CustomData = CustomDataWrapper<PluginCustomData>;
}