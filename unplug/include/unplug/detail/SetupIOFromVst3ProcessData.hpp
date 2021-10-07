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

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "unplug/IO.hpp"
#include <cassert>

namespace unplug::detail {

template<class SampleType>
SampleType** getBuffer(Steinberg::Vst::AudioBusBuffers& buffers)
{
  static_assert(std::is_same_v<SampleType, double> || std::is_same_v<SampleType, float>);
  if constexpr (std::is_same_v<SampleType, double>) {
    return buffers.channelBuffers64;
  }
  else {
    return buffers.channelBuffers32;
  }
}

template<class SampleType>
void setupIO(CachedIO& io, Steinberg::Vst::ProcessData& data)
{
  using namespace detail;
  auto const isFlushing = data.numInputs == 0 && data.numOutputs == 0;
  if (isFlushing)
    return;
  assert(data.numInputs == io.ins.size());
  assert(data.numOutputs == io.outs.size());
  io.ins.resize(data.numInputs);
  io.outs.resize(data.numOutputs);
  for (Index in = 0; in < data.numInputs; ++in) {
    io.ins[in].setChannels(getBuffer<SampleType>(data.inputs[in]));
    io.ins[in].numChannels = data.inputs[in].numChannels;
  }
  for (Index out = 0; out < data.numOutputs; ++out) {
    io.outs[out].setChannels(getBuffer<SampleType>(data.outputs[out]));
    io.outs[out].numChannels = data.outputs[out].numChannels;
  }
}

} // namespace unplug::detail