#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


namespace sorcery {
template<class T>
inline constexpr auto kEnableNotNull = false;

template<class T, class Deleter>
inline constexpr auto kEnableNotNull<std::unique_ptr<T, Deleter>> = true;

template<class T>
inline constexpr auto kEnableNotNull<std::shared_ptr<T>> = true;


template<class T>
concept NotNullable = kEnableNotNull<T> && requires(T const& t) {
  { t == nullptr } -> std::convertible_to<bool>;
};


template<NotNullable T>
class NotNull;


namespace detail {
template<class T>
inline constexpr auto kIsNotNull = false;

template<class T>
inline constexpr auto kIsNotNull<NotNull<T>> = true;
}


// Wraps a smart pointer (unique_ptr, shared_ptr, or any type opted in through kEnableNotNull) and guarantees it is never null.
// Constructing from a null value calls std::terminate (or fails to compile during constant evaluation).
// Constructing from a nullptr literal is a compile error.
//
// Note: a moved-from NotNull holding a move-only type (e.g. unique_ptr) is
// null, just like the moved-from pointer itself. Don't use it after the move.
template<NotNullable T>
class NotNull {
public:
  template<std::convertible_to<T> U>
    requires (!detail::kIsNotNull<std::remove_cvref_t<U>>)
  constexpr NotNull(U&& u) noexcept(std::is_nothrow_constructible_v<T, U>);

  template<class U>
    requires std::convertible_to<U const&, T>
  constexpr NotNull(NotNull<U> const& other) noexcept(std::is_nothrow_constructible_v<T, U const&>);

  template<class U>
    requires (!std::same_as<U, T> && std::convertible_to<U&&, T>)
  constexpr NotNull(NotNull<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>);

  NotNull(std::nullptr_t) = delete;

  NotNull(NotNull const& other) = default;
  NotNull(NotNull&& other) = default;

  ~NotNull() = default;

  auto operator=(NotNull const& other) -> NotNull& = default;
  auto operator=(NotNull&& other) -> NotNull& = default;
  auto operator=(std::nullptr_t) -> NotNull& = delete;

  [[nodiscard]] constexpr auto Get() const & noexcept -> T const&;
  // Moves the wrapped pointer out, e.g. to hand a unique_ptr's ownership to a
  // function taking unique_ptr by value. The NotNull is null afterwards.
  [[nodiscard]] constexpr auto Get() && noexcept(std::is_nothrow_move_constructible_v<T>) -> T;
  constexpr operator T const&() const noexcept;

  constexpr auto operator->() const noexcept -> T const&;
  constexpr auto operator*() const noexcept(noexcept(*std::declval<T const&>()))
    -> decltype(*std::declval<T const&>());

  // No pointer arithmetic.
  auto operator++() -> NotNull& = delete;
  auto operator--() -> NotNull& = delete;
  auto operator++(int) -> NotNull = delete;
  auto operator--(int) -> NotNull = delete;
  auto operator+=(std::ptrdiff_t) -> NotNull& = delete;
  auto operator-=(std::ptrdiff_t) -> NotNull& = delete;
  auto operator[](std::ptrdiff_t) const -> void = delete;

private:
  template<NotNullable U>
  friend class NotNull;

  T ptr_;
};


template<class T>
NotNull(T) -> NotNull<T>;


namespace detail {
template<class T, class U>
concept EqualityComparableWith = requires(T const& t, U const& u) {
  { t == u } -> std::convertible_to<bool>;
};

template<class T, class U>
inline constexpr auto kIsNothrowEq{noexcept(std::declval<T const&>() == std::declval<U const&>())};

template<class T, class U>
inline constexpr auto kIsNothrowThreeWay{
  noexcept(std::compare_three_way{}(std::declval<T const&>(), std::declval<U const&>()))
};

template<class U>
concept PlainComparand = !kIsNotNull<U> && !std::same_as<U, std::nullptr_t>;
}


template<class T, class U>
  requires detail::EqualityComparableWith<T, U>
[[nodiscard]] constexpr
auto operator==(NotNull<T> const& lhs, NotNull<U> const& rhs) noexcept(detail::kIsNothrowEq<T, U>) -> bool;

template<class T, class U>
  requires std::three_way_comparable_with<T, U>
[[nodiscard]] constexpr
auto operator<=>(NotNull<T> const& lhs, NotNull<U> const& rhs) noexcept(detail::kIsNothrowThreeWay<T, U>);

template<class T, detail::PlainComparand U>
  requires detail::EqualityComparableWith<T, U>
[[nodiscard]] constexpr
auto operator==(NotNull<T> const& lhs, U const& rhs) noexcept(detail::kIsNothrowEq<T, U>) -> bool;

template<class T, detail::PlainComparand U>
  requires std::three_way_comparable_with<T, U>
[[nodiscard]] constexpr
auto operator<=>(NotNull<T> const& lhs, U const& rhs) noexcept(detail::kIsNothrowThreeWay<T, U>);


template<class T>
auto operator+(NotNull<T> const&, std::ptrdiff_t) -> NotNull<T> = delete;

template<class T>
auto operator+(std::ptrdiff_t, NotNull<T> const&) -> NotNull<T> = delete;

template<class T>
auto operator-(NotNull<T> const&, std::ptrdiff_t) -> NotNull<T> = delete;

template<class T, class U>
auto operator-(NotNull<T> const&, NotNull<U> const&) -> std::ptrdiff_t = delete;
}


template<class T>
struct std::hash<sorcery::NotNull<T>> {
  [[nodiscard]] auto operator()(sorcery::NotNull<T> const& p) const noexcept -> std::size_t;
};


#include <sorcery/not_null.inl>
