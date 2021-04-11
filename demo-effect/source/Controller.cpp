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

#include "Controller.hpp"
#include "Id.hpp"
#include "Parameters.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "unplug/StringConversion.hpp"
#include "unplug/Vst3DemoView.hpp"

using namespace Steinberg;

namespace unplug {

using ViewClass = vst3::DemoView;

tresult PLUGIN_API
UnPlugDemoEffectController::initialize(FUnknown* context)
{
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

  // todo maybe: add support for preset lists

  auto const initializeParameter = [this, unitId = kRootUnitId](unplug::ParameterDescription const& description) {
    TString title = ToVstTChar{}(description.name);
    TString shortTitle = ToVstTChar{}(description.shortName);
    TString units = ToVstTChar{}(description.measureUnit);
    auto pUnits = units.empty() ? nullptr : units.c_str();
    auto pShortTitle = shortTitle.empty() ? nullptr : shortTitle.c_str();

    switch (description.type) {
      case ParameterDescription::Type::numeric: {
        int32 flags = description.canBeAutomated ? ParameterInfo::kCanAutomate : ParameterInfo::kNoFlags;
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
        int32 flags =
          ParameterInfo::kIsList | (description.canBeAutomated ? ParameterInfo::kCanAutomate : ParameterInfo::kNoFlags);
        auto parameter =
          new StringListParameter(title.c_str(), description.tag, pUnits, flags, unitId, pShortTitle);
        for (auto& entry : description.labels) {
          auto label = ToVstTChar{}(entry);
          parameter->appendString(label.c_str());
        }
        parameters.addParameter(parameter);
      } break;
    }
  };

  getParameterInitializer().initializeParameters(initializeParameter);

  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectController::terminate()
{
  return EditControllerEx1::terminate();
}

tresult PLUGIN_API
UnPlugDemoEffectController::setComponentState(IBStream* state)
{
  // loads the dsp state
  if (!state)
    return kResultFalse;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < ParamTag::numParams; ++i) {
    double value;
    if (!streamer.readDouble(value))
      return kResultFalse;
    auto parameter = parameters.getParameterByIndex(i);
    setParamNormalized(parameter->getInfo().id, parameter->toNormalized(value));
  }
  return kResultOk;
}

tresult PLUGIN_API
UnPlugDemoEffectController::setState(IBStream* state)
{
  // used to load ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const loadInteger = [&](int64_t& x) { return streamer.readInt64(x); };
  auto const loadIntegerArray = [&](int64_t* x, int64_t size) {
    return streamer.readInt64Array(x, static_cast<int>(size));
  };
  auto const loadDoubleArray = [&](double* x, int64_t size) {
    return streamer.readDoubleArray(x, static_cast<int>(size));
  };
  auto const loadBytes = [&](void* x, int64_t size) { return streamer.readRaw(x, static_cast<int>(size)); };
  persistentData.load(loadInteger, loadIntegerArray, loadDoubleArray, loadBytes);
  return kResultTrue;
}

tresult PLUGIN_API
UnPlugDemoEffectController::getState(IBStream* state)
{
  // used to save ui-only data
  IBStreamer streamer(state, kLittleEndian);
  auto const saveInteger = [&](int64_t const& x) { return streamer.writeInt64(x); };
  auto const saveIntegerArray = [&](int64_t const* x, int64_t size) {
    return streamer.writeInt64Array(x, static_cast<int>(size));
  };
  auto const saveDoubleArray = [&](double const* x, int64_t size) {
    return streamer.writeDoubleArray(x, static_cast<int>(size));
  };
  auto const saveBytes = [&](void const* x, int64_t size) { return streamer.writeRaw(x, static_cast<int>(size)); };
  persistentData.save(saveInteger, saveIntegerArray, saveDoubleArray, saveBytes);
  return kResultTrue;
}

IPlugView* PLUGIN_API
UnPlugDemoEffectController::createView(FIDString name)
{
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    auto ui = new ViewClass(*this, persistentData, "UnPlugDemoEffect");
    return ui;
  }
  return nullptr;
}

tresult PLUGIN_API
UnPlugDemoEffectController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
{
  // called by host to update your parameters
  tresult result = EditControllerEx1::setParamNormalized(tag, value);
  return result;
}

tresult PLUGIN_API
UnPlugDemoEffectController::getParamStringByValue(Vst::ParamID tag,
                                                  Vst::ParamValue valueNormalized,
                                                  Vst::String128 string)
{
  // called by host to get a string for given normalized value of a specific
  // parameter (without having to set the value!)
  return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

tresult PLUGIN_API
UnPlugDemoEffectController::getParamValueByString(Vst::ParamID tag,
                                                  Vst::TChar* string,
                                                  Vst::ParamValue& valueNormalized)
{
  // called by host to get a normalized value from a string representation of a
  // specific parameter (without having to set the value!)
  return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

} // namespace unplug
