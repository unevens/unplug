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
#include <array>
#include <string_view>

/**
 * The functions in the namespace unplug::UserInterface have to be implemented by the plugin.
 * */

namespace unplug::UserInterface {

/**
 * This function implements the user interface of the plugin.
 * */
void paint();

/**
 * This function is used to define any custom ImGui style. It is called before the function "paint"
 * */
void setupStyle();

/**
 * This function is used to adjust the size of the plugin.
 * */
void adjustSize(int& width, int& height, int prevWidth, int prevHeight);

/**
 * This function is used to tell the host if the plugin user interface can be resized.
 * */
bool isResizingAllowed();

/**
 * If this function returns true, then any resizing will keep the original ratio.
 * */
bool keepDefaultRatio();

/**
 * @return the default resolution of the plugin user interface
 * */
std::array<int, 2> getDefaultSize();

/**
 * @return the minimum zoom factor of the plugin
 * */
float getMinZoom();

/**
 * @return the window name of the plugin user interface
 * */
const char* getWindowName();

/**
 * @return the color of the plugin background
 * */
std::array<float, 3> getBackgroundColor();

/**
 * @return the desired refresh rate for the plugin user interface
 * */
float getRefreshRate();

} // namespace unplug::UserInterface