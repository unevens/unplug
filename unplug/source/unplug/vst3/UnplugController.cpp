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
#include "Parameters.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "unplug/GetParameterDescriptions.hpp"
#include "unplug/GetVersion.hpp"
#include "unplug/Presets.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/UserInterface.hpp"
#include "unplug/detail/GetSortedParameterDescriptions.hpp"
#include "unplug/detail/Vst3MessageIds.hpp"
#include "unplug/detail/Vst3NonlinearParameter.hpp"
#include <memory>

namespace Steinberg::Vst {

using namespace unplug;
using Presets = detail::Presets;

tresult PLUGIN_API UnplugController::initialize(FUnknown* context)
{
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
                                                  description.index,
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
                                              description.index,
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
          new StringListParameter(title.c_str(), description.index, pUnits, flags, kRootUnitId, pShortTitle);
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
        midiMapping.mapParameter(description.index, mapping.control);
      }
      else {
        midiMapping.mapParameter(description.index, description.defaultMidiMapping.control, mapping.channel);
      }
    }
  }

  auto const& presets = Presets::get();

  if (presets.empty()) {
    unitInfo.programListId = kNoProgramListId;
  }
  else {
    auto const presetParameterTag = std::numeric_limits<uint32>::max();
    parameters.addParameter(new RangeParameter(STR16("Preset"),
                                               presetParameterTag,
                                               STR16(""),
                                               0,
                                               presets.size() - 1,
                                               0,
                                               presets.size() - 1,
                                               ParameterInfo::kIsProgramChange));

    auto const listId = 0;
    auto list = new ProgramListWithPitchNames(STR16("Factory Presets"), listId, kRootUnitId);
    unitInfo.programListId = listId;

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
  // loads the dsp state
  if (!state)
    return kResultFalse;
  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < NumParameters::value; ++i) {
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
  if (!streamer.readInt32Array(lastViewSize.data(), lastViewSize.size())) {
    return kResultFalse;
  }
  auto version = Version{ 0 };
  if (!streamer.readInt32Array(version.data(), version.size())) {
    return kResultFalse;
  }
  return persistentData.load(loadInteger, loadIntegerArray, loadDoubleArray, loadBytes) ? kResultTrue : kResultFalse;
}

tresult PLUGIN_API UnplugController::getState(IBStream* state)
{
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
  if (!streamer.writeInt32Array(lastViewSize.data(), lastViewSize.size())) {
    return kResultFalse;
  }
  auto constexpr version = getVersion();
  if (!streamer.writeInt32Array(version.data(), version.size())) {
    return kResultFalse;
  }
  persistentData.save(saveInteger, saveIntegerArray, saveDoubleArray, saveBytes);
  return kResultTrue;
}

IPlugView* PLUGIN_API UnplugController::createView(FIDString name)
{
  if (FIDStringsEqual(name, ViewType::kEditor)) {
    auto ui = new View(*this);
    auto message = owned(allocateMessage());
    message->setMessageID(vst3::messaageIds::userInterfaceChangedId);
    message->getAttributes()->setInt(vst3::messaageIds::userInterfaceStateId, 1);
    sendMessage(message);
    return ui;
  }
  return nullptr;
}

tresult UnplugController::getMidiControllerAssignment(int32 busIndex,
                                                      int16 channel,
                                                      CtrlNumber midiControllerNumber,
                                                      ParamID& index)
{
  if (busIndex == 0) {
    auto const mappedParameter = midiMapping.getParameter(static_cast<int>(midiControllerNumber), channel);
    if (mappedParameter != MidiMapping::unmapped) {
      index = mappedParameter;
      return kResultTrue;
    }
  }
  return kResultFalse;
}

tresult UnplugController::setParamNormalized(ParamID index, ParamValue value)
{
  if (Parameter* parameter = getParameterObject(index)) {
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
    }
    auto message = owned(allocateMessage());
    message->setMessageID(vst3::messaageIds::programChangeId);
    message->getAttributes()->setInt(vst3::messaageIds::programChangeId, presetIndex);
    sendMessage(message);
  }
}

tresult PLUGIN_API UnplugController::notify(IMessage* message)
{
  using namespace vst3::messaageIds;
  if (!message)
    return kInvalidArgument;

  if (FIDStringsEqual(message->getMessageID(), meterSharingId)) {
    auto const getAddress = [&](auto attributeId) {
      const void* binary;
      uint32 size;
      bool gotProgramIndexOk = message->getAttributes()->getBinary(attributeId, binary, size) == kResultOk;
      assert(gotProgramIndexOk);
      assert(size == sizeof(binary));
      auto const address = *reinterpret_cast<const uintptr_t*>(binary);
      return address;
    };
    meters = *reinterpret_cast<std::shared_ptr<MeterStorage>*>(getAddress(meterStorageId));
    customData = *reinterpret_cast<std::shared_ptr<CustomSharedData>*>(getAddress(customStorageId));
    return kResultOk;
  }
  else
    return onNotify(message) ? kResultOk : kResultFalse;
}

void UnplugController::onViewClosed()
{
  auto message = owned(allocateMessage());
  message->setMessageID(vst3::messaageIds::userInterfaceChangedId);
  message->getAttributes()->setInt(vst3::messaageIds::userInterfaceStateId, 0);
  sendMessage(message);
}

bool UnplugController::onNotify(IMessage* message)
{
  return EditControllerEx1::notify(message) == kResultOk;
}

} // namespace Steinberg::Vst