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

#include "Meters.hpp"
#include "Parameters.hpp"
#include "PluginState.hpp"
#include "SharedData.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "unplug/AutomationEvent.hpp"
#include "unplug/GetParameterDescriptions.hpp"
#include "unplug/IO.hpp"
#include "unplug/MeterStorage.hpp"
#include "unplug/ParameterStorage.hpp"
#include "unplug/Serialization.hpp"
#include "unplug/detail/SetupIOFromVst3ProcessData.hpp"
#include <atomic>
#include <memory>
#include <unplug/detail/SetupIOFromVst3ProcessData.hpp>

namespace Steinberg::Vst {

class UnplugProcessor : public AudioEffect
{
protected:
  using Index = unplug::Index;
  using ParamId = unplug::ParamIndex;
  using MeterId = unplug::MeterIndex;
  template<class SampleType>
  using IO = unplug::IO<SampleType>;
  using NumIO = unplug::NumIO;
  using ContextInfo = unplug::ContextInfo;

  /** helper function for processing without sample precise automation */
  template<class SampleType, class StaticProcessing, class Upsampling, class Downsampling>
  void staticProcessing(ProcessData& data,
                        StaticProcessing staticProcessing_,
                        Upsampling upsampling,
                        Downsampling downsampling);

  /** helper function for processing without sample precise automation */
  template<class SampleType, class StaticProcessing>
  void staticProcessing(ProcessData& data, StaticProcessing staticProcessing_)
  {
    staticProcessing<SampleType>(
      data, staticProcessing_, [](IO<SampleType> const&) {}, [](IO<SampleType> const&) {});
  }

  /** helper function for processing with sample precise automation */
  template<class SampleType,
           class StaticProcessing,
           class PrepareAutomation,
           class AutomatedProcessing,
           class SetParameterAutomation,
           class Upsampling,
           class Downsampling>
  void processWithSamplePreciseAutomation(ProcessData& data,
                                          StaticProcessing staticProcessing,
                                          PrepareAutomation prepareAutomation,
                                          AutomatedProcessing automatedProcessing,
                                          SetParameterAutomation setParameterAutomation,
                                          Upsampling upsampling,
                                          Downsampling downsampling);

  /** helper function for processing with sample precise automation */
  template<class SampleType,
           class StaticProcessing,
           class PrepareAutomation,
           class AutomatedProcessing,
           class SetParameterAutomation>
  void processWithSamplePreciseAutomation(ProcessData& data,
                                          StaticProcessing staticProcessing,
                                          PrepareAutomation prepareAutomation,
                                          AutomatedProcessing automatedProcessing,
                                          SetParameterAutomation setParameterAutomation)
  {
    processWithSamplePreciseAutomation<SampleType>(
      data,
      staticProcessing,
      prepareAutomation,
      automatedProcessing,
      setParameterAutomation,
      [](IO<SampleType> const&) {},
      [](IO<SampleType> const&) {});
  }

  /** updates the parameters to the last values received by the host  */
  void updateParametersToLastPoint(ProcessData& data);

  /** helper function to accept common bus arrangements  */
  tresult acceptSimpleBusArrangement(
    SpeakerArrangement* inputs,
    int32 numIns,
    SpeakerArrangement* outputs,
    int32 numOuts,
    bool acceptSidechain,
    const std::function<bool(int numInputs, int numOutputs, int numSidechain)>& acceptNumChannels);

  /** Gets information regarding the audio buffer */
  ContextInfo const& getContextInfo() const
  {
    return contextInfo;
  }

  /** Override this function to let unplug know about the oversampling rate used by your processing call */
  virtual Index getOversamplingRate() const
  {
    return 1;
  }

  /** Called from initialize, at first after constructor */
  virtual void onInitialization();

  /** Called at the end before destructor, by terminate */
  virtual void onTermination() {}

  /** Called by setActive on the UI Thread, before the processing is started, or after it is finished. */
  virtual void onSetActive(bool isActive) {}

  virtual bool supportsDoublePrecision()
  {
    return true;
  }

  /** used for communication with the controller */
  virtual bool onNotify(IMessage* message);

