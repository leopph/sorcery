#pragma once

#include <cstdint>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/mutex.hpp>
#include <sorcery/object_id.hpp>
#include <sorcery/object_ptr.hpp>
#include <sorcery/observer_ptr.hpp>


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


#include <sorcery/host/object_registry.inl>
