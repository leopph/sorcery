#pragma once

#include <concepts>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "app.hpp"
#include "Core.hpp"
#include "Object.hpp"
#include "resource_manager.hpp"
#include "Util.hpp"
#include "Resources/Resource.hpp"


namespace sorcery {
LEOPPHAPI auto SetImGuiContext(ImGuiContext& ctx) -> void;


namespace detail {
class ObjectPickerBase {
  static int sNextInstanceId;

protected:
  [[nodiscard]] LEOPPHAPI static auto GetNextInstanceId() noexcept -> int;
};
}


template<std::derived_from<Object> T>
class ObjectPicker : detail::ObjectPickerBase {
public:
  // Returns whether an assignment was made.
  [[nodiscard]] auto Draw(T*& targetObj, bool allowNull = true) noexcept -> bool;

private:
  using StoredType = std::conditional_t<std::derived_from<T, Resource>, ResourceManager::ResourceInfo, T*>;

  auto QueryObjects(bool insertNull) noexcept -> void;

  std::vector<StoredType> mObjects;
  std::string mFilter;
  int const mInstanceId{GetNextInstanceId()};
  std::string const mPopupId{std::format("PopupObjectPicker{}", mInstanceId)};
  std::string const mButtonLabel{std::format("Select##ObjectPicker{}", mInstanceId)};
  std::string const mInputTextLabel{std::format("###FilterObjectPicker{}", mInstanceId)};

  constexpr static std::string_view NULL_DISPLAY_NAME{"None"};
};


struct ObjectDragDropPayload {
  Object* ptr;
  constexpr static std::string_view kTypeStr{"OBJECT_DRAG_DROP_PAYLOAD"};
};


template<typename F>
decltype(auto) ImGuiDisabled(bool disabled, F&& func);
}


#include "gui_helpers.inl"
