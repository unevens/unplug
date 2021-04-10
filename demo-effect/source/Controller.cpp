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
#include "base/source/fstreamer.h"
#include "unplug/ParameterSettings.hpp"
#include "unplug/Vst3DemoView.hpp"

using namespace Steinberg;

namespace unplug {

using ViewClass = vst3::DemoView;

tresult PLUGIN_API
UnPlugDemoEffectController::initialize(FUnknown* context)
{
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  ViewClass::initializePersistentData(persistentData);

  return result;
}

tresult PLUGIN_API
UnPlugDemoEffectController::terminate()
{
  return EditControllerEx1::terminate();
}

tresult PLUGIN_API
UnPlugDemoEffectController::setComponentState(IBStream* state)
{
  // Here you get the state of the component (Processor part)
  if (!state)
    return kResultFalse;

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
