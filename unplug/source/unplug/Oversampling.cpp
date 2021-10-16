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

#include "unplug/Oversampling.hpp"

namespace unplug {

bool RealtimeOversampling::setContext(Context const& context)
{
  auto const isContextChanged = [&](Oversampling const& oversampling) {
    return oversampling.getSettings().context != context;
  };
  auto const adaptOversampling = [&](Oversampling const& oversampling) {
    auto settings = oversampling.getSettings();
    settings.context = context;
    return std::make_unique<Oversampling>(settings);
  };
  bool const hasChanged = oversampling.changeIf(adaptOversampling, isContextChanged);
  if (hasChanged) {
    setupPlugin.setLatency(getProcessorOnUiThread().getLatency());
  }
  return hasChanged;
}

bool RealtimeOversampling::setup(Index numChannels, Index maxAudioBlockSize, FloatingPointPrecision floatingPointPrecision)
{
  auto context = Context{};
  context.numChannels = numChannels;
  context.numSamplesPerBlock = maxAudioBlockSize;
  context.supportedScalarTypes = floatingPointPrecision == FloatingPointPrecision::float64
                                   ? SupportedSampleTypes::floatAndDouble
                                   : SupportedSampleTypes::onlyFloat;
  return setContext(context);
}

bool RealtimeOversampling::setRequirements(const RealtimeOversampling::Requirements& requirements)
{
  auto const haveRequirementsChanges = [&](Oversampling const& oversampling) {
    return oversampling.getSettings().requirements != requirements;
  };
  auto const adaptOversampling = [&](Oversampling const& oversampling) {
    auto settings = oversampling.getSettings();
    settings.requirements = requirements;
    return std::make_unique<Oversampling>(settings);
  };
  bool const hasChanged = oversampling.changeIf(adaptOversampling, haveRequirementsChanges);
  if (hasChanged) {
    setupPlugin.setLatency(getProcessorOnUiThread().getLatency());
    setupPlugin.setup();
  }
  return hasChanged;
}

void RealtimeOversampling::changeRequirements(std::function<void(Requirements&)>const& change) {
  auto requirements = getRequirementsOnUiThread();
  change(requirements);
  setRequirements(requirements);
}

} // namespace unplug