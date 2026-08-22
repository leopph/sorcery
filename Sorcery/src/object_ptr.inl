#pragma once

#include "object_registry.hpp"


namespace sorcery {
template<std::derived_from<Object> T>
ObjectPtr<T>::ObjectPtr([[maybe_unused]] nullptr_t null) noexcept :
  id_{} {}


template<std::derived_from<Object> T>
ObjectPtr<T>::ObjectPtr(ObserverPtr<T> const obj) noexcept :
  id_{obj ? obj->GetId() : ObjectId{}} {}


template<std::derived_from<Object> T>
auto ObjectPtr<T>::Get() const -> ObserverPtr<T> {
  return ObserverPtr{static_cast<T*>(ObjectRegistry::Instance().Resolve(id_).Get())};
}


template<std::derived_from<Object> T>
auto ObjectPtr<T>::operator->() const -> ObserverPtr<T> {
  return Get();
}


template<std::derived_from<Object> T>
auto ObjectPtr<T>::operator*() const -> T& {
  return *Get();
}


template<std::derived_from<Object> T>
ObjectPtr<T>::operator bool() const {
  return Get() != nullptr;
}


template<std::derived_from<Object> T>
auto MakeObjectPtr(ObserverPtr<T> const object) noexcept -> ObjectPtr<T> {
  return ObjectPtr{object};
}
}


template<std::derived_from<sorcery::Object> T>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::get(type const& obj_ptr) -> wrapped_type {
  return obj_ptr.Get().Get();
}


template<std::derived_from<sorcery::Object> T>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::create(wrapped_type const& ptr) -> type {
  return sorcery::ObjectPtr{sorcery::MakeObserver(ptr)};
}


template<std::derived_from<sorcery::Object> T>
template<typename U>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::convert(type const& obj_ptr, bool& ok) -> sorcery::ObjectPtr<U> {
  auto* const obj{rttr_cast<typename sorcery::ObjectPtr<U>::wrapped_type>(get(obj_ptr))};
  ok = obj != nullptr;
  return wrapper_mapper<sorcery::ObjectPtr<U>>::create(obj);
}
