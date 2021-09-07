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

#ifdef _MSC_VER

namespace unplug {

class DenormaFlusher final
{
public:
  DenormaFlusher() {
    _controlfp_s(&prev_word, 0, 0);

    unsigned int new_word;
    _controlfp_s(&new_word, _DN_FLUSH, _MCW_DN);

    word_was_changed = (prev_word != new_word);
  }

  ~DenormaFlusher() {
    if (word_was_changed) {
      unsigned int word;
      _controlfp_s(&word, _DN_SAVE, _MCW_DN);
      assert(word == prev_word);
    }
  }

private:
  bool word_was_changed;
  unsigned int prev_word;
};

} // namespace unplug

#else

#include <fenv.h>

namespace unplug {

class DenormaFlusher final
{
public:
  DenormaFlusher() {
    fegetenv(&prev_env);
    fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);
  }

  ~DenormaFlusher() { fesetenv(&prev_env); }

private:
  fenv_t prev_env;
};

} // namespace unplug

#endif