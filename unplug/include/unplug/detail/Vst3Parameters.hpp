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
#include "public.sdk/source/vst/vsteditcontroller.h"
#include <cassert>
#include <unordered_set>

namespace unplug {
namespace vst3 {

using EditControllerEx1 = Steinberg::Vst::EditControllerEx1;
using ParamValue = Steinberg::Vst::ParamValue;
using tresult = Steinberg::tresult;
static constexpr auto kResultTrue = Steinberg::kResultTrue;
static constexpr auto kResultFalse = Steinberg::kResultFalse;

class Parameters final
{
public:
  explicit Parameters(EditControllerEx1* controller)
    : controller(controller)
  {
    controller->addRef();
  }

  ~Parameters() { controller->release(); }

  double get(int tag) { return controller->getParamNormalized(tag); }

  bool set(int tag, double value)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    return controller->performEdit(tag, value) == kResultTrue;
  }

  bool beginEdit(int tag)
  {
    auto const editedIter = paramsBeingEdited.find(tag);
    if (editedIter != paramsBeingEdited.end()) {
      assert(false);
      return true;
    }
    if (controller->beginEdit(tag) == kResultTrue) {
      paramsBeingEdited.insert(tag);
      return true;
    }
    else {
      return false;
    }
  }

  bool endEdit(int tag)
  {
    auto editedIter = paramsBeingEdited.find(tag);
    if (editedIter == paramsBeingEdited.end()) {
      assert(false);
      return false;
    }
    if (controller->endEdit(tag) == kResultTrue) {
      paramsBeingEdited.insert(tag);
      return true;
    }
    else {
      return false;
    }
  }

private:
  EditControllerEx1* controller;
  std::unordered_set<int> paramsBeingEdited;
};

} // namespace vst3
} // namespace unplug