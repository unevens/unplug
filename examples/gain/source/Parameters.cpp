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

#include "Parameters.hpp"
#include "unplug/GetParameterDescriptions.hpp"

namespace unplug {

std::vector<ParameterDescription> getParameterDescriptions()
{
  auto parameters = std::vector<ParameterDescription>();
  parameters.push_back(ParameterDescription::makeBypassParameter(Param::bypass));
  parameters.push_back(ParameterDescription(Param::gain, "Gain", -90.0, 6.0, 0.0).ControlledByDecibels());
  parameters.push_back(
//    ParameterDescription(Param::oversamplingOrder, "OverSampling", 0,5,0,5)
//      .EditPolicy(ParamEditPolicy::notAutomatableAndMayChangeLatencyOnEdit));
          ParameterDescription(Param::oversamplingOrder, "OverSampling", { "1x", "2x", "4x", "8x", "16x", "32x" })
      .EditPolicy(ParamEditPolicy::notAutomatableAndMayChangeLatencyOnEdit));
  parameters.push_back(ParameterDescription(Param::oversamplingLinearPhase, "Linear Phase", 0, 1, 0, 1)
                         .EditPolicy(ParamEditPolicy::notAutomatableAndMayChangeLatencyOnEdit));
  return parameters;
}

} // namespace unplug
