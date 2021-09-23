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

#include "unplug/Index.hpp"
#include <vector>
#include <type_traits>

namespace unplug::detail {

struct CachedIO final
{
public:
  class Channels final
  {
  public:
    Index numChannels = 0;

    template<class SampleType>
    SampleType** getChannels() const
    {
      static_assert(std::is_same_v<SampleType, double> || std::is_same_v<SampleType, float>);
      if constexpr (std::is_same_v<SampleType, double>) {
        return channels64;
      }
      else {
        return channels32;
      }
    }

    template<class SampleType>
    void setChannels(SampleType** channels)
    {
      static_assert(std::is_same_v<SampleType, double> || std::is_same_v<SampleType, float>);
      if constexpr (std::is_same_v<SampleType, double>) {
        channels64 = channels;
      }
      else {
        channels32 = channels;
      }
    }

  private:
    union
    {
      float** channels32{ nullptr };
      double** channels64;
    };
  };

  std::vector<Channels> ins;
  std::vector<Channels> outs;

  void resize(Index numIns, Index numOuts)
  {
    assert(numIns >= 0 && numOuts >= -1);
    ins.resize(std::max(numIns, Index(0)));
    outs.resize(std::max(numOuts, Index(0)));
  }
};

} // namespace unplug::detail
