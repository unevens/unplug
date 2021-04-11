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
#include <cstdint>
#include <string>
#include <vector>

namespace unplug {

struct ViewPersistentData
{
  std::vector<double> numbers;
  std::vector<int64_t> integers;
  std::vector<std::string> strings;
  std::vector<char> bytes;

  template<class SaveInteger, class SaveIntegerArray, class SaveDoubleArray, class SaveBytes>
  bool save(SaveInteger saveInteger,
            SaveIntegerArray saveIntegerArray,
            SaveDoubleArray saveDoubleArray,
            SaveBytes saveBytes) const
  {
    {
      auto const numIntegersToSave = static_cast<int64_t>(integers.size());
      if (!saveInteger(numIntegersToSave))
        return false;
      if (!saveIntegerArray(integers.data(), numIntegersToSave))
        return false;
    }
    {
      auto const numNumbersToSave = static_cast<int64_t>(numbers.size());
      if (!saveInteger(numNumbersToSave))
        return false;
      if (!saveDoubleArray(numbers.data(), numNumbersToSave))
        return false;
    }
    {
      auto const numStringsToSave = static_cast<int64_t>(strings.size());
      if (!saveInteger(numStringsToSave))
        return false;
      for (int i = 0; i < numStringsToSave; ++i) {
        auto const cString = strings[i].c_str();
        auto const stringLength = static_cast<int64_t>(strlen(cString));
        if (!saveInteger(stringLength))
          return false;
        if (!saveBytes(cString, stringLength))
          return false;
      }
    }
    {
      auto const numBytesToSave = static_cast<int64_t>(bytes.size());
      if (!saveInteger(numBytesToSave))
        return false;
      if (!saveBytes(bytes.data(), numBytesToSave))
        return false;
    }
    return true;
  }

  template<class LoadInteger, class LoadIntegerArray, class LoadDoubleArray, class LoadBytes>
  bool load(LoadInteger loadInteger,
            LoadIntegerArray loadIntegerArray,
            LoadDoubleArray loadDoubleArray,
            LoadBytes loadBytes)
  {
    auto copy = ViewPersistentData{};
    bool const loaded = copy.loadInternal(loadInteger, loadIntegerArray, loadDoubleArray, loadBytes);
    if (loaded) {
      *this = copy;
    }
    return loaded;
  }

private:
  template<class LoadInteger, class LoadIntegerArray, class LoadDoubleArray, class LoadBytes>
  bool loadInternal(LoadInteger loadInteger,
                    LoadIntegerArray loadIntegerArray,
                    LoadDoubleArray loadDoubleArray,
                    LoadBytes loadBytes)
  {
    {
      int64_t numIntegersToLoad = 0;
      if (!loadInteger(numIntegersToLoad))
        return false;
      integers.resize(numIntegersToLoad);
      if (!loadIntegerArray(integers.data(), numIntegersToLoad))
        return false;
    }
    {
      int64_t numNumbersToLoad = 0;
      if (!loadInteger(numNumbersToLoad))
        return false;
      numbers.resize(numNumbersToLoad);
      if (!loadDoubleArray(numbers.data(), numNumbersToLoad))
        return false;
    }
    {
      int64_t numStringsToLoad = 0;
      if (!loadInteger(numStringsToLoad))
        return false;
      strings.reserve(numStringsToLoad);
      for (int i = 0; i < numStringsToLoad; ++i) {
        int64_t stringLength = 0;
        if (!loadInteger(stringLength))
          return false;
        std::vector<char> buffer;
        buffer.resize(stringLength + 1);
        if (!loadBytes(buffer.data(), stringLength))
          return false;
        buffer[stringLength] = '\0';
        strings.emplace_back(buffer.data());
      }
    }
    {
      int64_t numBytesToLoad = 0;
      if (!loadInteger(numBytesToLoad))
        return false;
      bytes.resize(numBytesToLoad);
      if (!loadBytes(bytes.data(), numBytesToLoad))
        return false;
    }
    return true;
  }
};

} // namespace unplug