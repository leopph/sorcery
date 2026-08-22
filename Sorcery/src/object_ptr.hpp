#pragma once

#include <concepts>

#include "Object.hpp"
#include "observer_ptr.hpp"
#include "Reflection.hpp"


namespace sorcery {
template<std::derived_from<Object> T>
class ObjectPtr {
public:
  ObjectPtr() = default;
  ObjectPtr(nullptr_t null);
  ObjectPtr(ObserverPtr<T> obj);

  [[nodiscard]]
  auto Get() const -> ObserverPtr<T>;

  [[nodiscard]]
  auto operator->() const -> ObserverPtr<T>;

  [[nodiscard]]
  auto operator*() const -> T&;

  [[nodiscard]]
  operator bool() const;

private:
  ObjectId id_;
};
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
