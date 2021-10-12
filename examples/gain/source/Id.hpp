#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

static const Steinberg::FUID kProcessorUID(0x53624720, 0xA72E4FC2, 0xA4764509, 0xDF0D66D3);
static const Steinberg::FUID kControllerUID(0x3C63C830, 0xE4954F61, 0xBD9F749F, 0xDB7AC21E);

/*
To generate UUIDs ready to be copy-pasted into the Steinberg::FUID constructor, you can use this code

  FUID fuid;
  fuid.generate();
  char8 fuidText[100];
  fuid.print(fuidText,FUID::kFUID);
  std::cout << fuidText;

 */

#define UnplugGainExampleVST3Category "Fx"
