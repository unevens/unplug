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
#include "unplug/IO.hpp"
#include "unplug/NumIO.hpp"
#include "unplug/Oversampling.hpp"
#include "unplug/RingBuffer.hpp"
#include "unplug/Serialization.hpp"
#include "unplug/SharedDataWrapper.hpp"

struct SharedData final
{
  unplug::Oversampling oversampling;
  unplug::RingBufferUnit<float> levelRingBuffer;
  unplug::WaveformRingBufferUnit<float> waveformRingBuffer;

  explicit SharedData(unplug::SetupPluginFromDsp const& setupPlugin)
    : oversampling{ unplug::createOversamplingUnit(unplug::SetupPluginFromDspUnit(setupPlugin, 0),
                                                   oversamplingSettings()) }
    , levelRingBuffer{ std::move(unplug::createRingBufferUnit<float>(
        unplug::SetupPluginFromDspUnit(setupPlugin, unplug::SetupPluginFromDspUnit::noLatencyUnit))) }
    , waveformRingBuffer{ unplug::createWaveformRingBufferUnit<float>(
        unplug::SetupPluginFromDspUnit(setupPlugin, unplug::SetupPluginFromDspUnit::noLatencyUnit)) }
  {}

  void setup(unplug::ContextInfo const& context)
  {
    oversampling.setContext(context);
    levelRingBuffer.setContext(context);
    waveformRingBuffer.setContext(context);
  }

  template<unplug::Serialization::Action action>
  bool serialization(unplug::Serialization::Streamer<action>& streamer)
  {
    using namespace unplug;
    if (!unplug::serialization(levelRingBuffer, streamer))
      return false;
    if (!unplug::serialization(waveformRingBuffer, streamer))
      return false;
    if (!unplug::serialization(oversampling, streamer))
      return false;
    return true;
  }

private:
  static oversimple::OversamplingSettings oversamplingSettings()
  {
    auto settings = oversimple::OversamplingSettings{};
    settings.requirements.numScalarToScalarUpsamplers = 1;
    settings.requirements.numScalarToScalarDownsamplers = 1;
    return settings;
  }
};

namespace unplug {
using SharedDataWrapped = SharedDataWrapper<SharedData>;
}