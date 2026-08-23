#pragma once

#include <concepts>

#include "Core.hpp"
#include "object_ptr.hpp"
#include "observer_ptr.hpp"
#include "reflection.hpp"
#include "resources/Resource.hpp"


namespace sorcery {
template<typename T>
class ResourceRef {
public:
  ResourceRef() noexcept = default;
  ResourceRef(nullptr_t null) noexcept;
  explicit ResourceRef(ObserverPtr<T> res) noexcept requires std::derived_from<T, Resource>;
  explicit ResourceRef(ResourceId const& res_id) noexcept requires std::derived_from<T, Resource>;

  template<typename U> requires
    std::derived_from<std::remove_cv_t<U>, Resource> &&
    std::derived_from<std::remove_cv_t<T>, Resource> &&
    std::convertible_to<U*, T*>
  ResourceRef(ResourceRef<U> const& other) noexcept;

  [[nodiscard]]
  auto Get() const -> ObjectPtr<T> requires std::derived_from<T, Resource>;

  [[nodiscard]]
  auto operator->() const -> ObjectPtr<T> requires std::derived_from<T, Resource>;

  [[nodiscard]]
  auto operator*() const -> T& requires std::derived_from<T, Resource>;

  [[nodiscard]]
  operator bool() const;

private:
  template<typename>
  friend class ResourceRef;

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