  virtual bool onSetBusArrangements(SpeakerArrangement* inputs,
                                    int32 numIns,
                                    SpeakerArrangement* outputs,
                                    int32 numOuts);

  virtual bool onGetState(IBStreamer& streamer)
  {
    return true;
  }

  virtual bool onSetState(IBStreamer& streamer)
  {
    return true;
  }

  virtual bool onConnect(IConnectionPoint* other)
  {
    return true;
  }

  virtual bool onSetup(ContextInfo const& context)
  {
    return true;
  }

  void updateLatency(Index procesorId, uint64_t processorLatency);

private:
  template<unplug::Serialization::Action>
  bool serialization(IBStreamer& streamer);

  void sendSharedDataToController();

  void sendLatencyChangedMessage();

  NumIO updateNumIO();

public:
  bool setup();

  tresult PLUGIN_API initialize(FUnknown* context) final;

  tresult PLUGIN_API terminate() final;

  tresult PLUGIN_API setState(IBStream* state) final;

  tresult PLUGIN_API getState(IBStream* state) final;

  /** Reports if the plugin supports 32/64 bit floating point audio */
  tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) final;

  tresult PLUGIN_API setActive(TBool state) final;

  /** used to handle messages from the controller */
  tresult PLUGIN_API notify(IMessage* message) final;

  tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs,
                                        int32 numIns,
                                        SpeakerArrangement* outputs,
                                        int32 numOuts) final;

  tresult PLUGIN_API connect(IConnectionPoint* other) final;

  uint32 PLUGIN_API getLatencySamples() override
  {
    return static_cast<uint32>(latency);
  }

protected:
  std::shared_ptr<unplug::SharedDataWrapped> sharedDataWrapped;
  unplug::PluginState pluginState;
  unplug::detail::CachedIO ioCache;
  std::array<int32, unplug::NumParameters::value> automationPointsHandled;
  std::vector<uint64_t> latencies;
  uint64_t latency;

private:
  ContextInfo contextInfo;
};

template<class SampleType, class StaticProcessing, class Upsampling, class Downsampling>
void UnplugProcessor::staticProcessing(ProcessData& data,
                                       StaticProcessing staticProcessing_,
                                       Upsampling upsampling,
                                       Downsampling downsampling)
{
  unplug::detail::setupIO<SampleType>(ioCache, data);
  auto io = IO<SampleType>(ioCache);
  updateParametersToLastPoint(data);
  bool const isNotFlushing = !io.isFlushing();
  if (isNotFlushing) {
    auto const oversamplingRate = getContextInfo().oversamplingRate;
    auto const numSamples = data.numSamples * oversamplingRate;
    upsampling(io);
    staticProcessing_(io, numSamples);
    downsampling(io);
  }
}

template<class SampleType,
         class StaticProcessing,
         class PrepareAutomation,
         class AutomatedProcessing,
         class SetParameterAutomation,
         class Upsampling,
         class Downsampling>
