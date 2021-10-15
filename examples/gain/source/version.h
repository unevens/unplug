#pragma once

#include "pluginterfaces/base/fplatform.h"

//Here you can define the credits of your plugin
#define stringPluginName "UnplugGainExample"
#define stringCompanyName "unevens\0"
#define UNPLUG_PLUGIN_VENDOR_URL "https://unevens.net"
#define UNPLUG_PLUGIN_VENDOR_MAIL "mailto:hi@unevens.net"
#define stringLegalCopyright "Copyright(c) 2021 Dario Mambro."
#define stringOriginalFilename "UnplugGainExample" ".vst3"
#if SMTG_PLATFORM_64
#define stringFileDescription "UnplugGainExample" " VST3 (64Bit)"
#else
#define stringFileDescription "UnplugGainExample" " VST3"
#endif

// Here you can define the version of your plug-in: "Major.Sub.Release.Build""
#define MAJOR_VERSION_STR "1"
#define MAJOR_VERSION_INT 1

#define SUB_VERSION_STR "0"
#define SUB_VERSION_INT 0

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR "1" // Build number to be sure that each result could be identified.
#define BUILD_NUMBER_INT 1

#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR
#define stringLegalTrademarks "VST is a trademark of Steinberg Media Technologies GmbH"
