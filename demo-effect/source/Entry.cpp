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
#include "Processor.hpp"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "UnPlugDemoEffect"

//------------------------------------------------------------------------
//  Module init/exit
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// called after library was loaded
bool
InitModule()
{
  return true;
}

//------------------------------------------------------------------------
// called after library is unloaded
bool
DeinitModule()
{
  return true;
}

using namespace Steinberg::Vst;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF("unevens", "https://unevens.net", "mailto:hi@unevens.net")

//---First Plug-in included in this factory-------
// its kVstAudioEffectClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kUnPlugDemoEffectProcessorUID),
           PClassInfo::kManyInstances,                // cardinality
           kVstAudioEffectClass,                      // the component category (do not changed this)
           stringPluginName,                          // here the Plug-in name (to be changed)
           Vst::kDistributable,                       // means that component and controller could be
                                                      // distributed on different computers
           UnPlugDemoEffectVST3Category,              // Subcategory for this Plug-in (to be changed)
           FULL_VERSION_STR,                          // Plug-in version (to be changed)
           kVstVersionString,                         // the VST 3 SDK version (do not changed this, use always
                                                      // this define)
           UnPlugDemoEffectProcessor::createInstance) // function pointer called when
                                                      // this component should be
                                                      // instantiated

// its kVstComponentControllerClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kUnPlugDemoEffectControllerUID),
           PClassInfo::kManyInstances,                 // cardinality
           kVstComponentControllerClass,               // the Controller category (do not changed this)
           stringPluginName "Controller",              // controller name (could be the same than component name)
           0,                                          // not used here
           "",                                         // not used here
           FULL_VERSION_STR,                           // Plug-in version (to be changed)
           kVstVersionString,                          // the VST 3 SDK version (do not changed this, use always
                                                       // this define)
           UnPlugDemoEffectController::createInstance) // function pointer called when
                                                       // this component should be
                                                       // instantiated

//----for others Plug-ins contained in this factory, put like for the first
// Plug-in different DEF_CLASS2---

END_FACTORY
