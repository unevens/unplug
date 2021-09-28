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
#include "unplug/NumIO.hpp"
#include <cstring>

namespace unplug {

struct BlockSizeInfo final
{
  float sampleRate = 44100;
  float refreshRate = 30;
  Index maxAudioBlockSize = 128;
  NumIO numIO;

  bool operator==(BlockSizeInfo const& other) const noexcept
  {
    return std::memcmp(this, &other, sizeof(BlockSizeInfo)) == 0;
  }
  bool operator!=(BlockSizeInfo const& other) const noexcept
  {
    return !(*this == other);
  }
};

} // namespace unplug