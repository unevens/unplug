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
#include "lockfree/RealtimeObject.hpp"
#include "oversimple/Oversampling.hpp"
#include "unplug/BlockSizeInfo.hpp"

namespace unplug {

/**
 * A class that holds together an oversimple::Oversampling processor and its settings, ready to be used wrapped in a
 * lockfree::RealtimeObject
 */
class Oversampling final
{
public:
  using Settings = oversimple::OversamplingSettings;

  /**
   * Constructor.
   * @param settings the oversampling settings used to create the processor
   * */
  explicit Oversampling(Settings settings = Settings{})
    : settings(settings)
    , oversampling(settings)
  {}

  /**
   * Copy-constructor. Copies the settings and create a new processor from them.
   * */
  Oversampling(Oversampling const& other)
    : settings(other.settings)
    , oversampling(other.settings)
  {}

  /**
   * @return a reference to the oversampling processor
   * */
  oversimple::Oversampling& getProccessor()
  {
    return oversampling;
  }

  /**
   * @return a const reference to the oversampling processor
   * */
  oversimple::Oversampling const& getProccessor() const
  {
    return oversampling;
  }

  /**
   * @return a reference to the oversampling settings
   * */
  Settings const& getSettings() const
  {
    return settings;
  }

private:
  Settings settings;
  oversimple::Oversampling oversampling;
};

using SupportedSampleTypes = oversimple::OversamplingSettings::SupportedScalarTypes;

/**
 * Updates the settings of an oversampling object to match the requirements passed as arguments to the functions.
 * Also takes a function to update the plugin with the new latency of the oversampling object.
 * This overload lets you support both float and double precision if you desire to.
 * @param rtOversampling an Oversampling object wrapped by a RealtimeObject
 * @param numChannels the number of channels that the oversampling should allocate resources for
 * @param supportedSampleTypes weather the oversampling object should support only single precision, only double, or
 * both. Each type will allocate its own resources.
 * @updateLatency a function that will be called if the oversampling object has changed an to which the latency of the
 * oversampling object will be passed
 * */
bool setBlockSizeInfo(lockfree::RealtimeObject<Oversampling>& rtOversampling,
                      Index numChannels,
                      Index maxAudioBlockSize,
                      SupportedSampleTypes supportedSampleTypes,
                      std::function<void(uint64_t)> const& updateLatency);

/**
 * Updates the settings of an oversampling object to match the requirements passed as arguments to the functions.
 * Also takes a function to update the plugin with the new latency of the oversampling object.
 * This overload will only allocate the resources for the specified floating point type.
 * @param rtOversampling an Oversampling object wrapped by a RealtimeObject
 * @param numChannels the number of channels that the oversampling should allocate resources for
 * @param floatingPointPrecision the floating point type that will be supported by the oversampling object
 * @updateLatency a function that will be called if the oversampling object has changed an to which the latency of the
 * oversampling object will be passed
 * */
bool setBlockSizeInfo(lockfree::RealtimeObject<Oversampling>& rtOversampling,
                      Index numChannels,
                      Index maxAudioBlockSize,
                      FloatingPointPrecision floatingPointPrecision,
                      std::function<void(uint64_t)> const& updateLatency);

/**
 * Serialization helper for the oversampling settings.
 * @param rtOversampling an Oversampling object wrapped by a RealtimeObject
 * @streamer the streamer that perform the serialization and the deserialization
 * @updateLatency a function that will be called if the oversampling object has changed an to which the latency of the
 * oversampling object will be passed
 * */
template<Serialization::Action action>
bool oversamplingSettingsSerialization(lockfree::RealtimeObject<Oversampling>& rtOversampling,
                                       Serialization::Streamer<action>& streamer,
                                       std::function<void(uint64_t)> const& updateLatency)
{
  auto oversampling = rtOversampling.getFromNonRealtimeThread();
  if (!oversampling)
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
    auto const haveSettingsChanged = [&](Oversampling const& oversampling) {
      return !(settings == oversampling.getSettings());
    };
    auto const applySettings = [&](Oversampling const& oversampling) {
      return std::make_unique<Oversampling>(std::move(settings));
    };
    bool const hasChanged = rtOversampling.changeIf(applySettings, haveSettingsChanged);
    if (hasChanged) {
      oversampling = rtOversampling.getFromNonRealtimeThread();
      updateLatency(oversampling->getProccessor().getLatency());
    }
  }
  return true;
}

} // namespace unplug