void UnplugProcessor::processWithSamplePreciseAutomation(ProcessData& data,
                                                         StaticProcessing staticProcessing_,
                                                         PrepareAutomation prepareAutomation,
                                                         AutomatedProcessing automatedProcessing,
                                                         SetParameterAutomation setParameterAutomation,
                                                         Upsampling upsampling,
                                                         Downsampling downsampling)
{
  using AutomationEvent = unplug::AutomationEvent<SampleType>;
  unplug::detail::setupIO<SampleType>(ioCache, data);
  auto io = IO<SampleType>(ioCache);
  bool const isNotFlushing = !io.isFlushing();
  if (isNotFlushing) {
    upsampling(io);
    auto const oversamplingRate = getContextInfo().oversamplingRate;
    auto const numSamples = data.numSamples * oversamplingRate;
    if (data.inputParameterChanges) {
      auto automation = prepareAutomation();
      int32 const numParamsChanged = data.inputParameterChanges->getParameterCount();
      if (numParamsChanged == 0) {
        staticProcessing_(io, numSamples);
      }
      else {
        // reset cache of handled points
        std::fill(automationPointsHandled.begin(), automationPointsHandled.begin() + numParamsChanged, 0);
        // handle first changes if not on zero
        int32 numChangesToHandle = 0;
        for (int32 index = 0; index < numParamsChanged; index++) {
          if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
            int32 const numPoints = paramQueue->getPointCount();
            numChangesToHandle += numPoints;
            if (numPoints > 0) {
              ParamValue value;
              int32 sampleOffset;
              paramQueue->getPoint(0, sampleOffset, value);
              if (sampleOffset > 0) {
                sampleOffset *= oversamplingRate;
                auto const parameterId = paramQueue->getParameterId();
                value = pluginState.parameters.valueFromNormalized(parameterId, value);
                auto const prevValue = pluginState.parameters.get(parameterId);
                setParameterAutomation(automation, AutomationEvent(parameterId, -1, prevValue, sampleOffset, value));
                automationPointsHandled[index] = 1;
                --numChangesToHandle;
              }
            }
          }
        }
        // handle other changes until the end
        int32 currentSample = 0;
        while (numChangesToHandle > 0) {
          int32 nextSample = numSamples;
          for (int32 index = 0; index < numParamsChanged; index++) {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData(index)) {
              int32 const numPoints = paramQueue->getPointCount();
              auto const parameterId = paramQueue->getParameterId();
              for (int32 point = automationPointsHandled[index]; point < numPoints; ++point) {
                ParamValue value;
                int32 sampleOffset;
                paramQueue->getPoint(point, sampleOffset, value);
                bool const onEnd = sampleOffset == data.numSamples;
                if (onEnd) {
                  --numChangesToHandle;
                  ++automationPointsHandled[index];
                  break;
                }
                sampleOffset *= oversamplingRate;
                const bool startsAtCurrentSample = sampleOffset == currentSample;
                if (startsAtCurrentSample) {
                  value = pluginState.parameters.valueFromNormalized(parameterId, value);
                  ParamValue nextValue;
                  int32 nextSampleOffset;
                  // get the end
                  auto const endPoint = point + 1;
                  if (endPoint < numPoints) {
                    paramQueue->getPoint(endPoint, nextSampleOffset, nextValue);
                    nextSampleOffset *= oversamplingRate;
                    nextValue = pluginState.parameters.valueFromNormalized(parameterId, nextValue);
                  }
                  else {
                    nextSampleOffset = numSamples;
                    nextValue = value;
                  }
                  bool const isJump = sampleOffset == nextSampleOffset;
                  if (isJump) {
                    ParamValue nextValueAfterJump;
                    int32 nextSampleOffsetAfterJump;
                    // get the end
                    auto const endPointAfterJump = endPoint + 1;
                    if (endPointAfterJump < numPoints) {
                      paramQueue->getPoint(endPointAfterJump, nextSampleOffsetAfterJump, nextValueAfterJump);
                      nextSampleOffsetAfterJump *= oversamplingRate;
                      nextValueAfterJump = pluginState.parameters.valueFromNormalized(parameterId, nextValueAfterJump);
                    }
                    else {
                      nextSampleOffsetAfterJump = numSamples;
                      nextValueAfterJump = nextValue;
                    }
                    setParameterAutomation(
                      automation,
                      AutomationEvent(
                        parameterId, sampleOffset, nextValue, nextSampleOffsetAfterJump, nextValueAfterJump));
                    nextSample = std::min(nextSample, nextSampleOffsetAfterJump);
                  }
                  else {
                    setParameterAutomation(
                      automation, AutomationEvent(parameterId, sampleOffset, value, nextSampleOffset, nextValue));
                    nextSample = std::min(nextSample, nextSampleOffset);
                  }
                  --numChangesToHandle;
                  ++automationPointsHandled[index];
                }
                else {
                  nextSample = std::min(nextSample, sampleOffset);
                }
              }
            }
          }
          automatedProcessing(automation, io, currentSample, nextSample);
          currentSample = nextSample;
        }
      }
    }
    else {
      staticProcessing_(io, numSamples);
    }
    downsampling(io);
  }
  updateParametersToLastPoint(data);
}

} // namespace Steinberg::Vst