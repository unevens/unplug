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

#include "unplug/UnplugProcessor.hpp"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "unplug/GetVersion.hpp"
#include "unplug/Presets.hpp"
#include "unplug/UserInterface.hpp"
#include "unplug/detail/GetSortedParameterDescriptions.hpp"
#include "unplug/detail/Vst3MessageIds.hpp"

using namespace unplug;
using Presets = detail::Presets;

namespace Steinberg::Vst {

void UnplugProcessor::onInitialization()
{
  //--- create Audio IO ------
  addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);
}

tresult PLUGIN_API UnplugProcessor::initialize(FUnknown* context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  auto const parameterDescriptions = detail::getSortedParameterDescriptions();
  pluginState.parameters.initialize(parameterDescriptions);

  onInitialization();

  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::terminate()
{
  onTermination();
  return AudioEffect::terminate();
}

void UnplugProcessor::updateParametersToLastPoint(ProcessData& data)
{
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
        int32 numPoints = paramQueue->getPointCount();
        if (numPoints > 0) {
          ParamValue value;
          int32 sampleOffset;
          paramQueue->getPoint(numPoints - 1, sampleOffset, value);
          auto const parameterTag = paramQueue->getParameterId();
          pluginState.parameters.setNormalized(parameterTag, value);
        }
      }
    }
  }
}

