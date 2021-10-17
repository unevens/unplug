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

#include "unplug/Index.hpp"
#include <cstdint>
#include <functional>
#include <utility>

namespace unplug {

class SetupPluginFromDsp final
{
public:
  using UpdateLatency = std::function<void(unplug::Index dspUnitIndex, uint32_t dspUnitLatency)>;

  void setLatency(unplug::Index dspUnitIndex, uint32_t dspUnitLatency) const
  {
    onUpdateLatency(dspUnitIndex, dspUnitLatency);
  }

  void restart() const
  {
    onRestart();
  }

  SetupPluginFromDsp(std::function<void()> onRestart, UpdateLatency onUpdateLatency)
    : onRestart{ std::move(onRestart) }
    , onUpdateLatency{ std::move(onUpdateLatency) }
  {}

private:
  UpdateLatency onUpdateLatency;
  std::function<void()> onRestart;
};

class SetupPluginFromDspUnit final
{
public:
  void setLatency(uint32_t dspUnitLatency) const
  {
    if (dspUnitIndex != noLatencyUnit) {
      pluginInterface.setLatency(dspUnitIndex, dspUnitLatency);
    }
  }

  void restart() const
  {
    pluginInterface.restart();
  }

  SetupPluginFromDspUnit(SetupPluginFromDsp pluginInterface, unplug::Index dspUnitIndex)
    : pluginInterface{ std::move(pluginInterface) }
    , dspUnitIndex{ dspUnitIndex }
  {}

  inline constexpr static Index noLatencyUnit = std::numeric_limits<Index>::max();

private:
  SetupPluginFromDsp const pluginInterface;
  int dspUnitIndex;
};

} // namespace unplug