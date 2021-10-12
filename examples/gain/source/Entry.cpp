#include "Controller.hpp"
#include "Id.hpp"
#include "Processor.hpp"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

//------------------------------------------------------------------------
//  Module init/exit
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// called after library was loaded
bool InitModule()
{
  return true;
}

//------------------------------------------------------------------------
// called after library is unloaded
bool DeinitModule()
{
  return true;
}

using namespace Steinberg::Vst;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF(stringCompanyName, UNPLUG_PLUGIN_VENDOR_URL, UNPLUG_PLUGIN_VENDOR_MAIL)

//---First Plug-in included in this factory-------
// its kVstAudioEffectClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kProcessorUID),
           PClassInfo::kManyInstances,    // cardinality
           kVstAudioEffectClass,          // the component category (do not changed this)
           stringPluginName,              // here the Plug-in name (to be changed)
           Vst::kDistributable,           // means that component and controller could be
                                          // distributed on different computers
           UnplugGainExampleVST3Category,  // Subcategory for this Plug-in (to be changed)
           FULL_VERSION_STR,              // Plug-in version (to be changed)
           kVstVersionString,             // the VST 3 SDK version (do not changed this, use always
                                          // this define)
           Processor::createInstance) // function pointer called when
                                          // this component should be
                                          // instantiated

// its kVstComponentControllerClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kControllerUID),
           PClassInfo::kManyInstances,     // cardinality
           kVstComponentControllerClass,   // the Controller category (do not changed this)
           stringPluginName "Controller",  // controller name (could be the same than component name)
           0,                              // not used here
           "",                             // not used here
           FULL_VERSION_STR,               // Plug-in version (to be changed)
           kVstVersionString,              // the VST 3 SDK version (do not changed this, use always
                                           // this define)
           Controller::createInstance) // function pointer called when
                                           // this component should be
                                           // instantiated

//----for others Plug-ins contained in this factory, put like for the first
// Plug-in different DEF_CLASS2---

END_FACTORY
