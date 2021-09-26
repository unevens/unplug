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

template<class ElementType, class Allocator, class Plotter>
void TPlotRingBuffer(const char* name,
                     RingBuffer<ElementType, Allocator>& ringBuffer,
                     float xScale,
                     float x0,
                     Plotter plotter)
{
  ImPlot::BeginPlot(name);
  auto const numChannels = ringBuffer.getNumChannels();

  auto const offset = numChannels * ringBuffer.getReadPosition();
  auto const stride = ringBuffer.getNumChannels() * sizeof(ElementType);
  auto const readBlockSize = numChannels * ringBuffer.getReadBlockSize();
  auto const totalSize = static_cast<Index>(ringBuffer.getBuffer().size());
  for (Index channel = 0; channel < numChannels; ++channel) {
    auto const start = offset + channel;
    auto const end = start + readBlockSize;
    auto contiguousEnd = std::min(end, totalSize);
    auto contiguousSize = contiguousEnd - start;
    plotter(name, ringBuffer, contiguousSize / numChannels, xScale, x0, start, stride, channel);
    if (contiguousEnd != end) {
      auto const size = (end - totalSize) / numChannels;
      plotter(name, ringBuffer, size, xScale, x0 + contiguousSize / numChannels, 0, stride, channel);
    }
  }
  ImPlot::EndPlot();
}

template<class ElementType, class Allocator>
void PlotRingBuffer(const char* name,
                    RingBuffer<ElementType, Allocator>& ringBuffer,
                    float xScale = 100.f,
                    float x0 = 0.f)
{
  TPlotRingBuffer(name,
                  ringBuffer,
                  xScale,
                  x0,
                  [&](const char* name,
                      const RingBuffer<ElementType, Allocator>& buffer,
                      int count,
                      double xScale,
                      double x0,
                      int offset,
                      int stride,
                      Index channel) {
                    ImPlot::PushStyleColor(ImPlotCol_Line,
                                           channel == 0 ? ImVec4(0.f, 0.5f, 1.f, 1.f) : ImVec4(1.f, 0.5f, 0.f, 1.f));
                    ImPlot::PlotLine(name, ringBuffer.getBuffer().data() + offset, count, xScale, x0, 0, stride);
                    ImPlot::PopStyleColor(ImPlotCol_Line);
                  });
}

template<class ElementType, class Allocator>
void PlotWaveformRingBuffer(const char* name,
                            WaveformRingBuffer<ElementType, Allocator>& ringBuffer,
                            float xScale = 100.f,
                            float x0 = 0.f)
{
  TPlotRingBuffer(name,
                  ringBuffer,
                  xScale,
                  x0,
                  [&](const char* name,
                      const WaveformRingBuffer<ElementType, Allocator>& buffer,
                      int count,
                      double xScale,
                      double x0,
                      int offset,
                      int stride,
                      Index channel) {
                    auto const rawData = &ringBuffer.getBuffer()[0].negative;
                    ImPlot::PushStyleColor(ImPlotCol_Line,
                                           channel == 0 ? ImVec4(0.f, 0.5f, 1.f, 1.f) : ImVec4(1.f, 0.5f, 0.f, 1.f));
                    ImPlot::PlotLine(name, rawData + 2 * offset, 2 * count, xScale, x0, 0, 2 * stride);
                    ImPlot::PlotLine(name, rawData + 2 * offset + 1, 2 * count, xScale, x0, 0, 2 * stride);
                    ImPlot::PopStyleColor(ImPlotCol_Line);
                  });
}

} // namespace unplug