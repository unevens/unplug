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

#include <utility>

namespace unplug {

Oversampling createOversamplingUnit(SetupPluginFromDspUnit setupPlugin, oversimple::OversamplingSettings settings)
{
  using SupportedSampleTypes = oversimple::OversamplingSettings::SupportedScalarTypes;

  return DspUnit<oversimple::Oversampling, oversimple::OversamplingSettings>{
    std::move(setupPlugin),
    [](ContextInfo const& context, oversimple::OversamplingSettings& settings) {
      settings.context.numChannels = context.numIO.numOuts;
      settings.context.numSamplesPerBlock = context.maxAudioBlockSize;
      settings.context.supportedScalarTypes = context.precision == FloatingPointPrecision::float64
                                                ? SupportedSampleTypes::floatAndDouble
                                                : SupportedSampleTypes::onlyFloat;
    },
    settings,
    [](oversimple::Oversampling& oversampling) { return oversampling.getLatency(); }
  };
}

} // namespace unplug