#pragma once

namespace sorcery {
template<typename W>
constexpr ObserverPtr<W>::ObserverPtr() noexcept :
  ptr_{nullptr} {}


template<typename W>
constexpr ObserverPtr<W>::ObserverPtr(std::nullptr_t) noexcept :
  ObserverPtr{} {}


template<typename W>
constexpr ObserverPtr<W>::ObserverPtr(ElementType* const p) noexcept :
  ptr_{p} {}


template<typename W>
template<typename W2>
constexpr ObserverPtr<W>::ObserverPtr(ObserverPtr<W2> other) noexcept :
  ptr_{static_cast<ElementType*>(other.Get())} {}


template<typename W>
constexpr auto ObserverPtr<W>::Release() noexcept -> ElementType* {
  auto const ret{ptr_};
  ptr_ = nullptr;
  return ret;
}


template<typename W>
constexpr auto ObserverPtr<W>::Reset(ElementType* const p) noexcept -> void {
  ptr_ = p;
}


template<typename W>
constexpr auto ObserverPtr<W>::Swap(ObserverPtr& other) noexcept {
  std::swap(ptr_, other);
}


template<typename W>
constexpr auto ObserverPtr<W>::Get() const noexcept -> ElementType* {
  return ptr_;
}


template<typename W>
constexpr ObserverPtr<W>::operator bool() const noexcept {
  return Get() != nullptr;
}


template<typename W>
constexpr auto ObserverPtr<W>::operator*() const -> std::add_lvalue_reference_t<ElementType> {
  return *ptr_;
}


template<typename W>
constexpr auto ObserverPtr<W>::operator->() const noexcept -> ElementType* {
  return ptr_;
}


template<typename W>
constexpr ObserverPtr<W>::operator W*() const noexcept {
  return Get();
}


template<typename W>
auto MakeObserver(W* p) noexcept -> ObserverPtr<W> {
  return ObserverPtr{p};
}


template<typename W1, typename W2>
auto operator==(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  return p1.Get() == p2.Get();
}


template<typename W1, typename W2>
auto operator!=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  return !(p1 == p2);
}


template<typename W>
auto operator==(ObserverPtr<W> const& p, std::nullptr_t) noexcept -> bool {
  return !p;
}


template<typename W>
auto operator==(std::nullptr_t, ObserverPtr<W> const& p) noexcept -> bool {
  return !p;
}


template<class W>
auto operator!=(ObserverPtr<W> const& p, std::nullptr_t) noexcept -> bool {
  return static_cast<bool>(p);
}


template<class W>
auto operator!=(std::nullptr_t, ObserverPtr<W> const& p) noexcept -> bool {
  return static_cast<bool>(p);
}


template<class W1, class W2>
auto operator<(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  using W3 = std::common_type_t<W1*, W2*>;
  return std::less<W3>{}(static_cast<W3>(p1.Get()), static_cast<W3>(p2.Get()));
}


template<class W1, class W2>
auto operator>(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  return p2 < p1;
}


template<class W1, class W2>
auto operator<=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  return !(p2 < p1);
}


template<class W1, class W2>
auto operator>=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool {
  return !(p1 < p2);
}
}


template<typename T>
auto std::hash<sorcery::ObserverPtr<T>>::operator()(sorcery::ObserverPtr<T> const& p) const -> size_t {
  return std::hash<T*>{}(p.Get());
}


template<typename T>
auto rttr::wrapper_mapper<sorcery::ObserverPtr<T>>::get(type const& obs_ptr) -> wrapped_type {
  return obs_ptr.Get();
}


template<typename T>
auto rttr::wrapper_mapper<sorcery::ObserverPtr<T>>::create(wrapped_type const& ptr) -> type {
  return sorcery::MakeObserver(ptr);
}


template<typename T>
template<typename U>
auto rttr::wrapper_mapper<sorcery::ObserverPtr<T>>::convert(type const& obj_ptr, bool& ok) -> sorcery::ObserverPtr<U> {
  auto* const obj{rttr_cast<typename sorcery::ObserverPtr<U>::wrapped_type>(get(obj_ptr))};
  ok = obj != nullptr;
  return wrapper_mapper<sorcery::ObserverPtr<U>>::create(obj);
}
