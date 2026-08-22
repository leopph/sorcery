#pragma once

namespace sorcery {
template<std::derived_from<Resource> T>
ResourceRef<T>::ResourceRef([[maybe_unused]] nullptr_t const null) noexcept :
  id_{ResourceId::Invalid()} {}


template<std::derived_from<Resource> T>
ResourceRef<T>::ResourceRef(ObserverPtr<T> res) noexcept :
  id_{res ? res->GetResId() : ResourceId::Invalid()},
  cached_{res} {}


template<std::derived_from<Resource> T>
ResourceRef<T>::ResourceRef(ResourceId const& res_id) noexcept :
  id_{res_id} {}


template<std::derived_from<Resource> T>
auto ResourceRef<T>::Get() const -> ObjectPtr<T> {
  if (cached_) {
    return cached_;
  }

  if (auto const res = detail::ResolveResource(id_)) {
    cached_ = MakeObserver(rttr::rttr_cast<T*>(res.Get()));
  } else {
    cached_ = nullptr;
  }

  return cached_;
}


template<std::derived_from<Resource> T>
auto ResourceRef<T>::operator->() const -> ObjectPtr<T> {
  return Get();
}


template<std::derived_from<Resource> T>
auto ResourceRef<T>::operator*() const -> T& {
  return *Get();
}


template<std::derived_from<Resource> T>
ResourceRef<T>::operator bool() const {
  return Get().Get() != nullptr;
}


template<std::derived_from<Resource> T>
auto MakeResourceRef(ObserverPtr<T> const resource) noexcept -> ResourceRef<T> {
  return ResourceRef{resource};
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
