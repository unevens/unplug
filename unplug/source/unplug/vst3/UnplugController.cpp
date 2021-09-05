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

#include "unplug/UnplugController.hpp"
#include "NumParameters.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "unplug/Parameters.hpp"
#include "unplug/Presets.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/UserInterface.hpp"
#include "unplug/detail/GetSortedParameterDescriptions.hpp"
#include "unplug/detail/Vst3MessageIds.hpp"
#include "unplug/detail/Vst3NonlinearParameter.hpp"
#include <memory>

namespace Steinberg::Vst {

enum
{
  kPresetParam = std::numeric_limits<uint32>::max()
};

using Presets = unplug::detail::Presets;

tresult PLUGIN_API UnplugController::initialize(FUnknown* context)
{
  using namespace unplug;
  using namespace Steinberg;
  using namespace Vst;

  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  UserInterface::initializePersistentData(persistentData);

  UnitInfo unitInfo;
  unitInfo.id = kRootUnitId;
  unitInfo.parentUnitId = kNoParentUnitId;
  UString setUnitName(unitInfo.name, 128);
  setUnitName.fromAscii("Root");

  auto const parameterDescriptions = detail::getSortedParameterDescriptions();

  for (auto const& description : parameterDescriptions) {
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
        if (description.isNonlinear()) {
          auto parameter = new NonlinearParameter(title.c_str(),
                                                  description.tag,
                                                  description.nonlinearToLinear,
                                                  description.linearToNonlinear,
                                                  description.min,
                                                  description.max,
                                                  description.defaultValue,
                                                  flags,
                                                  kRootUnitId,
                                                  pShortTitle);
          parameters.addParameter(parameter);
        }
        else {
          auto parameter = new RangeParameter(title.c_str(),
                                              description.tag,
                                              pUnits,
                                              description.min,
                                              description.max,
                                              description.defaultValue,
                                              description.numSteps,
                                              flags,
                                              kRootUnitId,
                                              pShortTitle);
          parameters.addParameter(parameter);
        }
      } break;
      case ParameterDescription::Type::list: {
        int32 const flags =
          ParameterInfo::kIsList | (description.canBeAutomated ? ParameterInfo::kCanAutomate : ParameterInfo::kNoFlags);
        auto parameter =
          new StringListParameter(title.c_str(), description.tag, pUnits, flags, kRootUnitId, pShortTitle);
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
  }

  auto const& presets = Presets::get();

  if (presets.empty()) {
    unitInfo.programListId = kNoProgramListId;
  }
  else {
    unitInfo.programListId = kPresetParam;
    auto const presetParameterTag = unplug::NumParameters::value;
    parameters.addParameter(new RangeParameter(STR16("Preset"),
                                               presetParameterTag,
                                               STR16(""),
                                               0,
                                               presets.size() - 1,
                                               0,
                                               presets.size() - 1,
                                               ParameterInfo::kIsProgramChange));

    auto list = new ProgramListWithPitchNames(STR16("Factory Presets"), 0, kRootUnitId);

    int programIndex = 0;
    for (auto& preset : presets) {
      String128 programName;
      UString(programName, str16BufferSize(String128)).assign(ToVstTChar{}(preset.name).c_str());
      list->addProgram(programName);
      for (auto [pitch, name] : preset.pitchNames) {
        String128 pitchName;
        UString(pitchName, str16BufferSize(String128)).assign(ToVstTChar{}(name).c_str());
        list->setPitchName(programIndex, static_cast<int16>(pitch), pitchName);
      }
      ++programIndex;
    }

    addProgramList(list);
  }

  return kResultOk;
}

tresult PLUGIN_API UnplugController::setComponentState(IBStream* state)
{
  using namespace Steinberg;
  // loads the dsp state
  if (!state)
    return kResultFalse;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < unplug::NumParameters::value; ++i) {
    double value;
    if (!streamer.readDouble(value))
      return kResultFalse;
    auto parameter = parameters.getParameterByIndex(i);
    bool const isProgramChange = (parameter->getInfo().flags & ParameterInfo::kIsProgramChange) != 0;
    assert(!isProgramChange);
    if (!isProgramChange)
      setParamNormalized(parameter->getInfo().id, parameter->toNormalized(value));
  }
  return kResultOk;
}

tresult PLUGIN_API UnplugController::setState(IBStream* state)
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

tresult PLUGIN_API UnplugController::getState(IBStream* state)
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

IPlugView* PLUGIN_API UnplugController::createView(FIDString name)
{
  using namespace Steinberg;
  if (FIDStringsEqual(name, ViewType::kEditor)) {
    auto ui = new View(*this, persistentData, midiMapping, lastViewSize);
    return ui;
  }
  return nullptr;
}

tresult UnplugController::getMidiControllerAssignment(int32 busIndex,
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

tresult UnplugController::setParamNormalized(ParamID tag, ParamValue value)
{
  if (Parameter* parameter = getParameterObject(tag)) {
    parameter->setNormalized(value);
    bool const isProgramChange = (parameter->getInfo().flags & ParameterInfo::kIsProgramChange) != 0;
    if (isProgramChange) {
      int const selectedPreset = static_cast<int>(std::round(value * static_cast<double>(Presets::get().size())));
      applyPreset(selectedPreset);
    }
    return kResultTrue;
  }
  return kResultFalse;
}

void UnplugController::applyPreset(int presetIndex)
{
  if (Presets::get().size() > presetIndex) {
    auto& preset = Presets::get()[presetIndex];
    for (auto [parameterTag, value] : preset.parameterValues) {
      auto const valueNormalized = parameters.getParameter(parameterTag)->toNormalized(value);
      setParamNormalized(parameterTag, valueNormalized);
      bool const setOk = setParamNormalized(parameterTag, valueNormalized) == kResultTrue;
      assert(setOk);
      assert(parameterStorage);
      if (setOk && parameterStorage) {
        parameterStorage->set(parameterTag, value);
      }
    }
  }
}

tresult UnplugController::notify(IMessage* message)
{
  if (!message)
    return kInvalidArgument;

  if (FIDStringsEqual(message->getMessageID(), unplug::vst3::initializationMessage)) {
    void const* binary = nullptr;
    uint32 size = 0;
    message->getAttributes()->getBinary(unplug::vst3::parameterStorageId, binary, size);
    assert(binary);
    if (binary) {
      auto address = *static_cast<uintptr_t const*>(binary);
      parameterStorage = reinterpret_cast<unplug::ParameterStorage<unplug::NumParameters::value>*>(address);
    }
    return kResultOk;
  }

  return EditController::notify(message);
}

} // namespace Steinberg::Vst

namespace unplug {

using UnplugController = Steinberg::Vst::UnplugController;
}
