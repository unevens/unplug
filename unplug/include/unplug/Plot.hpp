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
                    RingBuffer<SampleType, Allocator>& circularBuffer,
                    float xScale = 1.f,
                    float x0 = 0.f)
{
  ImPlot::BeginPlot(name);
  auto buffer = circularBuffer.getBuffer().data();
  auto const numChannels = circularBuffer.getNumChannels();
  auto const offset = numChannels * circularBuffer.getReadPosition();
  auto const stride = circularBuffer.getNumChannels() * sizeof(SampleType);
  auto const size = circularBuffer.getReadBlockSize();
  for (Index channel = 0; channel < numChannels; ++channel) {
    ImPlot::PlotLine(name, buffer + offset + channel, size, xScale, x0, 0, stride);
  }
  ImPlot::EndPlot();
}

} // namespace unplug