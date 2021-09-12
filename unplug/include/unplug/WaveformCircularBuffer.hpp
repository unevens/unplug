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
#include "unplug/CircularBuffer.hpp"

namespace unplug {

struct WaveformElement final
{
  float positive = 0.f;
  float negative = 0.f;
};

using WaveformCircularBuffer = unplug::CircularBuffer<WaveformElement>;

template<class SampleType>
void sendToWaveformCircularBuffer(WaveformCircularBuffer& waveform,
                                  SampleType** buffers,
                                  Index numChannels,
                                  Index startSample,
                                  Index endSample)
{
  using FractionalIndex = unplug::FractionalIndex;
  auto currentWritePosition = waveform.getWritePosition();
  auto const samplesPerPoint = FractionalIndex(waveform.getSamplesPerPoint());
  auto const numPoints = FractionalIndex(static_cast<float>(endSample - startSample) * waveform.getPointsPerSample());
  for (Index channel = 0; channel < numChannels; ++channel) {
    for (Index pointIndex = currentWritePosition; pointIndex < currentWritePosition + numPoints.integer; ++pointIndex) {
      auto const firstSampleIndex = FractionalIndex(static_cast<float>(pointIndex) * samplesPerPoint.value);
      auto pointValue = unplug::WaveformElement{ 0.f, 0.f };
      auto firstSampleValue = static_cast<float>(buffers[channel][firstSampleIndex.integer]);
      pointValue.positive = std::max(pointValue.positive, firstSampleValue);
      pointValue.negative = std::min(pointValue.negative, firstSampleValue);
      for (Index offset = 1; offset <= samplesPerPoint.integer; ++offset) {
        auto sampleValue = static_cast<float>(buffers[channel][firstSampleIndex.integer + offset]);
        pointValue.positive = std::max(pointValue.positive, sampleValue);
        pointValue.negative = std::min(pointValue.negative, sampleValue);
      }
      auto const lastSampleIndex = FractionalIndex(firstSampleIndex.value + samplesPerPoint.value);
      assert(lastSampleIndex.integer < endSample);
      auto const lastSampleValue =
        static_cast<float>(buffers[channel][std::min(lastSampleIndex.integer, endSample - 1)]);
      pointValue.positive = std::max(pointValue.positive, lastSampleValue);
      pointValue.negative = std::min(pointValue.negative, lastSampleValue);
      waveform.at(channel, pointIndex) = pointValue;
    }
  }
  waveform.incrementWritePosition(numPoints.integer);
}

} // namespace unplug