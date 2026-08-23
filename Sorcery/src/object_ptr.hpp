#pragma once

#include <concepts>

#include "Core.hpp"
#include "Object.hpp"
#include "observer_ptr.hpp"
#include "reflection.hpp"


namespace sorcery {
// A persistent pointer to an Object.
// Semantically, an ObserverPtr<Object> is the memory address of an Object instance.
// An ObjectPtr expresses the identity of the Object regardless of its memory address.
// In practice, ObjectPtr will point to its object as long as it exists. Once it's
// destroyed, ObjectPtr will be considered null and won't resolve to anything.
// This guarantee has some performance overhead over regular ObserverPtr,
// so the recommended usage is to hold ObjectPtr for persistent identity and
// resolving it to ObserverPtr for immediate use.
template<typename T>
class ObjectPtr {
public:
  // Create a null pointer.
  ObjectPtr() noexcept = default;
  // Create a null pointer.
  ObjectPtr(nullptr_t null) noexcept;
  // Create an ObjectPtr that identifies the Object instance this pointer points to.
  explicit ObjectPtr(ObserverPtr<T> obj) noexcept requires std::derived_from<T, Object>;

  template<typename U> requires
    std::derived_from<std::remove_cv_t<U>, Object> &&
    std::derived_from<std::remove_cv_t<T>, Object> &&
    std::convertible_to<U*, T*>
  ObjectPtr(ObjectPtr<U> const& other) noexcept;

  // Observe the current instance of the Object.
  // If the Object no longer lives, this returns nullptr.
  [[nodiscard]]
  auto Get() const -> ObserverPtr<T> requires std::derived_from<T, Object>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator->() const -> ObserverPtr<T> requires std::derived_from<T, Object>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator*() const -> T& requires std::derived_from<T, Object>;

  // Does the ObjectPtr point to a living Object?
  [[nodiscard]]
  operator bool() const;

  // Does the ObjectPtr hold a valid Object identity?
  // Due to performance reasons it is sometimes beneficial to avoid resolving
  // the pointed-to Object and enough to know that the pointer is structurally
  // null and cannot resolve to anything.
  [[nodiscard]]
  auto HasIdentity() const -> bool;

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
