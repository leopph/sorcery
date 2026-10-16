#pragma once

#include <concepts>

#include <sorcery/Core.hpp>
#include <sorcery/object_ptr.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/reflection.hpp>
#include <sorcery/assets/Asset.hpp>


namespace sorcery {
// A persistent reference to an Asset.
// Semantically, an ObjectPtr<Asset> is the identity of a runtime instance of the asset.
// An AssetRef expresses a reference to a specific logical asset.
// In practice, AssetRef will refer to an instance of an asset as long as that logical asset
// exists. Once that asset is removed (e.g. goes out-of-scope at runtime or deleted in the Editor)
// it is considered null and won't resolve to anything.
// These semantics have a measurable performance overhead compared to ObserverPtr and even ObjectPtr.
// It is recommended to use AssetRef to express a persistent reference to a specific logical asset
// and resolve to ObjectPtr for identity tracking or ObserverPtr for immediate use.
template<typename T>
class AssetRef {
public:
  // Create a null reference.
  AssetRef() noexcept = default;
  // Create a null reference.
  AssetRef(nullptr_t null) noexcept;
  // Create a reference to the logical asset this object is an instance of.
  // It is also valid to pass an asset instance that has not yet been made logically
  // persistant (e.g. not yet saved in the Editor).
  explicit AssetRef(ObserverPtr<T> asset) noexcept requires std::derived_from<T, Asset>;
  // Create a reference to the logical asset with this ID.
  explicit AssetRef(AssetId const& asset_id) noexcept requires std::derived_from<T, Asset>;

  template<typename U> requires
    std::derived_from<std::remove_cv_t<U>, Asset> &&
    std::derived_from<std::remove_cv_t<T>, Asset> &&
    std::convertible_to<U*, T*>
  AssetRef(AssetRef<U> const& other) noexcept;

  // Returns a handle that represent the current instance of the logical asset.
  // If the asset identity is stale, this returns nullptr.
  [[nodiscard]]
  auto Get() const -> ObjectPtr<T> requires std::derived_from<T, Asset>;

  // Return a direct pointer to the current instance of the logical asset.
  // This is a performance optimization for immediate use.
  [[nodiscard]]
  auto Observe() const -> ObserverPtr<T> requires std::derived_from<T, Asset>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator->() const -> ObjectPtr<T> requires std::derived_from<T, Asset>;

  // Same semantics as Get.
  [[nodiscard]]
  auto operator*() const -> T& requires std::derived_from<T, Asset>;

  // Does the AssetRef refer to an existing, resolvable logical asset?
  [[nodiscard]]
  operator bool() const;

private:
  template<typename>
  friend class AssetRef;

  auto UpdateCache() const -> void;

  AssetId id_;
  mutable ObjectPtr<T> cached_;
};


template<std::derived_from<Asset> T>
[[nodiscard]]
auto MakeAssetRef(ObserverPtr<T> asset) noexcept -> AssetRef<T>;


template<std::derived_from<Asset> To, std::derived_from<Asset> From>
[[nodiscard]]
auto ReflCast(AssetRef<From> asset) noexcept -> AssetRef<To>;


namespace detail {
[[nodiscard]] SORCERYAPI
auto ResolveAsset(AssetId const& id) -> ObjectPtr<Asset>;
}
}


template<std::derived_from<sorcery::Asset> T>
struct rttr::wrapper_mapper<sorcery::AssetRef<T>> {
  using wrapped_type = T*;
  using type = sorcery::AssetRef<T>;

  static auto get(type const& asset_ref) -> wrapped_type;

  static auto create(wrapped_type const& ptr) -> type;

  template<typename U>
  static auto convert(type const& asset_ref, bool& ok) -> sorcery::AssetRef<U>;
};


#include <sorcery/asset_ref.inl>
