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

struct PlotChannelLegend
{
  std::string name;
  ImVec4 color{ 0.f, 1.f, 1.f, 1.f };
};

inline PlotChannelLegend getStereoPlotChannelLegend(Index channel, Index numChannels)
{
  assert(channel > -1 && channel < 2 && numChannels == 2);
  if (channel == 0)
    return { "Left", { 0.f, 0.5f, 1.f, 1.f } };
  if (channel == 1)
    return { "Right", { 1.f, 0.5f, 0.f, 1.f } };
  return { "invalid channel", { 1.f, 0.f, 1.f, 1.f } };
}

inline PlotChannelLegend getMidSidePlotChannelLegend(Index channel, Index numChannels)
{
  assert(channel > -1 && channel < 2 && numChannels == 2);
  if (channel == 0)
    return { "Mid", { 0.f, 0.5f, 1.f, 1.f } };
  if (channel == 1)
    return { "Side", { 1.f, 0.5f, 0.f, 1.f } };
  return { "invalid channel", { 1.f, 0.f, 1.f, 1.f } };
}

inline std::function<PlotChannelLegend(Index channel, Index numChannels)> makeGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f)
{
  return [=](Index channel, Index numChannels) -> PlotChannelLegend {
    auto label = std::string("Channel ") + std::to_string(channel + 1);
    auto color = hsvToRgb({ hueRotation + static_cast<float>(channel) / static_cast<float>(numChannels),
                            colorSaturation,
                            colorIntensity,
                            colorAlpha });
    return { std::move(label), color };
  };
}

inline std::function<PlotChannelLegend(Index channel, Index numChannels)> makeStereoOrGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f)
{
  auto genericLegend = makeGenericPlotChannelLegend(colorSaturation, colorIntensity, hueRotation, colorAlpha);
  return [genericLegend = std::move(genericLegend)](Index channel, Index numChannels) -> PlotChannelLegend {
    if (numChannels == 2) {
      return getStereoPlotChannelLegend(channel, numChannels);
    }
    return genericLegend(channel, numChannels);
  };
}

inline std::function<PlotChannelLegend(Index channel, Index numChannels)> makeMidSideOrGenericPlotChannelLegend(
  float colorSaturation = 0.75,
  float colorIntensity = 1.f,
  float hueRotation = 3.5f / 6.f,
  float colorAlpha = 1.f)
{
  auto genericLegend = makeGenericPlotChannelLegend(colorSaturation, colorIntensity, hueRotation, colorAlpha);
  return [genericLegend = std::move(genericLegend)](Index channel, Index numChannels) -> PlotChannelLegend {
    if (numChannels == 2) {
      return getMidSidePlotChannelLegend(channel, numChannels);
    }
    return genericLegend(channel, numChannels);
  };
}

template<class ElementType, class Allocator, class Plotter>
bool TPlotRingBuffer(const char* name,
                     RingBuffer<ElementType, Allocator>& ringBuffer,
                     std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend,
                     float x0,
                     Plotter plotter)
{
  if (ImPlot::BeginPlot(name)) {
    auto const numChannels = ringBuffer.getNumChannels();
    auto const offset = numChannels * ringBuffer.getReadPosition();
    auto const stride = ringBuffer.getNumChannels() * sizeof(ElementType);
    auto const readBlockSize = numChannels * ringBuffer.getReadBlockSize();
    auto const totalSize = static_cast<Index>(ringBuffer.getBuffer().size());
    auto const xScale = 1.f / ringBuffer.getPointsPerSecond();
    for (Index channel = 0; channel < numChannels; ++channel) {
      auto const start = offset + channel;
      auto const end = start + readBlockSize;
      auto contiguousEnd = std::min(end, totalSize);
      auto contiguousSize = contiguousEnd - start;
      auto const channelLegend = getChannelLegend(channel, numChannels);
      auto const elementCount = contiguousSize / numChannels;
      plotter(channelLegend, ringBuffer, elementCount, xScale, x0, start, stride, channel);
      if (contiguousEnd != end) {
        auto const size = (end - totalSize) / numChannels;
        plotter(channelLegend, ringBuffer, size, xScale, x0, 0, stride, channel);
      }
    }
    ImPlot::EndPlot();
    return true;
  }
  return false;
}

template<class ElementType, class Allocator>
bool PlotRingBuffer(const char* name,
                    RingBuffer<ElementType, Allocator>& ringBuffer,
                    std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                      makeStereoOrGenericPlotChannelLegend(),
                    float x0 = 0.f)
{
  return TPlotRingBuffer(
    name,
    ringBuffer,
    getChannelLegend,
    x0,
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
        channelLegend.name.c_str(), ringBuffer.getBuffer().data() + offset, count, xScale, x0, 0, stride);
      ImPlot::PopStyleColor(ImPlotCol_Line);
    });
}

template<class ElementType, class Allocator>
bool PlotWaveformRingBuffer(const char* name,
                            WaveformRingBuffer<ElementType, Allocator>& ringBuffer,
                            std::function<PlotChannelLegend(Index channel, Index numChannels)> const& getChannelLegend =
                              makeStereoOrGenericPlotChannelLegend(),
                            float x0 = 0.f)
{
  return TPlotRingBuffer(
    name,
    ringBuffer,
    getChannelLegend,
    x0,
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
      ImPlot::PlotLine(channelLegend.name.c_str(), rawData + 2 * offset, count, xScale, x0, 0, stride);
      ImPlot::PlotLine(channelLegend.name.c_str(), rawData + 2 * offset + 1, count, xScale, x0, 0, stride);
      ImPlot::PopStyleColor(ImPlotCol_Line);
    });
}

} // namespace unplug