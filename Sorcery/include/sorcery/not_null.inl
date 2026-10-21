#pragma once

#include <exception>


namespace sorcery {
template<NotNullable T>
template<std::convertible_to<T> U>
  requires (!detail::kIsNotNull<std::remove_cvref_t<U>>)
constexpr NotNull<T>::NotNull(U&& u) noexcept(std::is_nothrow_constructible_v<T, U>) :
  ptr_{std::forward<U>(u)} {
  if (ptr_ == nullptr) {
    std::terminate();
  }
}


template<NotNullable T>
template<class U>
  requires std::convertible_to<U const&, T>
constexpr NotNull<T>::NotNull(NotNull<U> const& other) noexcept(std::is_nothrow_constructible_v<T, U const&>) :
  ptr_{other.get()} {}


template<NotNullable T>
template<class U>
  requires (!std::same_as<U, T> && std::convertible_to<U&&, T>)
constexpr NotNull<T>::NotNull(NotNull<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) :
  ptr_{std::move(other.ptr_)} {}


template<NotNullable T>
constexpr auto NotNull<T>::Get() const & noexcept -> T const& {
  return ptr_;
}


template<NotNullable T>
constexpr auto NotNull<T>::Get() && noexcept(std::is_nothrow_move_constructible_v<T>) -> T {
  return std::move(ptr_);
}


template<NotNullable T>
constexpr NotNull<T>::operator T const&() const noexcept {
  return ptr_;
}


template<NotNullable T>
constexpr auto NotNull<T>::operator->() const noexcept -> T const& {
  return ptr_;
}


template<NotNullable T>
constexpr auto NotNull<T>::operator*() const noexcept(noexcept(*std::declval<T const&>()))
  -> decltype(*std::declval<T const&>()) {
  return *ptr_;
}


template<class T, class U>
  requires detail::EqualityComparableWith<T, U>
constexpr auto operator==(NotNull<T> const& lhs, NotNull<U> const& rhs) noexcept(detail::kIsNothrowEq<T, U>) -> bool {
  return lhs.get() == rhs.get();
}


template<class T, class U>
  requires std::three_way_comparable_with<T, U>
constexpr auto operator<=>(NotNull<T> const& lhs, NotNull<U> const& rhs) noexcept(detail::kIsNothrowThreeWay<T, U>) {
  return std::compare_three_way{}(lhs.get(), rhs.get());
}


template<class T, detail::PlainComparand U>
  requires detail::EqualityComparableWith<T, U>
constexpr auto operator==(NotNull<T> const& lhs, U const& rhs) noexcept(detail::kIsNothrowEq<T, U>) -> bool {
  return lhs.get() == rhs;
}


template<class T, detail::PlainComparand U>
  requires std::three_way_comparable_with<T, U>
constexpr auto operator<=>(NotNull<T> const& lhs, U const& rhs) noexcept(detail::kIsNothrowThreeWay<T, U>) {
  return std::compare_three_way{}(lhs.get(), rhs);
}
}


template<class T>
auto std::hash<sorcery::NotNull<T>>::operator()(sorcery::NotNull<T> const& p) const noexcept -> std::size_t {
  return std::hash<T>{}(p.get());
}
