#pragma once

namespace sorcery {
template<typename T>
AssetRef<T>::AssetRef([[maybe_unused]] nullptr_t const null) noexcept :
  id_{AssetId::Invalid()} {}


template<typename T>
AssetRef<T>::AssetRef(ObserverPtr<T> asset) noexcept requires std::derived_from<T, Asset> :
  id_{asset ? asset->GetAssetId() : AssetId::Invalid()},
  cached_{asset} {}


template<typename T>
AssetRef<T>::AssetRef(AssetId const& asset_id) noexcept requires std::derived_from<T, Asset> :
  id_{asset_id} {}


template<typename T>
template<typename U> requires
  std::derived_from<std::remove_cv_t<U>, Asset> &&
  std::derived_from<std::remove_cv_t<T>, Asset> &&
  std::convertible_to<U*, T*>
AssetRef<T>::AssetRef(AssetRef<U> const& other) noexcept :
  id_{other.id_},
  cached_{other.cached_} {}


template<typename T>
auto AssetRef<T>::Get() const -> ObjectPtr<T> requires std::derived_from<T, Asset> {
  // We want to support AssetRefs that hold pointers to assets not-yet saved or
  // registered in the AssetManager. Those might not have valid IDs yet. So we can
  // only avoid reading the cache if we know FOR SURE that it cannot possibly point to
  // an object. If the cache does not track any Object identity, it won't be able to 
  // resolve to anything. If our asset ID is also invalid, we will not able to update
  // the cache either. So we can be sure that we're just wasting our time.
  if (!cached_.HasIdentity() && !id_.IsValid()) {
    return nullptr;
  }

  if (cached_) {
    return cached_;
  }

  UpdateCache();
  return cached_;
}


template<typename T>
auto AssetRef<T>::Observe() const -> ObserverPtr<T> requires std::derived_from<T, Asset> {
  if (auto const obj = cached_.Get()) {
    return obj;
  }

  // This function is designed for hot paths. If the cache returned null, it might not even be
  // possible to resolve the ID. We are preventing the potential unnecessary Resolve attempt here.
  if (!id_.IsValid()) {
    return nullptr;
  }

  UpdateCache();
  return cached_.Get();
}


template<typename T>
auto AssetRef<T>::operator->() const -> ObjectPtr<T> requires std::derived_from<T, Asset> {
  return Get();
}


template<typename T>
auto AssetRef<T>::operator*() const -> T& requires std::derived_from<T, Asset> {
  return *Get();
}


template<typename T>
AssetRef<T>::operator bool() const {
  return Observe() != nullptr;
}


template<typename T>
auto AssetRef<T>::UpdateCache() const -> void {
  cached_ = ReflCast<T>(detail::ResolveAsset(id_));
}


template<std::derived_from<Asset> T>
auto MakeAssetRef(ObserverPtr<T> const asset) noexcept -> AssetRef<T> {
  return AssetRef{asset};
}


template<std::derived_from<Asset> To, std::derived_from<Asset> From>
auto ReflCast(AssetRef<From> asset) noexcept -> AssetRef<To> {
  return MakeAssetRef(ReflCast<To>(asset.Get()));
}
}


template<std::derived_from<sorcery::Asset> T>
auto rttr::wrapper_mapper<sorcery::AssetRef<T>>::get(type const& asset_ref) -> wrapped_type {
  return asset_ref.Get().Get().Get();
}


template<std::derived_from<sorcery::Asset> T>
auto rttr::wrapper_mapper<sorcery::AssetRef<T>>::create(wrapped_type const& ptr) -> type {
  return sorcery::MakeAssetRef(sorcery::MakeObserver(ptr));
}


template<std::derived_from<sorcery::Asset> T>
template<typename U>
auto rttr::wrapper_mapper<sorcery::AssetRef<T>>::convert(type const& asset_ref, bool& ok) -> sorcery::AssetRef<U> {
  auto* const asset = rttr_cast<typename sorcery::AssetRef<U>::wrapped_type>(get(asset_ref));
  ok = asset != nullptr;
  return wrapper_mapper<sorcery::AssetRef<U>>::create(asset);
}
