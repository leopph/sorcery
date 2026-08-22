#pragma once

#include <concepts>

#include "Core.hpp"
#include "object_ptr.hpp"
#include "observer_ptr.hpp"
#include "Reflection.hpp"
#include "resources/Resource.hpp"


namespace sorcery {
template<std::derived_from<Resource> T>
class ResourceRef {
public:
  ResourceRef() noexcept = default;
  ResourceRef(nullptr_t null) noexcept;
  ResourceRef(ObserverPtr<T> res) noexcept;
  explicit ResourceRef(ResourceId const& res_id) noexcept;

  [[nodiscard]]
  auto Get() const -> ObjectPtr<T>;

  [[nodiscard]]
  auto operator->() const -> ObjectPtr<T>;

  [[nodiscard]]
  auto operator*() const -> T&;

  [[nodiscard]]
  operator bool() const;

private:
  ResourceId id_;
  mutable ObjectPtr<T> cached_;
};


template<std::derived_from<Resource> T>
[[nodiscard]]
auto MakeResourceRef(ObserverPtr<T> resource) noexcept -> ResourceRef<T>;


namespace detail {
[[nodiscard]] SORCERYAPI
auto ResolveResource(ResourceId const& id) -> ObserverPtr<Resource>;
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
