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

#include "unplug/ContextInfo.hpp"
#include "unplug/SetupPluginFromDsp.hpp"
#include <utility>

namespace unplug {

namespace detail {
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
  oversimple::Oversampling& getProcessor()
  {
    return oversampling;
  }

  /**
   * @return a const reference to the oversampling processor
   * */
  oversimple::Oversampling const& getProcessor() const
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
} // namespace detail

using SupportedSampleTypes = oversimple::OversamplingSettings::SupportedScalarTypes;

class Oversampling final
{
public:
  using Requirements = oversimple::OversamplingSettings::Requirements;
  using Context = oversimple::OversamplingSettings::Context;
  using Settings = oversimple::OversamplingSettings;

  explicit Oversampling(SetupPluginFromDspUnit setupPlugin, Settings const& settings = {})
    : oversampling{ std::make_unique<detail::Oversampling>(settings) }
    , setupPlugin{ std::move(setupPlugin) }
  {}

  Requirements const& getRequirementsOnUiThread() const
  {
    return oversampling.getOnNonRealtimeThread()->getSettings().requirements;
  }

  Requirements const& getRequirementsOnAudioThread()
  {
    return oversampling.getOnRealtimeThread()->getSettings().requirements;
  }

  bool setRequirements(Requirements const& newRequirements);

  void changeRequirements(std::function<void(Requirements&)> const& change);

  /**
   * @return a reference to the oversampling processor
   * */
  oversimple::Oversampling& getProcessorOnAudioThread()
  {
    return oversampling.getOnRealtimeThread()->getProcessor();
  }

  /**
   * @return a const reference to the oversampling processor
   * */
  oversimple::Oversampling const& getProcessorOnUiThread() const
  {
    return oversampling.getOnNonRealtimeThread()->getProcessor();
  }

  /**
   * Updates the processor in use on the real-time thread too the last version available.
   * @return a const reference to the oversampling processor
   * */
  oversimple::Oversampling& receiveChangesOnAudioThread()
  {
    return oversampling.receiveChangesOnRealtimeThread()->getProcessor();
  }

  /**
   * Updates the settings of the oversampling object to the audio processing context passed to the function. Also takes
   * a function to update the plugin with the new latency of the oversampling object. This overload lets you support
   * both float and double precision if you desire to.
   * @param context an instance of struct that describes the audio processing context
   * @return true if settings has changed, false otherwise
   * */
  bool setContext(Context const& context);

  /**
   * Updates the settings of an oversampling object to the audio processing context described by arguments passed to the
   * function.
   * Also takes a function to update the plugin with the new latency of the oversampling object.
   * This overload will only allocate the resources for the specified floating point type.
   * @param numChannels the number of channels that the oversampling should allocate resources for
   * @param maxAudioBlockSize the maximum number of samples that can be received during a processing call
   * @param floatingPointPrecision the floating point type that will be supported by the oversampling object
   * @return true if settings has changed, false otherwise
   * */
  bool setup(Index numChannels, Index maxAudioBlockSize, FloatingPointPrecision floatingPointPrecision);

  /**
   * Serialization helper for the oversampling settings.
   * @streamer the streamer that perform the serialization and the deserialization
   * */
  template<Serialization::Action action>
  bool serialization(Serialization::Streamer<action>& streamer)
  {
    auto settings = oversampling.getOnNonRealtimeThread()->getSettings();
    if (!streamer(settings.requirements.numScalarToVecUpsamplers))
      return false;
    if (!streamer(settings.requirements.numVecToVecUpsamplers))
      return false;
    if (!streamer(settings.requirements.numScalarToScalarUpsamplers))
      return false;
    if (!streamer(settings.requirements.numScalarToScalarDownsamplers))
      return false;
    if (!streamer(settings.requirements.numVecToScalarDownsamplers))
      return false;
    if (!streamer(settings.requirements.numVecToVecDownsamplers))
      return false;
    if (!streamer(settings.requirements.numScalarBuffers))
      return false;
    if (!streamer(settings.requirements.numInterleavedBuffers))
      return false;
    if (!streamer(settings.requirements.order))
      return false;
    if (!streamer(settings.requirements.linearPhase))
      return false;
    if (!streamer(settings.requirements.firTransitionBand))
      return false;

    if constexpr (action == Serialization::Action::load) {
      auto const haveSettingsChanged = [&](detail::Oversampling const& oversampling) {
        return !(settings == oversampling.getSettings());
      };
      auto const applySettings = [&](detail::Oversampling const& oversampling) {
        return std::make_unique<detail::Oversampling>(std::move(settings));
      };
      bool const hasChanged = oversampling.changeIf(applySettings, haveSettingsChanged);
      if (hasChanged) {
        setupPlugin.setLatency(getProcessorOnUiThread().getLatency());
      }
    }
    return true;
  }

private:
  lockfree::RealtimeObject<detail::Oversampling> oversampling;
  SetupPluginFromDspUnit const setupPlugin;
};

} // namespace unplug