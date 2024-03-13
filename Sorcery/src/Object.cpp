#include "Object.hpp"

#include <imgui.h>

#include <cassert>
#include <format>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Object>{"Object"}
    .property("name", &sorcery::Object::mName);
}


namespace sorcery {
std::vector<Object*> Object::sAllObjects;
std::recursive_mutex Object::sAllObjectsMutex;


auto Object::GetName() const noexcept -> std::string const& {
  return mName;
}


auto Object::SetName(std::string const& name) -> void {
  mName = name;
}


auto Object::OnInit() -> void {
  std::unique_lock const lock{sAllObjectsMutex};
  sAllObjects.emplace_back(this);
}


auto Object::OnDestroy() -> void {
  std::unique_lock const lock{sAllObjectsMutex};

  std::erase(sAllObjects, this);

  for (auto const otherObj : sAllObjects) {
    for (auto const prop : rttr::type::get(*otherObj).get_properties()) {
      if (prop.get_type().is_pointer()) {
        if (prop.get_value(*otherObj).get_value<Object*>() == this) {
          [[maybe_unused]] auto const success{prop.set_value(*otherObj, nullptr)};
          assert(success);
        }
      }
    }
  }
}


auto Object::OnDrawProperties([[maybe_unused]] bool& changed) -> void {
  ImGui::SeparatorText(std::format("{} ({})", GetName(), rttr::type::get(*this).get_name().data()).c_str());
}


auto Object::DestroyAll() -> void {
  std::unique_lock const lock{sAllObjectsMutex};

  while (!sAllObjects.empty()) {
    Destroy(*sAllObjects.back());
  }
}


auto Destroy(Object& obj) -> void {
  obj.OnDestroy();
  delete std::addressof(obj);
}
}
