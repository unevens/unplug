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
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace unplug {
/**
 * A struct holding a plugin preset
 * */
struct Preset
{
  std::string name;
  std::unordered_map<int, std::string> pitchNames;
  std::map<ParamIndex, double> parameterValues;
};

/**
 * This function has to be implemented by the plugin
 * @return the plugin presets
 * */
std::vector<Preset> getPresets();

namespace detail {

class Presets
{
public:
  static std::vector<Preset> const& get()
  {
    static Presets instance;
    return instance.presets;
  }

private:
  Presets()
    : presets(getPresets())
  {}

  ~Presets() = default;
  Presets(const Presets&) = delete;
  Presets& operator=(const Presets&) = delete;

  std::vector<Preset> presets;
};

} // namespace detail

} // namespace unplug