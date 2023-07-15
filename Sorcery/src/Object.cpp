#include "Object.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Object>{ "Object" };
}


namespace sorcery {
std::vector<ObserverPtr<Object>> Object::sAllObjects;


Object::Object() {
  sAllObjects.emplace_back(this);
}


Object::~Object() {
  std::erase(sAllObjects, this);
}


auto Object::GetName() const noexcept -> std::string_view {
  return mName;
}


auto Object::SetName(std::string name) noexcept -> void {
  mName = std::move(name);
}
}
