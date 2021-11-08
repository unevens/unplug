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
#include "unplug/GetVersion.hpp"
#include "unplug/Presets.hpp"
#include "unplug/UserInterface.hpp"
#include "unplug/detail/GetSortedParameterDescriptions.hpp"
#include "unplug/detail/Vst3MessageIds.hpp"
#include <numeric>

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

  ioCache.resize(1, 1);

  sharedDataWrapped = std::make_shared<SharedDataWrapped>();
  pluginState.sharedData = &(sharedDataWrapped->get());
  pluginState.meters = std::make_shared<MeterStorage>();

  onInitialization();

  return kResultOk;
}

tresult PLUGIN_API UnplugProcessor::terminate()
{
  onTermination();
  return AudioEffect::terminate();
}

void UnplugProcessor::updateNotAutomatableParameters(ProcessData& data)
{
  if (pluginState.parameters.getNumNotAutomatableParameters() == 0)
    return;
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
          auto const isAutomatable = pluginState.parameters.isParameterAutomatable(parameterTag);
          if (!isAutomatable) {
            pluginState.parameters.setNormalized(static_cast<ParamIndex>(parameterTag), value);
          }
        }
      }
    }
  }
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
          pluginState.parameters.setNormalized(static_cast<ParamIndex>(parameterTag), value);
        }
      }
    }
  }
}

template<unplug::Serialization::Action action>
bool UnplugProcessor::serialization(IBStreamer& ibStreamer)
{
  using namespace unplug::Serialization;
  auto streamer = Streamer<action>(ibStreamer);
  Version version;
  if (!streamer(version.data(), version.size())) {
    return false;
  }
  for (int i = 0; i < NumParameters::value; ++i) {
    if constexpr (action == save) {
      double value = pluginState.parameters.getNormalized(i);
      if (!streamer(value)) {
        return false;
      }
    }
    else {
      double value = 0;
      if (!streamer(value)) {
        return false;
      }
      pluginState.parameters.setNormalized(i, value);
    }
  }
  auto const ok = pluginState.sharedData->template serialization<action>(streamer);
  return ok;
}

tresult PLUGIN_API UnplugProcessor::setState(IBStream* state)
{
  if (!state)
    return kResultFalse;
  using namespace unplug::Serialization;
  IBStreamer streamer(state, kLittleEndian);
  auto const success = serialization<load>(streamer);
  if (!success)
    return kResultFalse;
  return onSetState(streamer) ? kResultOk : kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::getState(IBStream* state)
{
  if (!state)
    return kResultFalse;
  using namespace unplug::Serialization;
  IBStreamer streamer(state, kLittleEndian);
  auto const success = serialization<save>(streamer);
  if (!success)
    return kResultFalse;
  return onGetState(streamer) ? kResultOk : kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64)
    return supportsDoublePrecision() ? kResultTrue : kResultFalse;

  return kResultFalse;
}

tresult PLUGIN_API UnplugProcessor::notify(IMessage* message)
{
  using namespace vst3::messageId;
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
  else if (FIDStringsEqual(message->getMessageID(), updateLatencyId)) {
    int64 paramId;
    if (kResultTrue != message->getAttributes()->getInt(vst3::messageId::updateLatencyParamChangedTagId, paramId)) {
      return kResultFalse;
    }
    double paramValue;
    if (kResultTrue !=
        message->getAttributes()->getFloat(vst3::messageId::updateLatencyParamChangedValueId, paramValue)) {
      return kResultFalse;
    }
    pluginState.parameters.set(paramId, paramValue);
    updateLatency((ParamIndex)paramId, paramValue);
    return kResultOk;
  }
  else
    return onNotify(message) ? kResultOk : kResultFalse;
}

tresult UnplugProcessor::setActive(TBool state)
{
  if (state) {
    contextInfo.sampleRate = static_cast<float>(processSetup.sampleRate);
    contextInfo.userInterfaceRefreshRate = UserInterface::getRefreshRate();
    contextInfo.maxAudioBlockSize = processSetup.maxSamplesPerBlock;
    contextInfo.numIO = updateNumIO();
    contextInfo.precision =
      processSetup.symbolicSampleSize == kSample64 ? FloatingPointPrecision::float64 : FloatingPointPrecision::float32;
    setup();
  }
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
  bool acceptSideChain,
  const std::function<bool(int numInputs, int numOutputs, int numSideChain)>& acceptNumChannels)
{
  if (numOuts != 1) {
    return kResultFalse;
  }
  if (acceptSideChain) {
    if (numIns != 1 && numIns != 2) {
      return kResultFalse;
    }
  }
  else {
    if (numIns != 1) {
      return kResultFalse;
    }
  }
  bool const hasSideChain = numIns == 2;
  auto const numInputChannels = SpeakerArr::getChannelCount(inputs[0]);
  auto const numOutputChannels = SpeakerArr::getChannelCount(outputs[0]);
  auto const numSideChainChannels = hasSideChain ? SpeakerArr::getChannelCount(inputs[1]) : 0;
  if (!acceptNumChannels(numInputChannels, numOutputChannels, numSideChainChannels)) {
    return kResultFalse;
  }
  getAudioInput(0)->setArrangement(inputs[0]);
  getAudioInput(0)->setName(STR16("Input"));
  getAudioOutput(0)->setArrangement(inputs[0]);
  getAudioOutput(0)->setName(STR16("Output"));
  if (hasSideChain) {
    getAudioInput(0)->setArrangement(inputs[1]);
    getAudioInput(0)->setName(STR16("SideChain"));
  }
  return kResultTrue;
}

unplug::NumIO UnplugProcessor::updateNumIO()
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
      return { static_cast<unplug::Index>(inputInfo.channelCount),
               static_cast<unplug::Index>(outputInfo.channelCount) };
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
  bool const hasSideChain = false;
  return acceptSimpleBusArrangement(inputs,
                                    numIns,
                                    outputs,
                                    numOuts,
                                    hasSideChain,
                                    [](int numInputChannels, int numOutputChannels, int numSideChainChannnels) {
                                      return numInputChannels == numOutputChannels;
                                    });
}
void UnplugProcessor::sendSharedDataToController()
{
  auto message = owned(allocateMessage());
  message->setMessageID(vst3::messageId::meterSharingId);
  auto const meterStorageAddress = reinterpret_cast<uintptr_t>(&pluginState.meters);
  message->getAttributes()->setBinary(
    vst3::messageId::meterStorageId, &meterStorageAddress, sizeof(meterStorageAddress));
  auto const sharedDataAddress = reinterpret_cast<uintptr_t>(&sharedDataWrapped);
  message->getAttributes()->setBinary(
    vst3::messageId::sharedDataStorageId, &sharedDataAddress, sizeof(sharedDataAddress));
  sendMessage(message);
}

tresult UnplugProcessor::connect(IConnectionPoint* other)
{
  auto const result = ComponentBase::connect(other);
  if (result == kResultTrue) {
    sendSharedDataToController();
  }
  return onConnect(other) ? kResultTrue : kResultFalse;
}

bool UnplugProcessor::setup()
{
  pluginState.sharedData->setup(contextInfo);
  return onSetup(contextInfo);
}

void UnplugProcessor::setLatency(uint32_t value)
{
  if (latency != value) {
    latency = value;
    auto message = owned(allocateMessage());
    message->setMessageID(vst3::messageId::updateLatencyId);
    sendMessage(message);
  }
}

} // namespace Steinberg::Vst