#pragma once

namespace sorcery {
template<typename T>
ResourceRef<T>::ResourceRef([[maybe_unused]] nullptr_t const null) noexcept :
  id_{ResourceId::Invalid()} {}


template<typename T>
ResourceRef<T>::ResourceRef(ObserverPtr<T> res) noexcept requires std::derived_from<T, Resource> :
  id_{res ? res->GetResId() : ResourceId::Invalid()},
  cached_{res} {}


template<typename T>
ResourceRef<T>::ResourceRef(ResourceId const& res_id) noexcept requires std::derived_from<T, Resource> :
  id_{res_id} {}


template<typename T>
template<typename U> requires
  std::derived_from<std::remove_cv_t<U>, Resource> &&
  std::derived_from<std::remove_cv_t<T>, Resource> &&
  std::convertible_to<U*, T*>
ResourceRef<T>::ResourceRef(ResourceRef<U> const& other) noexcept :
  id_{other.id_},
  cached_{other.cached_} {}


template<typename T>
auto ResourceRef<T>::Get() const -> ObjectPtr<T> requires std::derived_from<T, Resource> {
  // We want to support ResourceRefs that hold pointers to resources not-yet saved or
  // registered in the ResourceManager. Those might not have valid IDs yet. So we can
  // only avoid reading the cache if we know FOR SURE that it cannot possibly point to
  // an object. If the cache does not track any Object identity, it won't be able to 
  // resolve to anything. If our resource ID is also invalid, we will not able to update
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
auto ResourceRef<T>::Observe() const -> ObserverPtr<T> requires std::derived_from<T, Resource> {
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
auto ResourceRef<T>::operator->() const -> ObjectPtr<T> requires std::derived_from<T, Resource> {
  return Get();
}


template<typename T>
auto ResourceRef<T>::operator*() const -> T& requires std::derived_from<T, Resource> {
  return *Get();
}


template<typename T>
ResourceRef<T>::operator bool() const {
  return Observe() != nullptr;
}


template<typename T>
auto ResourceRef<T>::UpdateCache() const -> void {
  cached_ = ReflCast<T>(detail::ResolveResource(id_));
}


template<std::derived_from<Resource> T>
auto MakeResourceRef(ObserverPtr<T> const resource) noexcept -> ResourceRef<T> {
  return ResourceRef{resource};
}


template<std::derived_from<Resource> To, std::derived_from<Resource> From>
auto ReflCast(ResourceRef<From> res) noexcept -> ResourceRef<To> {
  return MakeResourceRef(ReflCast<To>(res.Get()));
}
}


template<std::derived_from<sorcery::Resource> T>
auto rttr::wrapper_mapper<sorcery::ResourceRef<T>>::get(type const& res_ref) -> wrapped_type {
  return res_ref.Get().Get().Get();
}


template<std::derived_from<sorcery::Resource> T>
auto rttr::wrapper_mapper<sorcery::ResourceRef<T>>::create(wrapped_type const& ptr) -> type {
  return sorcery::MakeResourceRef(sorcery::MakeObserver(ptr));
}


template<std::derived_from<sorcery::Resource> T>
template<typename U>
auto rttr::wrapper_mapper<sorcery::ResourceRef<T>>::convert(type const& res_ref, bool& ok) -> sorcery::ResourceRef<U> {
  auto* const res = rttr_cast<typename sorcery::ResourceRef<U>::wrapped_type>(get(res_ref));
  ok = res != nullptr;
  return wrapper_mapper<sorcery::ResourceRef<U>>::create(res);
}
