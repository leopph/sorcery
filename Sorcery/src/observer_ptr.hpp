#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "Reflection.hpp"


namespace sorcery {
template<typename W>
class ObserverPtr {
public:
  using ElementType = W;

  constexpr ObserverPtr() noexcept;
  constexpr ObserverPtr(std::nullptr_t) noexcept;
  constexpr explicit ObserverPtr(ElementType* p) noexcept;
  template<typename W2>
  constexpr ObserverPtr(ObserverPtr<W2> other) noexcept;

  constexpr auto Release() noexcept -> ElementType*;
  constexpr auto Reset(ElementType* p = nullptr) noexcept -> void;
  constexpr auto Swap(ObserverPtr& other) noexcept;

  constexpr auto Get() const noexcept -> ElementType*;
  constexpr explicit operator bool() const noexcept;

  constexpr auto operator*() const -> std::add_lvalue_reference_t<ElementType>;
  constexpr auto operator->() const noexcept -> ElementType*;

  constexpr explicit operator ElementType*() const noexcept;

private:
  ElementType* ptr_;
};


template<typename W>
auto MakeObserver(W* p) noexcept -> ObserverPtr<W>;

template<typename W1, typename W2>
auto operator==(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
template<typename W1, typename W2>
auto operator!=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
template<typename W>
auto operator==(ObserverPtr<W> const& p, std::nullptr_t) noexcept -> bool;
template<typename W>
auto operator==(std::nullptr_t, ObserverPtr<W> const& p) noexcept -> bool;
template<class W>
auto operator!=(ObserverPtr<W> const& p, std::nullptr_t) noexcept -> bool;
template<class W>
auto operator!=(std::nullptr_t, ObserverPtr<W> const& p) noexcept -> bool;
template<class W1, class W2>
auto operator<(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
template<class W1, class W2>
auto operator>(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
template<class W1, class W2>
auto operator<=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
template<class W1, class W2>
auto operator>=(ObserverPtr<W1> const& p1, ObserverPtr<W2> const& p2) -> bool;
}


template<typename T>
struct std::hash<sorcery::ObserverPtr<T>> {
  auto operator()(sorcery::ObserverPtr<T> const& p) const -> size_t;
};


template<typename T>
struct rttr::wrapper_mapper<sorcery::ObserverPtr<T>> {
  using wrapped_type = T*;
  using type = sorcery::ObserverPtr<T>;

  static auto get(type const& obj_ptr) -> wrapped_type;

  static auto create(wrapped_type const& ptr) -> type;

  template<typename U>
  static auto convert(type const& obj_ptr, bool& ok) -> sorcery::ObserverPtr<U>;
};


#include "observer_ptr.inl"
