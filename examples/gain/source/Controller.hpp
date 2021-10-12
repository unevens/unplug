#pragma once

#include "unplug/UnplugController.hpp"

namespace Steinberg::Vst {

class Controller final : public UnplugController
{
public:
  static FUnknown* createInstance(void* /*context*/)
  {
    return (IEditController*)new Controller;
  }
};

} // namespace Steinberg::Vst