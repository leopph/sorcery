#pragma once

#include <concepts>

#include "Core.hpp"
#include "object_ptr.hpp"
#include "observer_ptr.hpp"
#include "reflection.hpp"
#include "resources/Resource.hpp"


namespace sorcery {
// A persistent reference to a Resource.
// Semantically, an ObjectPtr<Resource> is the identity of a runtime instance of the resource.
// A ResourceRef expresses a reference to a specific logical resource.
// In practice, ResourceRef will refer to an instance of a resource as long as that logical resource
// exists. Once that resource is removed (e.g. goes out-of-scope at runtime or deleted in the Editor)
// it is considered null and won't resolve to anything.
// These semantics have a measurable performance overhead compared to ObserverPtr and even ObjectPtr.
// It is recommended to use ResourceRef to express a persistent reference to a specific logical resource
// and resolve to ObjectPtr for identity tracking or ObserverPtr for immediate use.
template<typename T>
class ResourceRef {
public:
  // Create a null reference.
  ResourceRef() noexcept = default;
  // Create a null reference.
  ResourceRef(nullptr_t null) noexcept;
  // Create a reference to the logical resource this object is an instance of.
  // It is also valid to pass a resource instance that has not yet been made logically
  // persistant (e.g. not yet saved in the Editor).
  explicit ResourceRef(ObserverPtr<T> res) noexcept requires std::derived_from<T, Resource>;
  // Create a reference to the logical resource with this ID.
  explicit ResourceRef(ResourceId const& res_id) noexcept requires std::derived_from<T, Resource>;

  template<typename U> requires
    std::derived_from<std::remove_cv_t<U>, Resource> &&
    std::derived_from<std::remove_cv_t<T>, Resource> &&
    std::convertible_to<U*, T*>
  ResourceRef(ResourceRef<U> const& other) noexcept;

  // Returns a handle that represent the current instance of the logical resource.
  // If the resource identity is stale, this returns nullptr.
  [[nodiscard]]
  auto Get() const -> ObjectPtr<T> requires std::derived_from<T, Resource>;

  // Return a direct pointer to the current instance of the logical resource.
  // This is a performance optimization for immediate use.
  [[nodiscard]]
  auto Observe() const -> ObserverPtr<T> requires std::derived_from<T, Resource>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator->() const -> ObjectPtr<T> requires std::derived_from<T, Resource>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator*() const -> T& requires std::derived_from<T, Resource>;

  // Does the ResourceRef refer to an existing, resolvable logical resource?
  [[nodiscard]]
  operator bool() const;

private:
  template<typename>
  friend class ResourceRef;

  auto UpdateCache() const -> void;

  ResourceId id_;
  mutable ObjectPtr<T> cached_;
};


template<std::derived_from<Resource> T>
[[nodiscard]]
auto MakeResourceRef(ObserverPtr<T> resource) noexcept -> ResourceRef<T>;


template<std::derived_from<Resource> To, std::derived_from<Resource> From>
[[nodiscard]]
auto ReflCast(ResourceRef<From> res) noexcept -> ResourceRef<To>;


namespace detail {
[[nodiscard]] SORCERYAPI
auto ResolveResource(ResourceId const& id) -> ObjectPtr<Resource>;
}
}


template<std::derived_from<sorcery::Resource> T>
struct rttr::wrapper_mapper<sorcery::ResourceRef<T>> {
  using wrapped_type = T*;
  using type = sorcery::ResourceRef<T>;

  static auto get(type const& res_ref) -> wrapped_type;

  static auto create(wrapped_type const& ptr) -> type;

  template<typename U>
  static auto convert(type const& res_ref, bool& ok) -> sorcery::ResourceRef<U>;
};


#include "resource_ref.inl"
