#include "object_registry.hpp"

#include <cassert>


namespace sorcery {
auto ObjectRegistry::Register(ObserverPtr<Object> const obj) -> ObjectId {
  if (free_slot_indices_.Lock()->empty()) {
    auto slots{slots_.Lock()};
    auto const idx{static_cast<std::uint32_t>(slots->size())};
    slots->emplace_back(obj, 0);
    return ObjectId{.idx = idx, .gen = 0};
  }

  auto const idx{
    [this] {
      auto free_slot_indices{free_slot_indices_.Lock()};
      auto const ret{free_slot_indices->back()};
      free_slot_indices->pop_back();
      return ret;
    }()
  };

  auto slots{slots_.Lock()};
  auto& [slot_obj, slot_gen]{(*slots)[idx]};
  slot_obj = obj;
  return ObjectId{.idx = idx, .gen = slot_gen};
}


auto ObjectRegistry::Unregister(ObjectId const id) -> void {
  {
    auto slots{slots_.Lock()};
    assert(id.idx < slots->size());
    auto& [slot_obj, slot_gen]{(*slots)[id.idx]};
    assert(slot_gen == id.gen);
    assert(slot_obj);

    slot_obj = nullptr;
    ++slot_gen;
  }

  auto free_slot_indices{free_slot_indices_.Lock()};
  free_slot_indices->emplace_back(id.idx);
}


auto ObjectRegistry::Resolve(ObjectId const id) const -> ObserverPtr<Object> {
  auto const slots{slots_.LockShared()};

  if (id.idx >= slots->size()) {
    return nullptr;
  }

  auto const& [obj, gen]{(*slots)[id.idx]};

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
