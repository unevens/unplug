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
#include "Serialization.hpp"
#include "oversimple/Oversampling.hpp"

namespace unplug {

template<class SampleType>
class Oversampling final
{
public:
  using Settings = oversimple::OversamplingSettings;
  using Impl = oversimple::Oversampling<SampleType>;

  explicit Oversampling(Settings settings_ = Settings{})
    : settings(settings_)
    , oversampling(settings)
  {}

  Oversampling(Oversampling const& other)
    : settings(other.settings)
    , oversampling(other.settings)
  {}

  Impl& get()
  {
    return oversampling;
  }

  Impl const& get() const
  {
    return oversampling;
  }

  Settings const& getSettings() const
  {
    return settings;
  }

private:
  Settings settings;
  Impl oversampling;
};

template<class SampleType>
bool setBlockSizeInfo(lockfree::RealtimeObject<Oversampling<SampleType>>& rtOversampling,
                      Index numChannels, Index maxAudioBlockSize,
                      std::function<void(uint64_t)> const& checkLatency)
{
  auto const isSizeInfoChanged = [&](Oversampling<SampleType> const& oversampling) {
    bool const isNumChannelsChanged = numChannels != oversampling.getSettings().numChannels;
    bool const isBlockSizeChanged = maxAudioBlockSize != oversampling.getSettings().numSamplesPerBlock;
    return isNumChannelsChanged || isBlockSizeChanged;
  };
  auto const adaptOversampling = [&](Oversampling<SampleType> const& oversampling) {
    auto settings = oversampling.getSettings();
    settings.numChannels = numChannels;
    settings.numSamplesPerBlock = maxAudioBlockSize;
    return std::make_unique<Oversampling<SampleType>>(std::move(settings));
  };
  bool const hasChanged = rtOversampling.changeIf(adaptOversampling, isSizeInfoChanged);
  if (hasChanged) {
    auto oversampling = rtOversampling.getFromNonRealtimeThread();
    checkLatency(oversampling->get().getLatency());
  }
  return hasChanged;
}

template<class SampleType, Serialization::Action action>
bool oversamplingSettingsSerialization(lockfree::RealtimeObject<Oversampling<SampleType>>& rtOversampling,
                                       Serialization::Streamer<action>& streamer,
                                       std::function<void(uint64_t)> const& checkLatency)
{
  auto oversampling = rtOversampling.getFromNonRealtimeThread();
  if(!oversampling)
    return false;
  auto settings = oversampling->getSettings();
  if (!streamer(settings.numChannels))
    return false;
  if (!streamer(settings.numScalarToVecUpsamplers))
    return false;
  if (!streamer(settings.numVecToVecUpsamplers))
    return false;
  if (!streamer(settings.numScalarToScalarUpsamplers))
    return false;
  if (!streamer(settings.numScalarToScalarDownsamplers))
    return false;
  if (!streamer(settings.numVecToScalarDownsamplers))
    return false;
  if (!streamer(settings.numVecToVecDownsamplers))
    return false;
  if (!streamer(settings.numScalarBuffers))
    return false;
  if (!streamer(settings.numInterleavedBuffers))
    return false;
  if (!streamer(settings.order))
    return false;
  if (!streamer(settings.linearPhase))
    return false;
  if (!streamer(settings.numSamplesPerBlock))
    return false;
  if (!streamer(settings.firTransitionBand))
    return false;

  if constexpr (action == Serialization::Action::load) {
    auto const haveSettingsChanged = [&](Oversampling<SampleType> const& oversampling) {
      return !(settings == oversampling.getSettings());
    };
    auto const applySettings = [&](Oversampling<SampleType> const& oversampling) {
      return std::make_unique<Oversampling<SampleType>>(std::move(settings));
    };
    bool const hasChanged = rtOversampling.changeIf(applySettings, haveSettingsChanged);
    if (hasChanged) {
      oversampling = rtOversampling.getFromNonRealtimeThread();
      checkLatency(oversampling->get().getLatency());
    }
  }
  return true;
}

} // namespace unplug