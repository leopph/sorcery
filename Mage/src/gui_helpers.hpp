#pragma once

#include <concepts>
#include <format>
#include <functional>
#include <string>
#include <type_traits>

#include <imgui.h>

#include "Object.hpp"
#include "object_ptr.hpp"
#include "resource_manager.hpp"
#include "Resources/Resource.hpp"


namespace sorcery::mage {
namespace detail {
class ObjectPickerBase {
protected:
  [[nodiscard]] static auto GetNextInstanceId() noexcept -> int;

private:
  static int sNextInstanceId;
};
}


template<std::derived_from<Object> T>
class ObjectPicker : detail::ObjectPickerBase {
public:
  // Returns whether an assignment was made.
  [[nodiscard]] auto Draw(ObjectPtr<T>& target_obj, bool allow_null = true) noexcept -> bool;

private:
  using StoredType = std::conditional_t<std::derived_from<T, Resource>, ResourceManager::ResourceInfo, ObserverPtr<T>>;

  auto QueryObjects(bool insert_null) noexcept -> void;

  std::vector<StoredType> objects_;
  std::string filter_;

  int const instance_id_{GetNextInstanceId()};
  std::string const popup_id_{std::format("PopupObjectPicker{}", instance_id_)};
  std::string const button_label_{std::format("Select##ObjectPicker{}", instance_id_)};
  std::string const input_text_label_{std::format("###FilterObjectPicker{}", instance_id_)};

  constexpr static std::string_view kNullDisplayName{"None"};
};


struct ObjectDragDropPayload {
  Object* ptr;
  constexpr static std::string_view kTypeStr{"OBJECT_DRAG_DROP_PAYLOAD"};
};


template<typename F>
decltype(auto) ImGuiDisabled(bool disabled, F&& func);

auto DrawSpinner(char const* label, float radius, int thickness, ImU32 const& color) -> bool;

template<typename T>
auto DrawReflectedProperties(T& obj, bool allow_edit, bool& changed) -> void;
}


#include "gui_helpers.inl"
