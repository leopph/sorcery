#pragma once

#include <cstdint>
#include <vector>

#include "Core.hpp"
#include "mutex.hpp"
#include "object_id.hpp"
#include "object_ptr.hpp"
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

  [[nodiscard]] SORCERYAPI
  auto Resolve(ObjectId id) const -> ObserverPtr<Object>;

  template<std::derived_from<Object> T>
  [[nodiscard]]
  auto FindObjectOfType() -> ObjectPtr<T>;

  template<std::derived_from<Object> T>
  auto FindObjectsOfType(std::vector<ObjectPtr<T>>& out) -> std::vector<ObjectPtr<T>>&;

  template<std::derived_from<Object> T>
  [[nodiscard]]
  auto FindObjectsOfType() -> std::vector<ObjectPtr<T>>;

private:
  struct RegistryData {
    std::vector<ObjectSlot> slots;
    std::vector<std::uint32_t> free_indices;
  };


  mutable Mutex<RegistryData, true> data_;
};
}


#include "object_registry.inl"
