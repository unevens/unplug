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
#include "unplug/ContextInfo.hpp"
#include "unplug/SetupPluginFromDsp.hpp"
#include <functional>

namespace unplug {

template<class TObject, class TSettings>
class DspUnit final
{
public:
  using Object = TObject;
  using Settings = TSettings;
  using ApplyContext = typename std::function<void(ContextInfo const&, Settings&)>;
  using GetLatency = std::function<uint32_t(Object&)>;

  Object& get()
  {
    return object;
  }

  Settings const& getSettingsForEditing()
  {
    return settings;
  }

  Settings const& getSettingsInUse()
  {
    return settingsInUse;
  }

  void changeSettings(std::function<void(Settings&)> const& change)
  {
    change(settingsInUse);
    checkSettings();
  }

  void setSettings(Settings newSettings)
  {
    settings = std::move(newSettings);
    checkSettings();
  }

  void setContext(ContextInfo const& context)
  {
    applyContext(context, settings);
    if (settings != settingsInUse) {
      settingsInUse = settings;
      object = Object{ settings };
    }
    auto const latency = getLatency(object);
    setupPlugin.setLatency(latency);
  }

  DspUnit(
    SetupPluginFromDspUnit setupPlugin,
    ApplyContext applyContext,
    Settings settings_ = {},
    GetLatency getLatency = [](Object&) { return 0; })
    : settings{ std::move(settings_) }
    , settingsInUse{ settings }
    , object{ settings }
    , setupPlugin{ std::move(setupPlugin) }
    , applyContext{ std::move(applyContext) }
    , getLatency{ std::move(getLatency) }
  {}

private:
  void checkSettings()
  {
    if (settings != settingsInUse) {
      setupPlugin.restart();
    }
  }

  Settings settings;
  Settings settingsInUse;
  Object object;
  SetupPluginFromDspUnit setupPlugin;
  ApplyContext applyContext;
  GetLatency getLatency;
};

} // namespace unplug