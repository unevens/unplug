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
#include "unplug/Color.hpp"
#include "unplug/RingBuffer.hpp"

namespace unplug {

/**
 * A struct that holds the data necessary to show an entry in the legend of a plot
 * */
struct PlotChannelLegend
{
  std::string label;
  ImVec4 color{ 0.f, 1.f, 1.f, 1.f };
};

/**
 * Creates a standard legend for a plot with a line for the left channel and a line for the right channel
 * */
PlotChannelLegend getStereoPlotChannelLegend(Index channel, Index numChannels);

/**
 * Creates a standard legend for a plot with a line for the mid channel and a line for the side channel
 * */
PlotChannelLegend getMidSidePlotChannelLegend(Index channel, Index numChannels);

/**
 * Creates a standard legend for a generic plot with an arbitrary amount of lines, using a different hue for each
 * channel
 * */
std::function<PlotChannelLegend(Index channel, Index numChannels)> makeGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f);

/**
 * Creates a standard legend for a plot with a line for the left channel and a line for the right channel if the number
 * of channel is 2, otherwise it creates a standard legend for a generic plot with an arbitrary amount of lines, using a
 * different hue for each channel
 * */
std::function<PlotChannelLegend(Index channel, Index numChannels)> makeStereoOrGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f);

/**
 * Creates a standard legend for a plot with a line for the mid channel and a line for the side channel if the number
 * of channel is 2, otherwise it creates a standard legend for a generic plot with an arbitrary amount of lines, using a
 * different hue for each channel
 * */
std::function<PlotChannelLegend(Index channel, Index numChannels)> makeMidSideOrGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f);

/**
 * Plots a ring buffer using a custom plotting function
 * */
template<class ElementType, class Allocator, class Plotter>
bool TPlotRingBuffer(const char* name,
                     RingBuffer<ElementType, Allocator>& ringBuffer,
                     std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend,
                     Plotter plotter)
{
  if (ImPlot::BeginPlot(name)) {
    auto const numChannels = ringBuffer.getNumChannels();
    auto const offset = numChannels * ringBuffer.getReadPosition();
    auto const stride = ringBuffer.getNumChannels() * sizeof(ElementType);
    auto const readBlockSize = numChannels * ringBuffer.getReadBlockSize();
    auto const totalSize = static_cast<Index>(ringBuffer.getBuffer().size());
    auto const xScale = ringBuffer.getSecondsPerPoint();
    for (Index channel = 0; channel < numChannels; ++channel) {
      auto const start = offset + channel;
      auto const end = start + readBlockSize;
      auto contiguousEnd = std::min(end, totalSize);
      auto contiguousSize = contiguousEnd - start;
      auto const channelLegend = getChannelLegend(channel, numChannels);
      auto const pointsCount = contiguousSize / numChannels;
      if (pointsCount > 0)
        plotter(channelLegend, ringBuffer, pointsCount, xScale, 0, start, stride, channel);
      if (contiguousEnd != end) {
        auto const size = (end - totalSize) / numChannels;
        if (size > 0)
          plotter(channelLegend, ringBuffer, size, xScale, pointsCount * xScale, channel, stride, channel);
      }
    }
    ImPlot::EndPlot();
    return true;
  }
  return false;
}

/**
 * Plots a simple ring buffer, suitable for ring buffers holding continuous numeric data
 * */
template<class ElementType, class Allocator>
bool PlotRingBuffer(const char* name,
                    RingBuffer<ElementType, Allocator>& ringBuffer,
                    std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                      makeStereoOrGenericPlotChannelLegend())
{
  return TPlotRingBuffer(
    name,
    ringBuffer,
    getChannelLegend,
    [&](PlotChannelLegend const& channelLegend,
        const RingBuffer<ElementType, Allocator>& buffer,
        int count,
        double xScale,
        double x0,
        int offset,
        int stride,
        Index channel) {
      ImPlot::PushStyleColor(ImPlotCol_Line, channelLegend.color);
      ImPlot::PlotLine(
        channelLegend.label.c_str(), ringBuffer.getBuffer().data() + offset, count, xScale, x0, 0, stride);
      ImPlot::PopStyleColor(ImPlotCol_Line);
    });
}

/**
 * Plots a simple ring buffer, suitable for ring buffers holding continuous numeric data.
 * */
template<class ElementType, class Allocator>
bool PlotRingBuffer(const char* name,
                    lockfree::RealtimeObject<RingBuffer<ElementType, Allocator>>& rtRingBuffer,
                    std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                      makeStereoOrGenericPlotChannelLegend())
{
  auto ringBuffer = rtRingBuffer.getFromNonRealtimeThread();
  if (ringBuffer) {
    return PlotRingBuffer(name, *ringBuffer, getChannelLegend);
  }
  return false;
}

/**
 * Plots a waveform ring buffer.
 * */
template<class ElementType, class Allocator>
bool PlotWaveformRingBuffer(const char* name,
                            WaveformRingBuffer<ElementType, Allocator>& ringBuffer,
                            float alpha = 0.5f,
                            std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                              makeStereoOrGenericPlotChannelLegend())
{
  return TPlotRingBuffer(
    name,
    ringBuffer,
    getChannelLegend,
    [&](PlotChannelLegend const& channelLegend,
        const WaveformRingBuffer<ElementType, Allocator>& buffer,
        int count,
        double xScale,
        double x0,
        int offset,
        int stride,
        Index channel) {
      auto const rawData = &ringBuffer.getBuffer()[0].negative;
      ImPlot::PushStyleColor(ImPlotCol_Line, channelLegend.color);
      if (alpha > 0.f) {
        assert(alpha <= 1.f);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, alpha);
        ImPlot::PlotShaded(channelLegend.label.c_str(), rawData + 2 * offset, count, 0.f, xScale, x0, 0, stride);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, alpha);
        ImPlot::PlotShaded(channelLegend.label.c_str(), rawData + 2 * offset + 1, count, 0.f, xScale, x0, 0, stride);
      }
      ImPlot::PlotLine(channelLegend.label.c_str(), rawData + 2 * offset, count, xScale, x0, 0, stride);
      ImPlot::PlotLine(channelLegend.label.c_str(), rawData + 2 * offset + 1, count, xScale, x0, 0, stride);
      ImPlot::PopStyleColor(ImPlotCol_Line);
    });
}

/**
 * Plots a waveform ring buffer.
 * */
template<class ElementType, class Allocator>
bool PlotWaveformRingBuffer(const char* name,
                            lockfree::RealtimeObject<WaveformRingBuffer<ElementType, Allocator>>& rtRingBuffer,
                            float alpha = 0.5f,
                            std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                              makeStereoOrGenericPlotChannelLegend())
{
  auto ringBuffer = rtRingBuffer.getFromNonRealtimeThread();
  if (ringBuffer) {
    return PlotWaveformRingBuffer(name, *ringBuffer, alpha, getChannelLegend);
  }
  return false;
}

} // namespace unplug