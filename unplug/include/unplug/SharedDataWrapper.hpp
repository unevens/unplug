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
#include "unplug/SetupPluginFromDsp.hpp"

namespace unplug {

namespace detail {
class EventHandler;
}

/**
 * A class that wraps the plugin custom data that will be available to both the user interface and the processor.
 * */
template<class TData>
class SharedDataWrapper final
{
public:
  using Data = TData;

  friend class detail::EventHandler;
  /**
   * Gets the plugin custom data from the user interface code
   * */
  static Data& getCurrent()
  {
    return *currentInstance;
  }

  /**
   * Gets the plugin custom data from the dsp code
   * */
  Data& get()
  {
    return data;
  }

  explicit SharedDataWrapper(unplug::SetupPluginFromDsp const& setupPlugin)
    : data(setupPlugin)
  {}

  SharedDataWrapper(SharedDataWrapper const&) = delete;

  SharedDataWrapper& operator=(SharedDataWrapper const&) = delete;

private:
  void setCurrent()
  {
    currentInstance = &data;
  }

  Data data;
  static inline thread_local Data* currentInstance = nullptr;
};

} // namespace unplug
