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
#include "unplug/DspUnit.hpp"
#include "oversimple/Oversampling.hpp"

namespace unplug {


using Oversampling = DspUnit<oversimple::Oversampling, oversimple::OversamplingSettings>;

Oversampling createOversamplingUnit(SetupPluginFromDspUnit setupPlugin,
                                    oversimple::OversamplingSettings settings_ = {});

template<Serialization::Action action>
bool serialization(Oversampling& oversampling, Serialization::Streamer<action>& streamer)
{
  auto settings = oversampling.getSettingsForEditing();
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

  if constexpr (action == Serialization::load) {
    oversampling.setSettings(settings);
  }

  return true;
}

} // namespace unplug