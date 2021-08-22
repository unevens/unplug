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

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

static const Steinberg::FUID kUnplugDemoEffectProcessorUID(0x53624720, 0xA72E4FC2, 0xA4764509, 0xDF0D66D3);
static const Steinberg::FUID kUnplugDemoEffectControllerUID(0x3C63C830, 0xE4954F61, 0xBD9F749F, 0xDB7AC21E);

/*
To generate UUIDs ready to be copy-pasted into the Steinberg::FUID constructor, you can use this code

  FUID fuid;
  fuid.generate();
  char8 fuidText[100];
  fuid.print(fuidText,FUID::kFUID);
  std::cout << fuidText;

 */

#define UnplugDemoEffectVST3Category "Fx"
