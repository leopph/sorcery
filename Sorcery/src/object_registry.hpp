#pragma once

#include <cstdint>
#include <vector>

#include "mutex.hpp"
#include "object_id.hpp"
#include "observer_ptr.hpp"


namespace sorcery {
class Object;


struct ObjectSlot {
  ObserverPtr<Object> obj{nullptr};
  std::uint32_t gen{0};
};


class ObjectRegistry {
public:
  [[nodiscard]]
  auto Register(ObserverPtr<Object> obj) -> ObjectId;
  auto Unregister(ObjectId id) -> void;

  [[nodiscard]]
  auto Resolve(ObjectId id) const -> ObserverPtr<Object>;

private:
  mutable Mutex<std::vector<ObjectSlot>, true> slots_;
  Mutex<std::vector<std::uint32_t>> free_slot_indices_;
};
}
