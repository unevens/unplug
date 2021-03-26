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
#include "unplug/DemoView.hpp"

using namespace Steinberg;

namespace unplug {

//------------------------------------------------------------------------
// charlieController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::initialize(FUnknown* context)
{
  // Here the Plug-in will be instanciated

  //---do not forget to call parent ------
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  // Here you could register some parameters

  return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::terminate()
{
  // Here the Plug-in will be de-instanciated, last possibility to remove some
  // memory!

  //---do not forget to call parent ------
  return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::setComponentState(IBStream* state)
{
  // Here you get the state of the component (Processor part)
  if (!state)
    return kResultFalse;

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::setState(IBStream* state)
{
  // Here you get the state of the controller

  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::getState(IBStream* state)
{
  // Here you are asked to deliver the state of the controller (if needed)
  // Note: the real state of your plug-in is saved in the processor

  return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API
UnPlugDemoEffectController::createView(FIDString name)
{
  // Here the Host wants to open your editor (if you have one)
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    auto ui = new unplug::DemoView("UnplugDemo");
    return ui;
  }
  return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
{
  // called by host to update your parameters
  tresult result = EditControllerEx1::setParamNormalized(tag, value);
  return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::getParamStringByValue(Vst::ParamID tag,
                                                  Vst::ParamValue valueNormalized,
                                                  Vst::String128 string)
{
  // called by host to get a string for given normalized value of a specific
  // parameter (without having to set the value!)
  return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
UnPlugDemoEffectController::getParamValueByString(Vst::ParamID tag,
                                                  Vst::TChar* string,
                                                  Vst::ParamValue& valueNormalized)
{
  // called by host to get a normalized value from a string representation of a
  // specific parameter (without having to set the value!)
  return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace unplug
