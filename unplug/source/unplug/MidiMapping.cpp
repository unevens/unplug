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

#include "unplug/MidiMapping.hpp"
#include <cassert>

namespace unplug {
namespace detail {

MidiMappingSingleChannel::MidiMappingSingleChannel() {
  std::fill(midiMapping.begin(), midiMapping.end(), unmapped);
}

void MidiMappingSingleChannel::mapParameter(ParamIndex paramIndex, int controller) {
  midiMapping[static_cast<std::size_t>(controller)] = paramIndex;
}

ParamIndex MidiMappingSingleChannel::getParameter(int controller) const {
  return midiMapping[static_cast<std::size_t>(controller)];
}

} // namespace detail

void MidiMapping::mapParameter(ParamIndex paramIndex, int controller, int channel) {
  assert(channel < midiMappingByChannel.size());
  if (channel < midiMappingByChannel.size()) {
    midiMappingByChannel[channel].mapParameter(paramIndex, controller);
  }
}

void MidiMapping::mapParameter(ParamIndex paramIndex, int controller) {
  for (auto& channelMidiMapping : midiMappingByChannel) {
    channelMidiMapping.mapParameter(paramIndex, controller);
  }
}

ParamIndex MidiMapping::getParameter(int controller, int channel) const {
  assert(channel < midiMappingByChannel.size());
  if (channel < midiMappingByChannel.size()) {
    return midiMappingByChannel[channel].getParameter(controller);
  }
  else
    return unmapped;
}

} // namespace unplug