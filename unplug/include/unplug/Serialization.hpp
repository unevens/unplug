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
#ifdef UNPLUG_VST3
#include "base/source/fstreamer.h"
#endif

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace unplug {

namespace Serialization {
enum Action
{
  read,
  write
};

template<Action action>
class Streamer final
{
  Steinberg::IBStreamer& stream;

public:
  Streamer(Steinberg::IBStreamer& stream)
    : stream{ stream }
  {}

  bool operator()(float& value)
  {
    if constexpr (action == read) {
      return stream.readFloat(value);
    }
    if constexpr (action == write) {
      return stream.writeFloat(value);
    }
  }

  bool operator()(double& value)
  {
    if constexpr (action == read) {
      return stream.readDouble(value);
    }
    if constexpr (action == write) {
      return stream.writeDouble(value);
    }
  }

  bool operator()(float* data, std::size_t numElements)
  {
    if constexpr (action == read) {
      return stream.readFloatArray(data, numElements);
    }
    if constexpr (action == write) {
      return stream.writeFloatArray(data, numElements);
    }
  }

  bool operator()(double* data, std::size_t numElements)
  {
    if constexpr (action == read) {
      return stream.readDoubleArray(data, numElements);
    }
    if constexpr (action == write) {
      return stream.writeDoubleArray(data, numElements);
    }
  }

  bool operator()(int32_t& value)
  {
    auto as32 = static_cast<Steinberg::int32>(value);
    if constexpr (action == read) {
      auto const success = stream.readInt32(as32);
      if (success)
        value = as32;
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt32(as32);
    }
  }

  bool operator()(int64_t& value)
  {
    auto as64 = static_cast<Steinberg::int64>(value);
    if constexpr (action == read) {
      auto const success = stream.readInt64(as64);
      if (success)
        value = as64;
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt64(as64);
    }
  }

  bool operator()(uint32_t& value)
  {
    auto as32 = static_cast<Steinberg::uint32>(value);
    if constexpr (action == read) {
      auto const success = stream.readInt32u(as32);
      if (success)
        value = as32;
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt32u(as32);
    }
  }

  bool operator()(uint64_t& value)
  {
    auto as64 = static_cast<Steinberg::uint64>(value);
    if constexpr (action == read) {
      auto const success = stream.readInt64u(as64);
      if (success)
        value = as64;
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt64u(as64);
    }
  }

  bool operator()(int32_t* data, std::size_t numElements)
  {
    auto numElements32 = static_cast<Steinberg::int32>(numElements);
    auto as32 = std::vector<Steinberg::int32>(data, data + numElements);
    if constexpr (action == read) {
      auto const success = stream.readInt32Array(as32.data(), numElements32);
      std::copy(as32.begin(), as32.end(), data);
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt32Array(as32.data(), numElements32);
    }
  }

  bool operator()(int64_t* data, std::size_t numElements)
  {
    auto numElements64 = static_cast<Steinberg::int64>(numElements);
    auto as64 = std::vector<Steinberg::int64>(data, data + numElements);
    if constexpr (action == read) {
      auto const success = stream.readInt64Array(as64.data(), numElements64);
      std::copy(as64.begin(), as64.end(), data);
      return success;
    }
    if constexpr (action == write) {
      return stream.writeInt64Array(as64.data(), numElements64);
    }
  }

  template<class T, std::size_t size>
  bool operator()(std::array<T, size>& value)
  {
    return this->operator()(value.data(), size);
  }

  template<class T>
  bool operator()(std::vector<T>& value)
  {
    return this->operator()(value.data(), value.size());
  }

  bool operator()(std::string& value)
  {
    if constexpr (action == read) {
      auto ptr = stream.readStr8();
      if (ptr) {
        value = ptr;
        delete ptr;
        return true;
      }
      else {
        return false;
      }
    }
    if constexpr (action == write) {
      return stream.writeStr8(value.c_str());
    }
  }
};

} // namespace Serialization

} // namespace unplug