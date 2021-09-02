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
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "unplug/MidiMapping.hpp"
#include "unplug/ParameterStorage.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/ViewPersistentData.hpp"
#include "unplug/detail/Vst3DBParameter.hpp"
#include <memory>

namespace Steinberg::Vst {

template<class View, class Parameters>
class UnplugController
  : public EditControllerEx1
  , public IMidiMapping
{
public:
  using FUnknown = FUnknown;
  using IEditController = IEditController;
  using MidiMapping = unplug::MidiMapping;

  UnplugController() = default;
  ~UnplugController() override = default;

  // IPluginBase
  tresult PLUGIN_API initialize(FUnknown* context) override;

  // EditController
  tresult PLUGIN_API setComponentState(IBStream* state) override;

  IPlugView* PLUGIN_API createView(FIDString name) override;

  tresult PLUGIN_API setState(IBStream* state) override;

  tresult PLUGIN_API getState(IBStream* state) override;

  tresult PLUGIN_API getMidiControllerAssignment(int32 busIndex,
                                                 int16 channel,
                                                 CtrlNumber midiControllerNumber,
                                                 ParamID& tag) override;

protected:
  unplug::MidiMapping midiMapping;

private:
  unplug::ViewPersistentData persistentData;
  std::array<int, 2> lastViewSize{ { -1, -1 } };

  DEFINE_INTERFACES
  DEF_INTERFACE(IMidiMapping);
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)
};

// implementation

template<class View, class Parameters>
tresult PLUGIN_API
UnplugController<View, Parameters>::initialize(FUnknown* context)
{
  using namespace unplug;
  using namespace Steinberg;
  using namespace Vst;

  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  View::initializePersistentData(persistentData);

  UnitInfo unitInfo;
  unitInfo.id = kRootUnitId;
  unitInfo.parentUnitId = kNoParentUnitId;
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
        if (description.controledInDecibels) {
          auto parameter = new DBParameter(title.c_str(),
                                           description.tag,
                                           description.min,
                                           description.max,
                                           description.defaultValue,
                                           description.linearZeroInDB,
                                           description.numSteps,
                                           flags,
                                           unitId,
                                           pShortTitle);
          parameters.addParameter(parameter);
        }
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

    if (description.defaultMidiMapping.isEnabled()) {
      auto const& mapping = description.defaultMidiMapping;
      if (description.defaultMidiMapping.listensToAllChannels()) {
        midiMapping.mapParameter(description.tag, mapping.control);
      }
      else {
        midiMapping.mapParameter(description.tag, description.defaultMidiMapping.control, mapping.channel);
      }
    }
  };

  Parameters::getParameterInitializer().initializeParameters(initializeParameter);

  return kResultOk;
}

template<class View, class Parameters>
tresult PLUGIN_API
UnplugController<View, Parameters>::setComponentState(IBStream* state)
{
  using namespace Steinberg;
  // loads the dsp state
  if (!state)
    return kResultFalse;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < Parameters::numParameters; ++i) {
    double value;
    if (!streamer.readDouble(value))
      return kResultFalse;
    auto parameter = parameters.getParameterByIndex(i);
    setParamNormalized(parameter->getInfo().id, parameter->toNormalized(value));
  }
  return kResultOk;
}

template<class View, class Parameters>
tresult PLUGIN_API
UnplugController<View, Parameters>::setState(IBStream* state)
{
  using namespace Steinberg;

  // used to load ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const loadInteger = [&](int64_t& x) { return streamer.readInt64((int64&)x); };
  auto const loadIntegerArray = [&](int64_t* x, int64_t size) { return streamer.readInt64Array((int64*)x, size); };
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
  std::array<int64, 2> loadedSize{};
  if (!loadInteger(loadedSize[0]))
    return kResultFalse;
  if (!loadInteger(loadedSize[1]))
    return kResultFalse;
  lastViewSize[0] = loadedSize[0];
  lastViewSize[1] = loadedSize[1];
  return persistentData.load(loadInteger, loadIntegerArray, loadDoubleArray, loadBytes) ? kResultTrue : kResultFalse;
}

template<class View, class Parameters>
tresult PLUGIN_API
UnplugController<View, Parameters>::getState(IBStream* state)
{
  using namespace Steinberg;
  // used to save ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const saveInteger = [&](int64_t const& x) { return streamer.writeInt64(x); };
  auto const saveIntegerArray = [&](int64_t const* x, int64_t size) {
    return streamer.writeInt64Array((int64*)x, static_cast<int32>(size));
  };
  auto const saveDoubleArray = [&](double const* x, int64_t size) {
    return streamer.writeDoubleArray(x, static_cast<int>(size));
  };
  auto const saveBytes = [&](void const* x, int64_t size) { return streamer.writeRaw(x, static_cast<int>(size)); };
  saveInteger(lastViewSize[0]);
  saveInteger(lastViewSize[1]);
  persistentData.save(saveInteger, saveIntegerArray, saveDoubleArray, saveBytes);
  return kResultTrue;
}

template<class View, class Parameters>
IPlugView* PLUGIN_API
UnplugController<View, Parameters>::createView(FIDString name)
{
  using namespace Steinberg;
  if (FIDStringsEqual(name, ViewType::kEditor)) {
    auto ui = new View(*this, persistentData, midiMapping, lastViewSize);
    return ui;
  }
  return nullptr;
}

template<class View, class Parameters>
tresult
UnplugController<View, Parameters>::getMidiControllerAssignment(int32 busIndex,
                                                                int16 channel,
                                                                CtrlNumber midiControllerNumber,
                                                                ParamID& tag)
{
  if (busIndex == 0) {
    auto const mappedParameter = midiMapping.getParameter(static_cast<int>(midiControllerNumber), channel);
    if (mappedParameter != MidiMapping::unmapped) {
      tag = mappedParameter;
      return kResultTrue;
    }
  }
  return kResultFalse;
}

} // namespace Steinberg::Vst

namespace unplug {
template<class View, class Parameters>
using UnplugController = Steinberg::Vst::UnplugController<View, Parameters>;
}
