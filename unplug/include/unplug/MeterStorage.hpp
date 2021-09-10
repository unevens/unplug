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
#include "Meters.hpp"
#include "unplug/Index.hpp"
#include <array>
#include <atomic>

namespace unplug {

template<int numValues>
class TMeterStorage final
{
public:
  void set(MeterIndex index, float value);

  float get(MeterIndex index) const;

  TMeterStorage();

  TMeterStorage(TMeterStorage const&) = delete;

  TMeterStorage& operator=(TMeterStorage&) = delete;

private:
  std::array<std::atomic<float>, numValues> values;
};

using MeterStorage = TMeterStorage<NumMeters::value>;

MeterStorage* getMeters();

namespace detail {
void setMeters(MeterStorage*);
} // namespace detail

// implementation

template<int numValues>
void TMeterStorage<numValues>::set(MeterIndex index, float value)
{
  values[index].store(value, std::memory_order_release);
}

template<int numValues>
float TMeterStorage<numValues>::get(MeterIndex index) const
{
  return values[index].load(std::memory_order_acquire);
}

template<int numValues>
TMeterStorage<numValues>::TMeterStorage()
{
  for (auto& value : values) {
    value = 0.0;
  }
}

} // namespace unplug