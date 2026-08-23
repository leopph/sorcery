#pragma once


namespace sorcery {
template<typename T>
ObjectPtr<T>::ObjectPtr([[maybe_unused]] nullptr_t null) noexcept :
  id_{} {}


template<typename T>
ObjectPtr<T>::ObjectPtr(ObserverPtr<T> const obj) noexcept requires std::derived_from<T, Object> :
  id_{obj ? obj->GetId() : ObjectId{}} {}


template<typename T>
template<typename U> requires
  std::derived_from<std::remove_cv_t<U>, Object> &&
  std::derived_from<std::remove_cv_t<T>, Object> &&
  std::convertible_to<U*, T*>
ObjectPtr<T>::ObjectPtr(ObjectPtr<U> const& other) noexcept :
  id_{other.id_} {}


template<typename T>
auto ObjectPtr<T>::Get() const -> ObserverPtr<T> requires std::derived_from<T, Object> {
  return ObserverPtr{static_cast<T*>(detail::ResolveObject(id_).Get())};
}


template<typename T>
auto ObjectPtr<T>::operator->() const -> ObserverPtr<T> requires std::derived_from<T, Object> {
  return Get();
}


template<typename T>
auto ObjectPtr<T>::operator*() const -> T& requires std::derived_from<T, Object> {
  return *Get();
}


template<typename T>
ObjectPtr<T>::operator bool() const {
  return Get() != nullptr;
}


template<std::derived_from<Object> T>
auto MakeObjectPtr(ObserverPtr<T> const object) noexcept -> ObjectPtr<T> {
  return ObjectPtr{object};
}


template<std::derived_from<Object> To, std::derived_from<Object> From>
auto ReflCast(ObjectPtr<From> obj) noexcept -> ObjectPtr<To> {
  return MakeObjectPtr(ReflCast<To>(obj.Get()));
}
}


template<std::derived_from<sorcery::Object> T>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::get(type const& obj_ptr) -> wrapped_type {
  return obj_ptr.Get().Get();
}


template<std::derived_from<sorcery::Object> T>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::create(wrapped_type const& ptr) -> type {
  return sorcery::MakeObjectPtr(sorcery::MakeObserver(ptr));
}


template<std::derived_from<sorcery::Object> T>
template<typename U>
auto rttr::wrapper_mapper<sorcery::ObjectPtr<T>>::convert(type const& obj_ptr, bool& ok) -> sorcery::ObjectPtr<U> {
  auto* const obj = rttr_cast<typename sorcery::ObjectPtr<U>::wrapped_type>(get(obj_ptr));
  ok = obj != nullptr;
  return wrapper_mapper<sorcery::ObjectPtr<U>>::create(obj);
}
