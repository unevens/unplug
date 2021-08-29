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
#include "ModifierKeys.h"
#include "imgui.h"
#include "pluginterfaces/base/keycodes.h"

namespace unplug::vst3::detail {

unplug::detail::ModifierKeys
modifierKeysFromBitmask(Steinberg::int16 mask)
{
  unplug::detail::ModifierKeys modifierKeys;
  modifierKeys.shift = mask & Steinberg::kShiftKey;
  modifierKeys.alt = mask & Steinberg::kAlternateKey;
  modifierKeys.control = mask & Steinberg::kControlKey;
  modifierKeys.command = mask & Steinberg::kCommandKey;
  return modifierKeys;
}

int
convertVirtualKeyCode(Steinberg::int16 virtualKeyCode)
{
  using namespace Steinberg;
  switch (virtualKeyCode) {
    case KEY_BACK:
      return ImGuiKey_Backspace;
    case KEY_TAB:
      return ImGuiKey_Tab;
    case KEY_RETURN:
      return ImGuiKey_Enter;
    case KEY_ESCAPE:
      return ImGuiKey_Escape;
    case KEY_SPACE:
      return ImGuiKey_Space;
    case KEY_END:
      return ImGuiKey_End;
    case KEY_HOME:
      return ImGuiKey_Home;
    case KEY_LEFT:
      return ImGuiKey_LeftArrow;
    case KEY_UP:
      return ImGuiKey_UpArrow;
    case KEY_RIGHT:
      return ImGuiKey_RightArrow;
    case KEY_DOWN:
      return ImGuiKey_DownArrow;
    case KEY_PAGEUP:
      return ImGuiKey_PageUp;
    case KEY_PAGEDOWN:
      return ImGuiKey_PageDown;
    case KEY_ENTER:
      return ImGuiKey_KeyPadEnter;
    case KEY_INSERT:
      return ImGuiKey_Insert;
    case KEY_DELETE:
      return ImGuiKey_Delete;
    default:
      return -1;
  }
}

int
convertNumPadKeyCode(Steinberg::int16 virtualKeyCode)
{
  using namespace Steinberg;
  switch (virtualKeyCode) {
    case KEY_NUMPAD0:
      return '0';
    case KEY_NUMPAD1:
      return '1';
    case KEY_NUMPAD2:
      return '2';
    case KEY_NUMPAD3:
      return '3';
    case KEY_NUMPAD4:
      return '4';
    case KEY_NUMPAD5:
      return '5';
    case KEY_NUMPAD6:
      return '6';
    case KEY_NUMPAD7:
      return '7';
    case KEY_NUMPAD8:
      return '8';
    case KEY_NUMPAD9:
      return '9';
    case KEY_MULTIPLY:
      return '*';
    case KEY_ADD:
      return '+';
    case KEY_SUBTRACT:
      return '-';
    case KEY_DIVIDE:
      return '/';
    case KEY_DECIMAL:
      return '.';
    default:
      return -1;
  }
}

} // namespace unplug