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

#include "unplug/detail/EditRegister.hpp"

namespace unplug::detail {

std::string ParameterEditRegister::getControllerEditingParameter(ParamIndex paramIndex) const
{
  auto it = paramsBeingEditedByControls.find(paramIndex);
  if (it != paramsBeingEditedByControls.end())
    return it->second;
  else
    return {};
}

bool ParameterEditRegister::isParameterBeingEdited(ParamIndex paramIndex) const
{
  return paramsBeingEditedByControls.find(paramIndex) != paramsBeingEditedByControls.end();
}

void ParameterEditRegister::unregisterEdit(ParamIndex paramIndex)
{
  paramsBeingEditedByControls.erase(paramIndex);
}

void ParameterEditRegister::registerEdit(ParamIndex paramIndex, std::string control)
{
  paramsBeingEditedByControls.emplace(paramIndex, std::move(control));
}
} // namespace unplug::detail