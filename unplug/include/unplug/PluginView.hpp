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
#include "unplug/detail/EventHandler.hpp"
#ifdef UNPLUG_VST3
#include "unplug/detail/Vst3View.hpp"
#else
// todo
#endif
namespace unplug::vst3 {

namespace detail {
#ifdef UNPLUG_VST3
template<class EventHandlerClass>
using View = Vst3View<EventHandlerClass>;
#else
// todo
#endif
} // namespace detail

template<class UserInterface>
using PluginView = detail::View<unplug::detail::EventHandler<UserInterface>>;

} // namespace unplug::vst3