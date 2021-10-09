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

#include "unplug/detail/CachedIO.hpp"

namespace unplug {

namespace detail {
struct CachedIO;
}

/**
 * A class that represents the plugin inputs and outputs
 * */
template<class SampleType>
class IO final
{
public:
  /**
   * A struct that holds the audio channels of an input or an output
   * */
  struct Channels
  {
    SampleType** buffers{ nullptr };
    Index numChannels = 0;
  };

  /**
   * Getter for the inputs
   * @inIndex the index of the desired input
   * @return the channels for the specified input
   * */
  Channels getIn(Index inIndex)
  {
    auto const& in = io.ins[inIndex];
    return { in.template getChannels<SampleType>(), in.numChannels };
  }

  /**
   * Getter for the outputs
   * @inIndex the index of the desired output
   * @return the channels for the specified output
   * */
  Channels getOut(Index outIndex)
  {
    auto const& out = io.outs[outIndex];
    return { out.template getChannels<SampleType>(), out.numChannels };
  }

  /**
   * @return true if the host is "flushing" the plugin (calling the process with no inputs and no outputs)
   * */
  bool isFlushing() const
  {
    return io.isFlushing;
  }

  explicit IO(detail::CachedIO& io)
    : io{ io }
  {}

private:
  detail::CachedIO& io;
};

} // namespace unplug