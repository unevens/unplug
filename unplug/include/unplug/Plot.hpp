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
#include "unplug/CircularBuffer.hpp"

namespace unplug {

template<class SampleType, class Allocator>
void PlotCircularBuffer(const char* name,
                        CircularBuffer<SampleType, Allocator>& circularBuffer,
                        float xScale = 1.f,
                        float x0 = 0.f)
{
  ImPlot::BeginPlot(name);
  auto buffer = circularBuffer.getBuffer().data();
  auto offset = circularBuffer.getReadPosition();
  auto size = circularBuffer.getReadBlockSize();
  ImPlot::PlotLine(name, buffer + offset, size, xScale, x0, 0, sizeof(SampleType) * circularBuffer.getNumChannels());
  ImPlot::EndPlot();
}

}; // namespace unplug