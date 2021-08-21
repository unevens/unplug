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

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "unplug/ParameterStorage.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/ViewPersistentData.hpp"
#include <memory>

namespace unplug {

template<class ViewClass, class PluginParameters>
class UnPlugController : public Steinberg::Vst::EditControllerEx1
{
public:
  UnPlugController() = default;
  ~UnPlugController() override = default;

  // IPluginBase
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

  // EditController
  Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;

  Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;

  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;

  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

private:
  unplug::ViewPersistentData persistentData;
};

// implementation

template<class ViewClass, class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugController<ViewClass, PluginParameters>::initialize(FUnknown* context)
{
  using namespace unplug;
  using namespace Steinberg;
  using namespace Steinberg::Vst;

  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  ViewClass::initializePersistentData(persistentData);

  UnitInfo unitInfo;
  unitInfo.id = kRootUnitId;
  unitInfo.parentUnitId = Steinberg::Vst::kNoParentUnitId;
  UString setUnitName(unitInfo.name, 128);
  setUnitName.fromAscii("Root");

  auto const initializeParameter = [this, unitId = kRootUnitId](unplug::ParameterDescription const& description) {
    TString title = ToVstTChar{}(description.name);
    TString shortTitle = ToVstTChar{}(description.shortName);
    TString units = ToVstTChar{}(description.measureUnit);
    auto pUnits = units.empty() ? nullptr : units.c_str();
    auto pShortTitle = shortTitle.empty() ? nullptr : shortTitle.c_str();

    switch (description.type) {
      case ParameterDescription::Type::numeric: {
        int32 const flags = [&] {
          int32 flags = ParameterInfo::kNoFlags;
          if (description.canBeAutomated)
            flags = ParameterInfo::kCanAutomate;
          if (description.isBypass)
            flags |= ParameterInfo::kIsBypass;
          return flags;
        }();
        auto parameter = new RangeParameter(title.c_str(),
                                            description.tag,
                                            pUnits,
                                            description.min,
                                            description.max,
                                            description.defaultValue,
                                            description.numSteps,
                                            flags,
                                            unitId,
                                            pShortTitle);
        parameters.addParameter(parameter);
      } break;
      case ParameterDescription::Type::list: {
        int32 const flags =
          ParameterInfo::kIsList | (description.canBeAutomated ? ParameterInfo::kCanAutomate : ParameterInfo::kNoFlags);
        auto parameter = new StringListParameter(title.c_str(), description.tag, pUnits, flags, unitId, pShortTitle);
        for (auto& entry : description.labels) {
          auto label = ToVstTChar{}(entry);
          parameter->appendString(label.c_str());
        }
        parameters.addParameter(parameter);
      } break;
    }
  };

  PluginParameters::getParameterInitializer().initializeParameters(initializeParameter);

  return kResultOk;
}

template<class ViewClass, class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugController<ViewClass, PluginParameters>::setComponentState(Steinberg::IBStream* state)
{
  using namespace Steinberg;
  // loads the dsp state
  if (!state)
    return kResultFalse;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < PluginParameters::numParameters; ++i) {
    double value;
    if (!streamer.readDouble(value))
      return kResultFalse;
    auto parameter = parameters.getParameterByIndex(i);
    setParamNormalized(parameter->getInfo().id, parameter->toNormalized(value));
  }
  return kResultOk;
}

template<class ViewClass, class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugController<ViewClass, PluginParameters>::setState(Steinberg::IBStream* state)
{
  using namespace Steinberg;

  // used to load ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const loadInteger = [&](int64_t& x) { return streamer.readInt64((Steinberg::int64&)x); };
  auto const loadIntegerArray = [&](int64_t* x, int64_t size) {
    return streamer.readInt64Array((Steinberg::int64*)x, size);
  };
  auto const loadDoubleArray = [&](double* x, int64_t size) {
    return streamer.readDoubleArray(x, static_cast<int>(size));
  };
  auto const loadBytes = [&](void* x, int64_t size) {
    if (size > 0) {
      auto numBytesRead = streamer.readRaw(x, static_cast<int>(size));
      return numBytesRead == size;
    }
    else {
      return true;
    }
  };
  persistentData.load(loadInteger, loadIntegerArray, loadDoubleArray, loadBytes);
  return kResultTrue;
}

template<class ViewClass, class PluginParameters>
Steinberg::tresult PLUGIN_API
UnPlugController<ViewClass, PluginParameters>::getState(Steinberg::IBStream* state)
{
  using namespace Steinberg;
  // used to save ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const saveInteger = [&](int64_t const& x) { return streamer.writeInt64(x); };
  auto const saveIntegerArray = [&](int64_t const* x, int64_t size) {
    return streamer.writeInt64Array((Steinberg::int64*)x, static_cast<Steinberg::int32>(size));
  };
  auto const saveDoubleArray = [&](double const* x, int64_t size) {
    return streamer.writeDoubleArray(x, static_cast<int>(size));
  };
  auto const saveBytes = [&](void const* x, int64_t size) { return streamer.writeRaw(x, static_cast<int>(size)); };
  persistentData.save(saveInteger, saveIntegerArray, saveDoubleArray, saveBytes);
  return kResultTrue;
}

template<class ViewClass, class PluginParameters>
Steinberg::IPlugView* PLUGIN_API
UnPlugController<ViewClass, PluginParameters>::createView(Steinberg::FIDString name)
{
  using namespace Steinberg;
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    auto ui = new ViewClass(*this, persistentData);
    return ui;
  }
  return nullptr;
}

} // namespace unplug
