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

  void setup() const
  {
    onSetup();
  }

  SetupPluginFromDsp(std::function<void()> onSetup, UpdateLatency onUpdateLatency)
    : onSetup{ std::move(onSetup) }
    , onUpdateLatency{ std::move(onUpdateLatency) }
  {}

private:
  UpdateLatency onUpdateLatency;
  std::function<void()> onSetup;
};

class SetupPluginFromDspUnit final
{
public:
  void setLatency(uint32_t dspUnitLatency) const
  {
    pluginInterface.setLatency(dspUnitIndex, dspUnitLatency);
  }

  void setup() const
  {
    pluginInterface.setup();
  }

  SetupPluginFromDspUnit(SetupPluginFromDsp pluginInterface, unplug::Index dspUnitIndex)
    : pluginInterface{ std::move(pluginInterface) }
    , dspUnitIndex{ dspUnitIndex }
  {}

private:
  SetupPluginFromDsp const pluginInterface;
  unplug::Index dspUnitIndex;
};

} // namespace unplug