#pragma once

#include "editor_drawer_context.hpp"
#include "Reflection.hpp"


namespace sorcery::mage {
class EditorDrawerBase {
  RTTR_ENABLE()

public:
  [[nodiscard]] virtual
  auto GetTargetType() const -> rttr::type = 0;

  virtual
  auto Draw(EditorDrawerContext const& ctx, rttr::instance obj, bool allow_edit, bool& changed) -> void = 0;

  EditorDrawerBase() = default;
  EditorDrawerBase(EditorDrawerBase const& other) = default;
  EditorDrawerBase(EditorDrawerBase&& other) noexcept = default;

  virtual ~EditorDrawerBase() = default;

  auto operator=(EditorDrawerBase const& other) -> EditorDrawerBase& = default;
  auto operator=(EditorDrawerBase&& other) noexcept -> EditorDrawerBase& = default;
};


template<typename T>
class EditorDrawer : public EditorDrawerBase {
  RTTR_ENABLE(EditorDrawerBase)

public:
  [[nodiscard]]
  auto GetTargetType() const -> rttr::type final;

  auto Draw(EditorDrawerContext const& ctx, rttr::instance obj, bool allow_edit, bool& changed) -> void final;

private:
  virtual
  auto Draw(EditorDrawerContext const& ctx, T& obj, bool allow_edit, bool& changed) -> void = 0;
};
}


#include "editor_drawer.inl"
