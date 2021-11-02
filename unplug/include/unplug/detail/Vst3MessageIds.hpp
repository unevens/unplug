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

namespace unplug::vst3::messageId {

inline constexpr auto programChangeId = "unplug program change message";
inline constexpr auto programIndexId = "unplug program index";

inline constexpr auto meterSharingId = "unplug meters message";
inline constexpr auto meterStorageId = "unplug meters storage";
inline constexpr auto sharedDataStorageId = "unplug shared data storage";

inline constexpr auto userInterfaceChangedId = "unplug user interface message";
inline constexpr auto userInterfaceStateId = "unplug user interface state";

inline constexpr auto updateLatencyId = "unplug update latency";
inline constexpr auto updateLatencyParamChangedTagId = "unplug update latency param id";
inline constexpr auto updateLatencyParamChangedValueId = "unplug update latency param value";

} // namespace unplug::vst3::messageId