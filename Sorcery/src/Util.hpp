#pragma once

#include "Core.hpp"

#include <concepts>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "Math.hpp"


namespace sorcery {
template<std::integral To, std::integral From>
[[nodiscard]] constexpr auto clamp_cast(From what) -> To;

template<auto& Obj, auto MemberFunc, typename... Args>
auto Call(Args&&... args) noexcept(std::is_nothrow_invocable_v<decltype(MemberFunc), decltype(Obj), Args...>);

template<class T> concept Scalar = std::is_scalar_v<T>;

[[nodiscard]] constexpr auto RoundToNextMultiple(auto what, auto multipleOf);

[[nodiscard]] LEOPPHAPI auto Contains(std::string_view src, std::string_view target) -> bool;

[[nodiscard]] SORCERYAPI
auto Trim(std::string_view sv) -> std::string_view;

LEOPPHAPI auto CalculateNormals(std::span<Vector3 const> positions, std::span<unsigned const> indices,
                                std::vector<Vector3>& out) -> std::vector<Vector3>&;
LEOPPHAPI auto CalculateTangents(std::span<Vector3 const> positions, std::span<Vector2 const> uvs,
                                 std::span<unsigned const> indices, std::vector<Vector3>& out) -> void;

// Appends an index to the specified file path to avoid name clashes
[[nodiscard]] LEOPPHAPI auto GenerateUniquePath(std::filesystem::path const& absolutePath) -> std::filesystem::path;

[[nodiscard]] LEOPPHAPI auto Join(std::span<std::string const> strings, std::string const& delim) -> std::string;

[[nodiscard]] LEOPPHAPI auto ToLower(std::string_view str) -> std::string;

[[nodiscard]] LEOPPHAPI auto IsSubpath(std::filesystem::path const& path, std::filesystem::path const& base) -> bool;

template<typename To, typename From>
[[nodiscard]] auto static_unique_ptr_cast(std::unique_ptr<From> ptr) -> std::unique_ptr<To>;

template<std::unsigned_integral T>
[[nodiscard]] constexpr auto SatSub(T lhs, T rhs) -> T;

template<std::unsigned_integral T>
[[nodiscard]] constexpr auto DivRoundUp(T lhs, T rhs) -> T;

template<std::unsigned_integral T>
[[nodiscard]] constexpr auto AlignsTo(T value, T alignment) -> bool;

template<std::integral T>
[[nodiscard]] constexpr auto Mod(T dividend, T modulus) -> T;

// Returns an untyped string_view with the same content as the given u8string_view.
// Does NOT perform any encoding conversions or checks.
// Consider this a "static_cast".
[[nodiscard]] LEOPPHAPI auto ToUntypedStdSv(std::u8string_view sv) -> std::string_view;
// Returns an UTF8-typed string_view with the same content as the given string_view.
// Does NOT perform any encoding conversions or checks.
// Consider this a "static_cast".
[[nodiscard]] LEOPPHAPI auto ToUtf8StdSv(std::string_view sv) -> std::u8string_view;


template<class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
  consteval auto operator()(auto) const -> void;
};
}


#include "util.inl"