tresult PLUGIN_API UnplugProcessor::setState(IBStream* state)
{
  IBStreamer streamer(state, kLittleEndian);
  Version version;
  if (!streamer.readInt32Array(version.data(), version.size())) {
    return kResultFalse;
  }
  for (int i = 0; i < NumParameters::value; ++i) {
    double value;
    if (!streamer.readDouble(value)) {
      return kResultFalse;
    }
    pluginState.parameters.set(i, value);
  }
  return onSetState(streamer) ? kResultOk : kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::getState(IBStream* state)
{
  IBStreamer streamer(state, kLittleEndian);
  auto constexpr version = getVersion();
  if (!streamer.writeInt32Array(version.data(), version.size())) {
    return kResultFalse;
  }
  for (int i = 0; i < NumParameters::value; ++i) {
    double const value = pluginState.parameters.get(i);
    if (!streamer.writeDouble(value)) {
      return kResultFalse;
    }
  }
  return onGetState(streamer) ? kResultOk : kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return supportsDoublePrecision();

  return kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::notify(IMessage* message)
{
  using namespace vst3::messaageIds;
  if (!message)
    return kInvalidArgument;

  if (FIDStringsEqual(message->getMessageID(), programChangeId)) {
    int64 programIndex = 0;
    bool gotProgramIndexOk = message->getAttributes()->getInt(programIndexId, programIndex) == kResultOk;
    assert(gotProgramIndexOk);
    if (gotProgramIndexOk) {
      auto& preset = Presets::get()[programIndex];
      int i = 0;
      for (auto [parameterTag, value] : preset.parameterValues) {
        pluginState.parameters.set(i++, value);
      }
      return kResultOk;
    }
    else {
      return kResultFalse;
    }
  }
  else if (FIDStringsEqual(message->getMessageID(), userInterfaceChangedId)) {
    int64 userInterfaceState = 0;
    bool gotStateOk = message->getAttributes()->getInt(userInterfaceStateId, userInterfaceState) == kResultOk;
    assert(gotStateOk);
    if (gotStateOk) {
      pluginState.isUserInterfaceOpen.store(userInterfaceState != 0, std::memory_order_release);
      return kResultOk;
    }
    else {
      return kResultFalse;
    }
  }
  else
    return onNotify(message) ? kResultOk : kResultFalse;
}

tresult UnplugProcessor::setActive(TBool state)
{
  if (state) {
    if (!pluginState.meters) {
      pluginState.meters = std::make_shared<MeterStorage>();
    }
  }
  else {
    pluginState.meters = nullptr;
  }

  if (!pluginState.circularBuffers) {
    pluginState.circularBuffers = std::make_shared<CircularBufferStorage>();
  }
  auto const numIO = getNumIO();
  pluginState.circularBuffers->get().resize(
    processSetup.sampleRate, UserInterface::getRefreshRate(), processSetup.maxSamplesPerBlock, numIO);

  auto message = owned(allocateMessage());
  message->setMessageID(vst3::messaageIds::meterSharingId);
  auto const meterStorageAddress = reinterpret_cast<uintptr_t>(&pluginState.meters);
  message->getAttributes()->setBinary(
    vst3::messaageIds::meterStorageId, &meterStorageAddress, sizeof(meterStorageAddress));
  auto const circularBufferStorageAddress = reinterpret_cast<uintptr_t>(&pluginState.circularBuffers);
  message->getAttributes()->setBinary(
    vst3::messaageIds::circularBuffersId, &circularBufferStorageAddress, sizeof(circularBufferStorageAddress));
  sendMessage(message);

  onSetActive(state);
  return AudioEffect::setActive(state);
}

tresult UnplugProcessor::setBusArrangements(SpeakerArrangement* inputs,
                                            int32 numIns,
                                            SpeakerArrangement* outputs,
                                            int32 numOuts)
{
  ioCache.resize(numIns, numOuts);
  return onSetBusArrangements(inputs, numIns, outputs, numOuts) ? kResultOk : kResultFalse;
}

tresult UnplugProcessor::acceptSimpleBusArrangement(
  SpeakerArrangement* inputs,
  int32 numIns,
  SpeakerArrangement* outputs,
  int32 numOuts,
  bool acceptSidechain,
  const std::function<bool(int numInputs, int numOutputs, int numSidechain)>& acceptNumChannels)
{
  if (numOuts != 1) {
    return kResultFalse;
  }
  if (acceptSidechain) {
    if (numIns != 1 && numIns != 2) {
      return kResultFalse;
    }
  }
  else {
    if (numIns != 1) {
      return kResultFalse;
    }
  }
  bool const hasSidechain = numIns == 2;
  auto const numInputChannels = SpeakerArr::getChannelCount(inputs[0]);
  auto const numOutputChannels = SpeakerArr::getChannelCount(outputs[0]);
  auto const numSidechainChannels = hasSidechain ? SpeakerArr::getChannelCount(inputs[1]) : 0;
  if (!acceptNumChannels(numInputChannels, numOutputChannels, numSidechainChannels)) {
    return kResultFalse;
  }
  getAudioInput(0)->setArrangement(inputs[0]);
  getAudioInput(0)->setName(STR16("Input"));
  getAudioOutput(0)->setArrangement(inputs[0]);
  getAudioOutput(0)->setName(STR16("Output"));
  if (hasSidechain) {
    getAudioInput(0)->setArrangement(inputs[1]);
    getAudioInput(0)->setName(STR16("Sidechain"));
  }
  return kResultTrue;
}

unplug::NumIO UnplugProcessor::getNumIO()
{
  auto const input = getAudioInput(0);
  auto const output = getAudioOutput(0);
  assert(input);
  if (input) {
    BusInfo inputInfo{};
    bool const gotInputInfoOk = input->getInfo(inputInfo);
    assert(gotInputInfoOk);
    BusInfo outputInfo{};
    bool const gotOutputInfoOk = output->getInfo(outputInfo);
    assert(gotOutputInfoOk);
    if (gotInputInfoOk && gotOutputInfoOk) {
      return { inputInfo.channelCount, outputInfo.channelCount };
    }
  }
  return { 0, 0 };
}

bool UnplugProcessor::onNotify(IMessage* message)
{
  return AudioEffect::notify(message) == kResultOk;
}

bool UnplugProcessor::onSetBusArrangements(SpeakerArrangement* inputs,
                                           int32 numIns,
                                           SpeakerArrangement* outputs,
                                           int32 numOuts)
{
  bool const hasSidechain = false;
  return acceptSimpleBusArrangement(inputs,
                                    numIns,
                                    outputs,
                                    numOuts,
                                    hasSidechain,
                                    [](int numInputChannels, int numOutputChannels, int numSidechainChannnels) {
                                      return numInputChannels == numOutputChannels;
                                    });
}

} // namespace Steinberg::Vst