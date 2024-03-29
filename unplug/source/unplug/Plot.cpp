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

#include "unplug/Plot.hpp"

namespace unplug {

PlotChannelLegend getStereoPlotChannelLegend(Index channel, Index numChannels)
{
  assert(channel < 2 && numChannels == 2);
  if (channel == 0)
    return { "Left", { 0.f, 0.5f, 1.f, 1.f } };
  if (channel == 1)
    return { "Right", { 1.f, 0.33f, 0.f, 1.f } };
  return { "invalid channel", { 1.f, 0.f, 1.f, 1.f } };
}

PlotChannelLegend getMidSidePlotChannelLegend(Index channel, Index numChannels)
{
  assert(channel < 2 && numChannels == 2);
  if (channel == 0)
    return { "Mid", { 0.f, 0.5f, 1.f, 1.f } };
  if (channel == 1)
    return { "Side", { 1.f, 0.33f, 0.f, 1.f } };
  return { "invalid channel", { 1.f, 0.f, 1.f, 1.f } };
}

std::function<PlotChannelLegend(Index channel, Index numChannels)> makeGenericPlotChannelLegend(float colorSaturation,
                                                                                                float colorIntensity,
                                                                                                float hueRotation,
                                                                                                float colorAlpha)
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

std::function<PlotChannelLegend(Index channel, Index numChannels)>
makeStereoOrGenericPlotChannelLegend(float colorSaturation, float colorIntensity, float hueRotation, float colorAlpha)
{
  auto genericLegend = makeGenericPlotChannelLegend(colorSaturation, colorIntensity, hueRotation, colorAlpha);
  return [genericLegend = std::move(genericLegend)](Index channel, Index numChannels) -> PlotChannelLegend {
    if (numChannels == 2) {
      return getStereoPlotChannelLegend(channel, numChannels);
    }
    return genericLegend(channel, numChannels);
  };
}

std::function<PlotChannelLegend(Index channel, Index numChannels)>
makeMidSideOrGenericPlotChannelLegend(float colorSaturation, float colorIntensity, float hueRotation, float colorAlpha)
{
  auto genericLegend = makeGenericPlotChannelLegend(colorSaturation, colorIntensity, hueRotation, colorAlpha);
  return [genericLegend = std::move(genericLegend)](Index channel, Index numChannels) -> PlotChannelLegend {
    if (numChannels == 2) {
      return getMidSidePlotChannelLegend(channel, numChannels);
    }
    return genericLegend(channel, numChannels);
  };
}

} // namespace unplug