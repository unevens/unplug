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

bool setBlockSizeInfo(lockfree::RealtimeObject<Oversampling>& rtOversampling,
                      Index numChannels,
                      Index maxAudioBlockSize,
                      SupportedSampleTypes supportedSampleTypes,
                      std::function<void(uint64_t)> const& updateLatency)
{
  auto const isSizeInfoChanged = [&](Oversampling const& oversampling) {
    bool const isNumChannelsChanged = numChannels != oversampling.getSettings().numChannels;
    bool const isBlockSizeChanged = maxAudioBlockSize != oversampling.getSettings().numSamplesPerBlock;
    bool const supportedSampleTypeChanged = supportedSampleTypes != oversampling.getSettings().supportedScalarTypes;
    return isNumChannelsChanged || isBlockSizeChanged || supportedSampleTypeChanged;
  };
  auto const adaptOversampling = [&](Oversampling const& oversampling) {
    auto settings = oversampling.getSettings();
    settings.numChannels = numChannels;
    settings.numSamplesPerBlock = maxAudioBlockSize;
    settings.supportedScalarTypes = supportedSampleTypes;
    return std::make_unique<Oversampling>(std::move(settings));
  };
  bool const hasChanged = rtOversampling.changeIf(adaptOversampling, isSizeInfoChanged);
  if (hasChanged) {
    auto oversampling = rtOversampling.getFromNonRealtimeThread();
    updateLatency(oversampling->getProccessor().getLatency());
  }
  return hasChanged;
}

bool setBlockSizeInfo(lockfree::RealtimeObject<Oversampling>& rtOversampling,
                      Index numChannels,
                      Index maxAudioBlockSize,
                      FloatingPointPrecision floatingPointPrecision,
                      std::function<void(uint64_t)> const& updateLatency)
{
  auto supportedSampleTypes = floatingPointPrecision == FloatingPointPrecision::float64
                                ? SupportedSampleTypes::onlyDouble
                                : SupportedSampleTypes::onlyFloat;
  return setBlockSizeInfo(rtOversampling, numChannels, maxAudioBlockSize, supportedSampleTypes, updateLatency);
}

} // namespace unplug