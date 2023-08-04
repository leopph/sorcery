#include "Object.hpp"

#include <cassert>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Object>{"Object"}
    .property("name", &sorcery::Object::mName);
}


namespace sorcery {
std::vector<ObserverPtr<Object>> Object::sAllObjects;


Object::Object() {
  sAllObjects.emplace_back(this);
}


Object::~Object() {
  std::erase(sAllObjects, this);

  for (auto const otherObj : sAllObjects) {
    for (auto const prop : rttr::type::get(*otherObj).get_properties()) {
      if (prop.get_type().is_pointer()) {
        if (prop.get_value(*otherObj).get_value<Object*>() == this) {
          auto const success{prop.set_value(*otherObj, nullptr)};
          assert(success);
        }
      }
    }
  }
}


auto Object::GetName() const noexcept -> std::string const& {
  return mName;
}


auto Object::SetName(std::string const& name) -> void {
  mName = name;
}


auto Object::DestroyAll() -> void {
  while (!sAllObjects.empty()) {
    delete sAllObjects.back();
  }
}
}
