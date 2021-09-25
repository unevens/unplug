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
#include "implot.h"
#include "unplug/RingBuffer.hpp"

namespace unplug {

template<class SampleType, class Allocator>
void PlotRingBuffer(const char* name,
                    RingBuffer<SampleType, Allocator>& ringBuffer,
                    float xScale = 100.f,
                    float x0 = 0.f)
{
  ImPlot::BeginPlot(name);
  auto const numChannels = ringBuffer.getNumChannels();

  auto const offset = numChannels * ringBuffer.getReadPosition();
  auto const stride = ringBuffer.getNumChannels() * sizeof(SampleType);
  auto const readBlockSize = numChannels * ringBuffer.getReadBlockSize();
  auto const totalSize = static_cast<Index>(ringBuffer.getBuffer().size());
  auto buffer = ringBuffer.getBuffer().data();
  for (Index channel = 0; channel < numChannels; ++channel) {
    auto const start = offset + channel;
    auto const end = start + readBlockSize;
    auto contiguousEnd = std::min(end, totalSize);
    auto contiguousSize = contiguousEnd - start;
    ImPlot::PlotLine(name, buffer + start, contiguousSize / numChannels, xScale, x0, 0, stride);
    if (contiguousEnd != end) {
      auto const size = (end - totalSize) / numChannels;
      ImPlot::PlotLine(name, buffer, size, xScale, x0 + contiguousSize / numChannels, 0, stride);
    }
  }
  ImPlot::EndPlot();
}

} // namespace unplug