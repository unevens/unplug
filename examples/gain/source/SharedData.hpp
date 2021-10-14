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
#include "unplug/ContextInfo.hpp"
#include "unplug/IO.hpp"
#include "unplug/NumIO.hpp"
#include "unplug/Oversampling.hpp"
#include "unplug/RingBuffer.hpp"
#include "unplug/Serialization.hpp"
#include "unplug/SharedDataWrapper.hpp"

struct SharedData final
{
  lockfree::RealtimeObject<unplug::RingBuffer<float>> levelRingBuffer;
  lockfree::RealtimeObject<unplug::WaveformRingBuffer<float>> waveformRingBuffer;
  unplug::Oversampling oversampling;

  SharedData(unplug::LatencyUpdater const& updateLatency)
    : levelRingBuffer{ std::make_unique<unplug::RingBuffer<float>>() }
    , waveformRingBuffer{ std::make_unique<unplug::WaveformRingBuffer<float>>() }
    , oversampling{ [=](int oversamplingLatency) { updateLatency(0, oversamplingLatency); } }
  {}

  void setup(unplug::ContextInfo const& context)
  {
    oversampling.setup(context.numIO.numOuts, context.maxAudioBlockSize, context.precision);
    unplug::setup(levelRingBuffer, context, [](unplug::RingBuffer<float>& ringBuffer) { ringBuffer.reset(0.f); });
    unplug::setup(waveformRingBuffer, context, [](unplug::WaveformRingBuffer<float>& ringBuffer) {
      ringBuffer.reset(unplug::WaveformElement<float>{ 0.f, 0.f });
    });
  }

  template<unplug::Serialization::Action action>
  bool serialization(unplug::Serialization::Streamer<action>& streamer)
  {
    using namespace unplug;
    if (!ringBufferSettingsSerialization(levelRingBuffer, streamer))
      return false;
    if (!ringBufferSettingsSerialization(waveformRingBuffer, streamer))
      return false;
    if (!oversampling.serialization(streamer))
      return false;
    return true;
  }
};

namespace unplug {
using SharedDataWrapped = SharedDataWrapper<SharedData>;
}