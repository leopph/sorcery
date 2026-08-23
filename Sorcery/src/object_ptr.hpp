#pragma once

#include <concepts>

#include "Core.hpp"
#include "Object.hpp"
#include "observer_ptr.hpp"
#include "reflection.hpp"


namespace sorcery {
template<typename T>
class ObjectPtr {
public:
  ObjectPtr() noexcept = default;
  ObjectPtr(nullptr_t null) noexcept;
  explicit ObjectPtr(ObserverPtr<T> obj) noexcept requires std::derived_from<T, Object>;

  template<typename U> requires
    std::derived_from<std::remove_cv_t<U>, Object> &&
    std::derived_from<std::remove_cv_t<T>, Object> &&
    std::convertible_to<U*, T*>
  ObjectPtr(ObjectPtr<U> const& other) noexcept;

  [[nodiscard]]
  auto Get() const -> ObserverPtr<T> requires std::derived_from<T, Object>;

  [[nodiscard]]
  auto operator->() const -> ObserverPtr<T> requires std::derived_from<T, Object>;

  [[nodiscard]]
  auto operator*() const -> T& requires std::derived_from<T, Object>;

  [[nodiscard]]
  operator bool() const;

private:
  template<typename>
  friend class ObjectPtr;

  ObjectId id_;
};


template<std::derived_from<Object> T>
[[nodiscard]]
auto MakeObjectPtr(ObserverPtr<T> object) noexcept -> ObjectPtr<T>;


template<std::derived_from<Object> To, std::derived_from<Object> From>
[[nodiscard]]
auto ReflCast(ObjectPtr<From> obj) noexcept -> ObjectPtr<To>;


namespace detail {
[[nodiscard]] SORCERYAPI
auto ResolveObject(ObjectId id) -> ObserverPtr<Object>;
}
}


template<std::derived_from<sorcery::Object> T>
struct rttr::wrapper_mapper<sorcery::ObjectPtr<T>> {
  using wrapped_type = T*;
  using type = sorcery::ObjectPtr<T>;

  static auto get(type const& obj_ptr) -> wrapped_type;

  static auto create(wrapped_type const& ptr) -> type;

  template<typename U>
  static auto convert(type const& obj_ptr, bool& ok) -> sorcery::ObjectPtr<U>;
};


#include "object_ptr.inl"
