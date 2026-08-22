#include "object_registry.hpp"

#include <cassert>


namespace sorcery {
// ReSharper disable once CppMemberFunctionMayBeConst
// Logically not const
auto ObjectRegistry::Register(ObserverPtr<Object> const obj) -> ObjectId {
  auto data = data_.Lock();

  if (data->free_indices.empty()) {
    auto const idx = static_cast<std::uint32_t>(data->slots.size());
    data->slots.emplace_back(obj, 0);
    return ObjectId{.idx = idx, .gen = 0};
  }

  auto const idx = data->free_indices.back();
  data->free_indices.pop_back();

  auto& [slot_obj, slot_gen] = data->slots[idx];
  slot_obj = obj;
  return ObjectId{.idx = idx, .gen = slot_gen};
}


// ReSharper disable once CppMemberFunctionMayBeConst
// Logically not const
auto ObjectRegistry::Unregister(ObjectId const id) -> void {
  auto data = data_.Lock();
  assert(id.idx < data->slots.size());
  auto& [slot_obj, slot_gen] = data->slots[id.idx];
  assert(slot_gen == id.gen);
  assert(slot_obj);

  slot_obj = nullptr;
  ++slot_gen;

  data->free_indices.emplace_back(id.idx);
}


auto ObjectRegistry::Resolve(ObjectId const id) const -> ObserverPtr<Object> {
  auto const data = data_.LockShared();

  if (id.idx >= data->slots.size()) {
    return nullptr;
  }

  auto const& [obj, gen] = data->slots[id.idx];

  if (gen != id.gen) {
    return nullptr;
  }

  return obj;
}


auto ObjectRegistry::Instance() -> ObjectRegistry& {
  static ObjectRegistry instance;
  return instance;
}
}
