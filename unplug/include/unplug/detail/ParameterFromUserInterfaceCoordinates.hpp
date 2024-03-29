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
#include <string>
#include <vector>

namespace unplug::detail {

class ParameterFromUserInterfaceCoordinates
{
  struct Rectangle
  {
    ParamIndex paramIndex;
    int left;
    int top;
    int right;
    int bottom;
  };

  std::vector<Rectangle> rectangles;

public:
  bool findParameterFromUserInterfaceCoordinates(int xPos, int yPos, ParamIndex& paramIndex) const;
  void addParameterRectangle(ParamIndex paramIndex, int left, int top, int right, int bottom);
  void clear();
};

} // namespace unplug::detail