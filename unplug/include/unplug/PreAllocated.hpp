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
#include "LockFreeMessenger.hpp"
#include <cassert>

namespace unplug {

template<class Object>
class PreAllocated final
{
public:
  Object* getFromAudioThread()
  {
    auto newObject = messengerForNewObjects.receiveLastMessage();
    if (newObject) {
      messengerForOldObjects.send(std::move(currentObjectStorage));
      currentObjectStorage = std::move(newObject.value());
      currentObjectPtr.store(currentObjectStorage.get(), std::memory_order_release);
    }
    return currentObjectStorage.get();
  }

  Object* getFromUiThread()
  {
    return currentObjectPtr.load(std::memory_order_acquire);
  }

  void set(std::unique_ptr<Object> newObject)
  {
    lockfree::receiveAndHandleMessageStack(messengerForOldObjects, [](auto& object) { object.reset(); });
    messengerForNewObjects.send(std::move(newObject));
  }

  void preallocateMessageNodes(int numNodesToPreallocate)
  {
    messengerForNewObjects.preallocateNodes(numNodesToPreallocate);
    messengerForOldObjects.preallocateNodes(numNodesToPreallocate);
  }

  explicit PreAllocated(int numNodesToPreallocate = 128)
  {
    preallocateMessageNodes(numNodesToPreallocate);
  }

private:
  lockfree::Messenger<std::unique_ptr<Object>> messengerForNewObjects;
  lockfree::Messenger<std::unique_ptr<Object>> messengerForOldObjects;
  std::unique_ptr<Object> currentObjectStorage;
  std::atomic<Object*> currentObjectPtr{ nullptr };
};

} // namespace unplug
