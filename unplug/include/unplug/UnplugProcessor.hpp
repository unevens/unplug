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

#include "CircularBuffers.hpp"
#include "Meters.hpp"
#include "Parameters.hpp"
#include "ProcessingData.hpp"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "unplug/GetParameterDescriptions.hpp"
#include "unplug/IO.hpp"
#include "unplug/MeterStorage.hpp"
#include "unplug/ParameterStorage.hpp"
#include <atomic>
#include <memory>

namespace Steinberg::Vst {

class UnplugProcessor : public AudioEffect
{
public:
  using Index = unplug::Index;
  using ParamId = unplug::ParamIndex;
  using MeterId = unplug::MeterIndex;
  template<class SampleType>
  using IO = unplug::IO<SampleType>;

  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  tresult PLUGIN_API initialize(FUnknown* context) override;

  tresult PLUGIN_API terminate() override;

  /** Will be called before any process call */
  tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup) override;

  /** For persistence */
  tresult PLUGIN_API setState(IBStream* state) override;
  tresult PLUGIN_API getState(IBStream* state) override;

  /** Reports if the plugin supports 32/64 bit floating point audio */
  Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

  tresult PLUGIN_API notify(IMessage* message) override;

  tresult PLUGIN_API setActive(TBool state) override;

  tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs,
                                        int32 numIns,
                                        SpeakerArrangement* outputs,
                                        int32 numOuts) override;

protected:
  template<class SampleType, class StaticProcessing, class AutomatedProcessing, class SetParameterAutomation>
  void processWithSamplePreciseAutomation(ProcessData& data,
                                          IO<SampleType> io,
                                          StaticProcessing staticProcessing,
                                          AutomatedProcessing automatedProcessing,
                                          SetParameterAutomation setParameterAutomation);

  void updateParametersToLastPoint(ProcessData& data);

  tresult acceptBusArrangement(
    SpeakerArrangement* inputs,
    int32 numIns,
    SpeakerArrangement* outputs,
    int32 numOuts,
    bool acceptSidechain,
    const std::function<bool(int numInputs, int numOutputs, int numSidechain)>& acceptNumChannels);

private:
  /** Called from initialize, at first after constructor */
  virtual void onInitialization();

  /** Called at the end before destructor, by terminate */
  virtual void onTermination() {}

  /**
   * Called by setupProcessing on the UI Thread, before the processing is started. The newSetup object contains
   * information such as the maximum number of samples per audio block and the currentSample rate, so this is the right
   * place where to allocate resources that depend on those.
   * */
  virtual void onSetupProcessing(ProcessSetup& newSetup) {}

  virtual bool supportsDoublePrecision()
  {
    return true;
  }

protected:
  unplug::ProcessingData processingData;
  std::array<int32, unplug::NumParameters::value> automationPointsHandled;
};

} // namespace Steinberg::Vst

namespace unplug {
using UnplugProcessor = Steinberg::Vst::UnplugProcessor;

namespace detail {
template<class SampleType>
SampleType** getBuffer(Steinberg::Vst::AudioBusBuffers& buffers)
{
  static_assert(std::is_same_v<SampleType, double> || std::is_same_v<SampleType, float>);
  if constexpr (std::is_same_v<SampleType, double>) {
    return buffers.channelBuffers64;
  }
  else {
    return buffers.channelBuffers32;
  }
}
} // namespace detail

template<class SampleType>
IO<SampleType> getIO(Steinberg::Vst::ProcessData& data)
{
  using namespace detail;
  return {
    data.numInputs >= 1 ? getBuffer<SampleType>(data.inputs[0]) : nullptr,
    data.numInputs >= 1 ? data.inputs[0].numChannels : 0,
    data.numOutputs >= 1 ? getBuffer<SampleType>(data.outputs[0]) : nullptr,
    data.numOutputs >= 1 ? data.outputs[0].numChannels : 0,
    data.numInputs >= 2 ? getBuffer<SampleType>(data.inputs[1]) : nullptr,
    data.numInputs >= 2 ? data.inputs[1].numChannels : 0,
  };
}

} // namespace unplug

namespace Steinberg::Vst {

template<class SampleType, class StaticProcessing, class AutomatedProcessing, class SetParameterAutomation>
void UnplugProcessor::processWithSamplePreciseAutomation(ProcessData& data,
                                                         IO<SampleType> io,
                                                         StaticProcessing staticProcessing,
                                                         AutomatedProcessing automatedProcessing,
                                                         SetParameterAutomation setParameterAutomation)
{
  if (data.inputParameterChanges) {
    int32 const numParamsChanged = data.inputParameterChanges->getParameterCount();
    if (numParamsChanged == 0) {
      staticProcessing(io, data.numSamples);
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
              auto const parameterId = paramQueue->getParameterId();
              value = processingData.parameters.valueFromNormalized(parameterId, value);
              auto const prevValue = processingData.parameters.get(parameterId);
              setParameterAutomation(parameterId, -1, prevValue, sampleOffset, value);
              automationPointsHandled[index] = 1;
              --numChangesToHandle;
            }
          }
        }
      }
      // handle other changes until the end
      int32 currentSample = 0;
      while (numChangesToHandle > 0) {
        int32 nextSample = data.numSamples;
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
              const bool startsAtCurrentSample = sampleOffset == currentSample;
              if (startsAtCurrentSample) {
                value = processingData.parameters.valueFromNormalized(parameterId, value);
                ParamValue nextValue;
                int32 nextSampleOffset;
                // get the end
                auto const endPoint = point + 1;
                if (endPoint < numPoints) {
                  paramQueue->getPoint(endPoint, nextSampleOffset, nextValue);
                  nextValue = processingData.parameters.valueFromNormalized(parameterId, nextValue);
                }
                else {
                  nextSampleOffset = data.numSamples;
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
                    nextValueAfterJump = processingData.parameters.valueFromNormalized(parameterId, nextValueAfterJump);
                  }
                  else {
                    nextSampleOffsetAfterJump = data.numSamples;
                    nextValueAfterJump = nextValue;
                  }
                  setParameterAutomation(
                    parameterId, sampleOffset, nextValue, nextSampleOffsetAfterJump, nextValueAfterJump);
                  nextSample = std::min(nextSample, nextSampleOffsetAfterJump);
                }
                else {
                  setParameterAutomation(parameterId, sampleOffset, value, nextSampleOffset, nextValue);
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
        automatedProcessing(io, currentSample, nextSample);
        currentSample = nextSample;
      }
    }
  }
  else {
    staticProcessing(io, data.numSamples);
  }
  updateParametersToLastPoint(data);
}

} // namespace Steinberg::Vst