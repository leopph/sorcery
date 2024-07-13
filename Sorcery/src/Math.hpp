#pragma once

#if defined(LEOPPH_MATH_NO_INTRINSICS) && defined(LEOPPH_MATH_USE_INTRINSICS)
#undef LEOPPH_MATH_USE_INTRINSICS
#endif

#if !defined(LEOPPH_MATH_USE_INTRINSINCS) && !defined(LEOPPH_MATH_NO_INTRINSICS) && defined(__AVX2__)
#define LEOPPH_MATH_USE_INTRINSICS
#endif

#include "Reflection.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <ostream>

#ifdef LEOPPH_MATH_USE_INTRINSICS
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#endif


namespace sorcery {
float constexpr PI{std::numbers::pi_v<float>};

[[nodiscard]] constexpr auto ToRadians(float degrees) noexcept -> float;
[[nodiscard]] constexpr auto ToDegrees(float radians) noexcept -> float;

[[nodiscard]] constexpr auto IsPowerOfTwo(unsigned value) noexcept -> bool;
[[nodiscard]] constexpr auto NextPowerOfTwo(unsigned value) noexcept -> unsigned;

[[nodiscard]] constexpr auto Lerp(float from, float to, float t) noexcept -> float;

template<typename T>
[[nodiscard]] constexpr auto Pow(T base, T exp) noexcept;


template<typename T, int N> requires(N > 1)
class Vector {
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] constexpr auto operator[](size_t index) const noexcept -> T const&;
  [[nodiscard]] constexpr auto operator[](size_t index) noexcept -> T&;
  [[nodiscard]] constexpr auto GetData() const noexcept -> T const*;
  [[nodiscard]] constexpr auto GetData() noexcept -> T*;

  [[nodiscard]] static constexpr auto Up() noexcept -> Vector;
  [[nodiscard]] static constexpr auto Down() noexcept -> Vector;
  [[nodiscard]] static constexpr auto Left() noexcept -> Vector;
  [[nodiscard]] static constexpr auto Right() noexcept -> Vector;
  [[nodiscard]] static constexpr auto Forward() noexcept -> Vector requires(N >= 3);
  [[nodiscard]] static constexpr auto Backward() noexcept -> Vector requires(N >= 3);

  constexpr Vector() noexcept = default;
  [[nodiscard]] static constexpr auto Zero() noexcept -> Vector;

  template<std::convertible_to<T> T1>
  constexpr explicit Vector(T1 value) noexcept;

  template<std::convertible_to<T> T1>
  [[nodiscard]] static constexpr auto Filled(T1 value) noexcept -> Vector;

  template<std::convertible_to<T>... Args> requires(sizeof...(Args) == N)
  constexpr explicit Vector(Args&&... args) noexcept;

  template<int M> requires (M > N)
  constexpr explicit Vector(Vector<T, M> const& other) noexcept;

  template<int M, std::convertible_to<T>... Args> requires (M < N && sizeof...(Args) == N - M)
  constexpr explicit Vector(Vector<T, M> const& other, Args&&... additionalElems) noexcept;

  constexpr Vector(Vector const& other) noexcept = default;
  constexpr Vector(Vector&& other) noexcept = default;

  constexpr ~Vector() noexcept = default;

  constexpr auto operator=(Vector const& other) noexcept -> Vector& = default;
  constexpr auto operator=(Vector&& other) noexcept -> Vector& = default;

private:
  std::array<T, N> mData{};
};


using Vector2 = Vector<float, 2>;
using Vector3 = Vector<float, 3>;
using Vector4 = Vector<float, 4>;


template<typename T, int N>
[[nodiscard]] auto Length(Vector<T, N> const& vector) noexcept -> T;

template<typename T, int N>
[[nodiscard]] auto Normalized(Vector<T, N> const& vector) noexcept -> Vector<T, N>;

template<typename T, int N>
auto Normalize(Vector<T, N>& vector) noexcept -> Vector<T, N>&;

template<typename T, int N>
auto Normalize(Vector<T, N>&& vector) noexcept -> Vector<T, N>&&;

template<typename T, int N> requires (N > 1)
[[nodiscard]] constexpr auto Dot(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T;

template<typename T>
[[nodiscard]] constexpr auto Cross(Vector<T, 3> const& left, Vector<T, 3> const& right) noexcept -> Vector<T, 3>;

template<typename T, int N> requires (N > 1)
[[nodiscard]] auto Distance(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T;

template<typename T, int N>
[[nodiscard]] constexpr auto Lerp(Vector<T, N> const& from, Vector<T, N> const& to, float t) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator-(Vector<T, N> const& operand) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator+(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
constexpr auto operator+=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&;

template<typename T, int N>
[[nodiscard]] constexpr auto operator-(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
constexpr auto operator-=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&;

template<typename T, int N>
[[nodiscard]] constexpr auto operator*(Vector<T, N> const& left,
                                       std::type_identity_t<T> right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator*(std::type_identity_t<T> left,
                                       Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator*(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, std::type_identity_t<T> right) noexcept -> Vector<T, N>&;

template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&;

template<typename T, int N>
[[nodiscard]] constexpr auto operator/(Vector<T, N> const& left,
                                       std::type_identity_t<T> right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator/(std::type_identity_t<T> left,
                                       Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] constexpr auto operator/(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
constexpr auto operator/=(Vector<T, N>& left, std::type_identity_t<T> right) noexcept -> Vector<T, N>&;

template<typename T, int N>
constexpr auto operator/=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&;

template<typename T, int N>
[[nodiscard]] constexpr auto operator==(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool;

template<typename T, int N>
[[nodiscard]] constexpr auto operator!=(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool;

template<typename T, int N>
auto operator<<(std::ostream& stream, Vector<T, N> const& vector) -> std::ostream&;

template<typename T, int N>
[[nodiscard]] auto Clamp(Vector<T, N> const& v, T min, T max) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto Clamp(Vector<T, N> const& v, Vector<T, N> const& min,
                         Vector<T, N> const& max) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto Min(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto Max(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto ScalarProject(Vector<T, N> const& what, Vector<T, N> const& onto) noexcept -> float;

template<typename T, int N>
[[nodiscard]] auto VectorProject(Vector<T, N> const& what, Vector<T, N> const& onto) noexcept -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto Floor(Vector<T, N> const& v) -> Vector<T, N>;

template<typename T, int N>
[[nodiscard]] auto Round(Vector<T, N> const& v) -> Vector<T, N>;

#ifdef LEOPPH_MATH_USE_INTRINSICS
template<>
[[nodiscard]] inline auto Length(Vector3 const& vector) noexcept -> float;
template<>
[[nodiscard]] inline auto Length(Vector4 const& vector) noexcept -> float;

template<>
[[nodiscard]] inline auto Dot(Vector3 const& left, Vector3 const& right) noexcept -> float;
template<>
[[nodiscard]] inline auto Dot(Vector4 const& left, Vector4 const& right) noexcept -> float;

template<>
[[nodiscard]] inline auto Cross(Vector3 const& left, Vector3 const& right) noexcept -> Vector3;

template<>
[[nodiscard]] inline auto operator+(Vector3 const& left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator+(Vector4 const& left, Vector4 const& right) noexcept -> Vector4;

template<>
inline auto operator+=(Vector3& left, Vector3 const& right) noexcept -> Vector3&;
template<>
inline auto operator+=(Vector4& left, Vector4 const& right) noexcept -> Vector4&;

template<>
[[nodiscard]] inline auto operator-(Vector3 const& left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator-(Vector4 const& left, Vector4 const& right) noexcept -> Vector4;

template<>
inline auto operator-=(Vector3& left, Vector3 const& right) noexcept -> Vector3&;
template<>
inline auto operator-=(Vector4& left, Vector4 const& right) noexcept -> Vector4&;

template<>
[[nodiscard]] inline auto operator*(Vector3 const& left, float right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator*(Vector4 const& left, float right) noexcept -> Vector4;

template<>
[[nodiscard]] inline auto operator*(float left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator*(float left, Vector4 const& right) noexcept -> Vector4;

template<>
[[nodiscard]] inline auto operator*(Vector3 const& left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator*(Vector4 const& left, Vector4 const& right) noexcept -> Vector4;

template<>
inline auto operator*=(Vector3& left, Vector3 const& right) noexcept -> Vector3&;
template<>
inline auto operator*=(Vector4& left, Vector4 const& right) noexcept -> Vector4&;

template<>
inline auto operator*=(Vector3& left, float right) noexcept -> Vector3&;
template<>
inline auto operator*=(Vector4& left, float right) noexcept -> Vector4&;

template<>
[[nodiscard]] inline auto operator/(Vector3 const& left, float right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator/(float left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator/(Vector3 const& left, Vector3 const& right) noexcept -> Vector3;
template<>
[[nodiscard]] inline auto operator/(Vector4 const& left, float right) noexcept -> Vector4;
template<>
[[nodiscard]] inline auto operator/(float left, Vector4 const& right) noexcept -> Vector4;
template<>
[[nodiscard]] inline auto operator/(Vector4 const& left, Vector4 const& right) noexcept -> Vector4;

template<>
inline auto operator/=(Vector3& left, float right) noexcept -> Vector3&;
template<>
inline auto operator/=(Vector3& left, Vector3 const& right) noexcept -> Vector3&;
template<>
inline auto operator/=(Vector4& left, float right) noexcept -> Vector4&;
template<>
inline auto operator/=(Vector4& left, Vector4 const& right) noexcept -> Vector4&;
#endif


template<typename T, int N, int M> requires(N > 1 && M > 1)
class Matrix {
public:
  [[nodiscard]] constexpr auto Determinant() const noexcept -> T requires(N == M);
  [[nodiscard]] constexpr auto Transpose() const noexcept -> Matrix<T, M, N>;
  [[nodiscard]] constexpr auto Inverse() const noexcept -> Matrix requires(N == M);
  [[nodiscard]] constexpr auto Trace() const noexcept -> float requires(N == M);

  [[nodiscard]] constexpr auto operator[](size_t index) const noexcept -> Vector<T, M> const&;
  [[nodiscard]] constexpr auto operator[](size_t index) noexcept -> Vector<T, M>&;
  [[nodiscard]] constexpr auto GetData() noexcept -> T*;
  [[nodiscard]] constexpr auto GetData() const noexcept -> T const*;

  [[nodiscard]] static constexpr auto Identity() noexcept -> Matrix requires (N == M);

  [[nodiscard]] static constexpr auto Translate(Vector<T, 3> const& vector) noexcept -> Matrix<T, 4, 4> requires (
    N == 4 && M == 4);
  [[nodiscard]] static constexpr auto Scale(Vector<T, 3> const& vector) noexcept -> Matrix<T, 4, 4> requires (
    N == 4 && M == 4);

  [[nodiscard]] static constexpr auto LookTo(Vector<T, 3> const& position, Vector<T, 3> const& direction,
                                             Vector<T, 3> const& worldUp) noexcept -> Matrix<T, 4, 4> requires(
    N == 4 && M == 4);
  [[nodiscard]] static constexpr auto LookAt(Vector<T, 3> const& position, Vector<T, 3> const& target,
                                             Vector<T, 3> const& worldUp) noexcept -> Matrix<T, 4, 4> requires(
    N == 4 && M == 4);

  [[nodiscard]] static constexpr auto OrthographicOffCenter(T left, T right, T top, T bottom, T zNear,
                                                            T zFar) noexcept -> Matrix<T, 4, 4> requires (
    N == 4 && M == 4);
  [[nodiscard]] static constexpr auto Orthographic(T width, T height, T zNear, T zFar) noexcept -> Matrix<T, 4, 4>
    requires (N == 4 && M == 4);

  [[nodiscard]] static constexpr auto PerspectiveOffCenter(T left, T right, T top, T bottom, T zNear,
                                                           T zFar) noexcept -> Matrix<T, 4, 4> requires (
    N == 4 && M == 4);
  [[nodiscard]] static constexpr auto Perspective(T width, T height, T zNear, T zFar) noexcept -> Matrix<T, 4, 4>
    requires (N == 4 && M == 4);
  [[nodiscard]] static auto PerspectiveFov(T fovVertRad, T aspectRatio, T zNear, T zFar) noexcept -> Matrix<T, 4, 4>
    requires (N == 4 && M == 4);

  constexpr Matrix() noexcept = default;
  [[nodiscard]] static constexpr auto Zero() noexcept -> Matrix;

  constexpr explicit Matrix(T value) noexcept;
  [[nodiscard]] static constexpr auto Diagonal(T value) noexcept -> Matrix;

  template<std::convertible_to<T>... Args> requires(sizeof...(Args) == std::min(N, M))
  constexpr explicit Matrix(Args&&... args) noexcept;

  template<std::convertible_to<T>... Args> requires(sizeof...(Args) == std::min(N, M))
  [[nodiscard]] static constexpr auto Diagonal(Args&&... args) noexcept -> Matrix;

  template<int K> requires(K == std::min(N, M))
  constexpr explicit Matrix(Vector<T, K> const& vec) noexcept;

  template<int K> requires(K == std::min(N, M))
  [[nodiscard]] static constexpr auto Diagonal(Vector<T, K> const& vec) noexcept -> Matrix;

  template<std::convertible_to<T>... Args> requires(sizeof...(Args) == static_cast<unsigned long long>(N * M))
  constexpr explicit Matrix(Args&&... args) noexcept;

  template<int N1, int M1> requires (N1 < N && M1 < M)
  constexpr explicit Matrix(Matrix<T, N1, M1> const& other) noexcept;

  template<int N1, int M1> requires (N1 > N && M1 > M)
  constexpr explicit Matrix(Matrix<T, N1, M1> const& other) noexcept;

  constexpr Matrix(Matrix const& other) noexcept = default;
  constexpr Matrix(Matrix&& other) noexcept = default;

  constexpr ~Matrix() noexcept = default;

  constexpr auto operator=(Matrix const& other) noexcept -> Matrix& = default;
  constexpr auto operator=(Matrix&& other) noexcept -> Matrix& = default;

private:
  Vector<T, M> mData[N]{};
};


using Matrix2 = Matrix<float, 2, 2>;
using Matrix3 = Matrix<float, 3, 3>;
using Matrix4 = Matrix<float, 4, 4>;


template<typename T, int N, int M>
[[nodiscard]] constexpr auto operator+(Matrix<T, N, M> const& left,
                                       Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>;

template<typename T, int N, int M>
constexpr auto operator+=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>&;

template<typename T, int N, int M>
[[nodiscard]] constexpr auto operator-(Matrix<T, N, M> const& left,
                                       Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>;

template<typename T, int N, int M>
constexpr auto operator-=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>&;

template<typename T, int N, int M, std::convertible_to<T> T1>
[[nodiscard]] constexpr auto operator*(Matrix<T, N, M> const& left, T1 const& right) noexcept -> Matrix<T, N, M>;

template<typename T, int N, int M, std::convertible_to<T> T1>
[[nodiscard]] constexpr auto operator*(T1 const& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>;

template<typename T, int N, int M>
[[nodiscard]] constexpr auto operator*(Matrix<T, N, M> const& left, Vector<T, M> const& right) noexcept -> Vector<T, N>;

template<typename T, int N, int M>
[[nodiscard]] constexpr auto operator*(Vector<T, N> const& left, Matrix<T, N, M> const& right) noexcept -> Vector<T, M>;

template<typename T, int N, int M, int P>
[[nodiscard]] constexpr auto operator*(Matrix<T, N, M> const& left,
                                       Matrix<T, M, P> const& right) noexcept -> Matrix<T, N, P>;

#ifdef LEOPPH_MATH_USE_INTRINSICS
template<>
[[nodiscard]] inline auto operator*(Matrix4 const& left, Matrix4 const& right) noexcept -> Matrix4;
#endif

template<typename T, int N, int M, std::convertible_to<T> T1>
constexpr auto operator*=(Matrix<T, N, M>& left, T1 const& right) noexcept -> Matrix<T, N, M>&;

template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, Matrix<T, N, N> const& right) noexcept -> Vector<T, N>&;

template<typename T, int N, int M>
constexpr auto operator*=(Matrix<T, N, M>& left, Matrix<T, M, M> const& right) noexcept -> Matrix<T, N, M>&;

template<typename T, int N, int M>
auto operator<<(std::ostream& stream, Matrix<T, N, M> const& matrix) -> std::ostream&;


struct Quaternion {
  float x;
  float y;
  float z;
  float w;


  constexpr explicit Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f) noexcept;
  inline Quaternion(Vector3 const& axis, float angleDegrees) noexcept;

  [[nodiscard]] inline static auto FromEulerAngles(float x, float y, float z) noexcept -> Quaternion;
  [[nodiscard]] inline static auto FromEulerAngles(Vector3 const& euler) noexcept -> Quaternion;
  [[nodiscard]] inline auto ToEulerAngles() const noexcept -> Vector3;

  [[nodiscard]] constexpr auto ToRotationMatrix() const noexcept -> Matrix4;

  [[nodiscard]] inline static auto FromAxisAngle(Vector3 const& axis, float angleDeg) noexcept -> Quaternion;
  inline auto ToAxisAngle(Vector3& axis, float& angle) const noexcept -> void;

  [[nodiscard]] inline static auto FromTo(Vector3 const& from, Vector3 const& to) noexcept -> Quaternion;

  [[nodiscard]] constexpr auto NormSquared() const noexcept -> float;
  [[nodiscard]] inline auto Norm() const noexcept -> float;

  [[nodiscard]] inline auto Normalized() const noexcept -> Quaternion;
  inline auto Normalize() noexcept -> Quaternion&;

  [[nodiscard]] constexpr auto Conjugate() const noexcept -> Quaternion;
  constexpr auto ConjugateInPlace() noexcept -> Quaternion&;

  [[nodiscard]] constexpr auto Inverse() const noexcept -> Quaternion;
  constexpr auto Invert() noexcept -> Quaternion&;

  [[nodiscard]] constexpr explicit operator Matrix4() const noexcept;

  template<typename T>
  [[nodiscard]] constexpr auto Rotate(Vector<T, 3> const& vec) const noexcept -> Vector<T, 3>;
};


[[nodiscard]] constexpr auto operator+(Quaternion const& left, Quaternion const& right) -> Quaternion;

constexpr auto operator+=(Quaternion& left, Quaternion const& right) -> Quaternion&;

[[nodiscard]] constexpr auto operator*(Quaternion const& left, Quaternion const& right) noexcept -> Quaternion;
[[nodiscard]] constexpr auto operator*(Quaternion const& left, float right) -> Quaternion;
[[nodiscard]] constexpr auto operator*(float left, Quaternion const& right) -> Quaternion;

constexpr auto operator*=(Quaternion& left, Quaternion const& right) noexcept -> Quaternion&;
constexpr auto operator*=(Quaternion& left, float right) -> Quaternion&;

inline auto operator<<(std::ostream& os, Quaternion const& q) -> std::ostream&;

// Assumes unit quaternions
[[nodiscard]] inline auto Slerp(Quaternion const& from, Quaternion const& to, float amount) -> Quaternion;

/* ############################################################################################
 * ######## IMPLEMENTATION PART ###############################################################
 * ############################################################################################ */

constexpr auto ToRadians(float const degrees) noexcept -> float {
  return degrees * PI / 180.0f;
}


constexpr auto ToDegrees(float const radians) noexcept -> float {
  return radians * 180.0f / PI;
}


constexpr auto IsPowerOfTwo(unsigned const value) noexcept -> bool {
  return value != 0 && (value & value - 1) == 0;
}


constexpr auto NextPowerOfTwo(unsigned const value) noexcept -> unsigned {
  unsigned ret{1};

  while (ret <= value) {
    ret <<= 1;
  }

  return ret;
}


constexpr auto Lerp(float const from, float const to, float const t) noexcept -> float {
  return (1 - t) * from + t * to;
}


template<typename T>
constexpr auto Pow(T const base, T const exp) noexcept {
  T ret{1};

  for (auto i = 0; i < exp; i++) {
    ret *= base;
  }

  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::operator[](size_t index) const noexcept -> T const& {
  return mData[index];
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::operator[](size_t const index) noexcept -> T& {
  return mData[index];
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::GetData() const noexcept -> T const* {
  return mData.data();
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::GetData() noexcept -> T* {
  return mData.data();
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Up() noexcept -> Vector {
  Vector<T, N> ret{};
  ret[1] = 1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Down() noexcept -> Vector {
  Vector<T, N> ret{};
  ret[1] = -1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Left() noexcept -> Vector {
  Vector<T, N> ret{};
  ret[0] = -1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Right() noexcept -> Vector {
  Vector<T, N> ret{};
  ret[0] = 1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Forward() noexcept -> Vector requires (N >= 3) {
  Vector<T, N> ret{};
  ret[2] = 1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Backward() noexcept -> Vector requires (N >= 3) {
  Vector<T, N> ret{};
  ret[2] = -1;
  return ret;
}


template<typename T, int N> requires (N > 1)
constexpr auto Vector<T, N>::Zero() noexcept -> Vector {
  return Vector{};
}


template<typename T, int N> requires (N > 1)
template<std::convertible_to<T> T1>
constexpr Vector<T, N>::Vector(T1 const value) noexcept {
  for (auto i = 0; i < N; i++) {
    mData[i] = static_cast<T>(value);
  }
}


template<typename T, int N> requires (N > 1)
template<std::convertible_to<T> T1>
constexpr auto Vector<T, N>::Filled(T1 const value) noexcept -> Vector {
  return Vector{static_cast<T>(value)};
}


template<typename T, int N> requires (N > 1)
template<std::convertible_to<T>... Args> requires (sizeof...(Args) == N)
constexpr Vector<T, N>::Vector(Args&&... args) noexcept :
  mData{static_cast<T>(args)...} {}


template<typename T, int N> requires (N > 1)
template<int M> requires (M > N)
constexpr Vector<T, N>::Vector(Vector<T, M> const& other) noexcept {
  for (auto i = 0; i < N; i++) {
    mData[i] = other[i];
  }
}


template<typename T, int N> requires (N > 1)
template<int M, std::convertible_to<T>... Args> requires (M < N && sizeof...(Args) == N - M)
constexpr Vector<T, N>::Vector(Vector<T, M> const& other, Args&&... additionalElems) noexcept {
  for (auto i = 0; i < M; i++) {
    mData[i] = other[i];
  }

  [this, &additionalElems...]<int... Is>(std::index_sequence<Is...>) noexcept {
    (static_cast<void>(mData[M + Is] = static_cast<T>(additionalElems)), ...);
  }(std::index_sequence_for<Args...>{});
}


template<typename T, int N>
auto Length(Vector<T, N> const& vector) noexcept -> T {
  return std::sqrt(Dot(vector, vector));
}


template<typename T, int N>
auto Normalized(Vector<T, N> const& vector) noexcept -> Vector<T, N> {
  Vector<T, N> copy{vector};
  Normalize(copy);
  return copy;
}


template<typename T, int N>
auto Normalize(Vector<T, N>& vector) noexcept -> Vector<T, N>& {
  auto const length{Length(vector)};

  if constexpr (std::is_floating_point_v<T>) {
    if (length < std::numeric_limits<T>::epsilon()) {
      return vector;
    }
  } else {
    if (length == 0) {
      return vector;
    }
  }

  for (auto i = 0; i < N; i++) {
    vector[i] /= length;
  }

  return vector;
}


template<typename T, int N>
auto Normalize(Vector<T, N>&& vector) noexcept -> Vector<T, N>&& {
  return std::move(Normalize(vector));
}


template<typename T, int N> requires (N > 1)
constexpr auto Dot(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T {
  T ret{};

  for (size_t i = 0; i < N; i++) {
    ret += left[i] * right[i];
  }

  return ret;
}


template<typename T>
constexpr auto Cross(Vector<T, 3> const& left, Vector<T, 3> const& right) noexcept -> Vector<T, 3> {
  return Vector<T, 3>
  {
    left[1] * right[2] - left[2] * right[1],
    left[2] * right[0] - left[0] * right[2],
    left[0] * right[1] - left[1] * right[0]
  };
}


template<typename T, int N> requires (N > 1)
auto Distance(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T {
  T sum{0};

  for (size_t i = 0; i < N; i++) {
    sum += std::pow(left[i] - right[i], static_cast<T>(2));
  }

  return std::sqrt(sum);
}


template<typename T, int N>
constexpr auto Lerp(Vector<T, N> const& from, Vector<T, N> const& to, float const t) noexcept -> Vector<T, N> {
  return (1 - t) * from + t * to;
}


template<typename T, int N>
constexpr auto operator-(Vector<T, N> const& operand) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (size_t i = 0; i < N; i++) {
    ret[i] = -operand[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator+(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (size_t i = 0; i < N; i++) {
    ret[i] = left[i] + right[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator+=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] += right[i];
  }
  return left;
}


template<typename T, int N>
constexpr auto operator-(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] - right[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator-=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] -= right[i];
  }
  return left;
}


template<typename T, int N>
constexpr auto operator*(Vector<T, N> const& left, std::type_identity_t<T> const right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (size_t i = 0; i < N; i++) {
    ret[i] = left[i] * right;
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator*(std::type_identity_t<T> const left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  return right * left;
}


template<typename T, int N>
constexpr auto operator*(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] * right[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, std::type_identity_t<T> const right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] *= right;
  }
  return left;
}


template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] *= right[i];
  }
  return left;
}


template<typename T, int N>
constexpr auto operator/(Vector<T, N> const& left, std::type_identity_t<T> const right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] / right;
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator/(std::type_identity_t<T> const left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left / right[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator/(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] / right[i];
  }
  return ret;
}


template<typename T, int N>
constexpr auto operator/=(Vector<T, N>& left, std::type_identity_t<T> const right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] /= right;
  }
  return left;
}


template<typename T, int N>
constexpr auto operator/=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>& {
  for (auto i = 0; i < N; i++) {
    left[i] /= right[i];
  }
  return left;
}


template<typename T, int N>
constexpr auto operator==(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool {
  for (size_t i = 0; i < N; i++) {
    if (left[i] != right[i]) {
      return false;
    }
  }
  return true;
}


template<typename T, int N>
constexpr auto operator!=(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool {
  return !(left == right);
}


template<typename T, int N>
auto operator<<(std::ostream& stream, Vector<T, N> const& vector) -> std::ostream& {
  stream << "(";
  for (size_t i = 0; i < N; i++) {
    stream << vector[i];
    if (i != N - 1) {
      stream << ", ";
    }
  }
  stream << ")";
  return stream;
}


template<typename T, int N>
auto Clamp(Vector<T, N> const& v, T const min, T const max) noexcept -> Vector<T, N> {
  Vector<T, N> ret{v};

  for (auto i = 0; i < N; i++) {
    ret[i] = std::clamp(v[i], min, max);
  }

  return ret;
}


template<typename T, int N>
auto Clamp(Vector<T, N> const& v, Vector<T, N> const& min, Vector<T, N> const& max) noexcept -> Vector<T, N> {
  Vector<T, N> ret{v};

  for (auto i = 0; i < N; i++) {
    ret[i] = std::clamp(v[i], min[i], max[i]);
  }

  return ret;
}


template<typename T, int N>
auto Min(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = std::min(left[i], right[i]);
  }
  return ret;
}


template<typename T, int N>
auto Max(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = std::max(left[i], right[i]);
  }
  return ret;
}


template<typename T, int N>
auto ScalarProject(Vector<T, N> const& what, Vector<T, N> const& onto) noexcept -> float {
  return Dot(what, Normalized(onto));
}


template<typename T, int N>
auto VectorProject(Vector<T, N> const& what, Vector<T, N> const& onto) noexcept -> Vector<T, N> {
  return ScalarProject(what, onto) * Normalized(onto);
}


template<typename T, int N>
auto Floor(Vector<T, N> const& v) -> Vector<T, N> {
  Vector<T, N> ret;

  for (auto i = 0; i < N; i++) {
    ret[i] = std::floor(v[i]);
  }

  return ret;
}


template<typename T, int N>
auto Round(Vector<T, N> const& v) -> Vector<T, N> {
  Vector<T, N> ret;

  for (auto i = 0; i < N; i++) {
    ret[i] = std::round(v[i]);
  }

  return ret;
}


#ifdef LEOPPH_MATH_USE_INTRINSICS
template<>
inline auto Length(Vector3 const& vector) noexcept -> float {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(vector.GetData(), mask) };
	auto const xmm1{ _mm_dp_ps(xmm0, xmm0, 0b11110001) };
	auto const xmm2{ _mm_sqrt_ss(xmm1) };
	return _mm_cvtss_f32(xmm2);
}


template<>
inline auto Length(Vector4 const& vector) noexcept -> float {
	auto const xmm0{ _mm_loadu_ps(vector.GetData()) };
	auto const xmm1{ _mm_dp_ps(xmm0, xmm0, 0b11110001) };
	auto const xmm2{ _mm_sqrt_ss(xmm1) };
	return _mm_cvtss_f32(xmm2);
}


template<>
inline auto Dot(Vector3 const& left, Vector3 const& right) noexcept -> float {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), mask) };
	auto const xmm1{ _mm_maskload_ps(right.GetData(), mask) };
	auto const xmm2{ _mm_dp_ps(xmm0, xmm1, 0b11110001) };
	return _mm_cvtss_f32(xmm2);
}


template<>
inline auto Dot(Vector4 const& left, Vector4 const& right) noexcept -> float {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_loadu_ps(right.GetData()) };
	auto const xmm2{ _mm_dp_ps(xmm0, xmm1, 0b11110001) };
	return _mm_cvtss_f32(xmm2);
}


template<>
inline auto Cross(Vector3 const& left, Vector3 const& right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{_mm_maskload_ps(left.GetData(), memMask)};
	auto const xmm1{_mm_maskload_ps(right.GetData(), memMask)};

	auto constexpr makeShuffleMask{ [](int const first, int const second, int const third) {
		return first | second << 2 | third << 4;
	}};

	auto constexpr shuffleMask{makeShuffleMask(1, 2, 0)};
	auto const xmm2{_mm_shuffle_ps(xmm0, xmm0, shuffleMask)};
	auto const xmm3{_mm_shuffle_ps(xmm1, xmm1, shuffleMask)};

	auto const xmm4{_mm_mul_ps(xmm1, xmm2)};
	auto const xmm5{_mm_fmsub_ps(xmm0, xmm3, xmm4)};
	auto const xmm6{_mm_shuffle_ps(xmm5, xmm5, shuffleMask)};

	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm6);
	return ret;
}


template<>
inline auto operator+(Vector3 const& left, Vector3 const& right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{_mm_maskload_ps(left.GetData(), memMask)};
	auto const xmm1{_mm_maskload_ps(right.GetData(), memMask)};
	auto const xmm2{ _mm_add_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm2);
	return ret;
}


template<>
inline auto operator+(Vector4 const& left, Vector4 const& right) noexcept -> Vector4 {
	auto const xmm0{_mm_loadu_ps(left.GetData())};
	auto const xmm1{_mm_loadu_ps(right.GetData())};
	auto const xmm2{ _mm_add_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator+=(Vector3& left, Vector3 const& right) noexcept -> Vector3& {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{_mm_maskload_ps(left.GetData(), memMask)};
	auto const xmm1{_mm_maskload_ps(right.GetData(), memMask)};
	auto const xmm2{ _mm_add_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), memMask, xmm2);
	return left;
}


template<>
inline auto operator+=(Vector4& left, Vector4 const& right) noexcept -> Vector4& {
	auto const xmm0{_mm_loadu_ps(left.GetData())};
	auto const xmm1{_mm_loadu_ps(right.GetData())};
	auto const xmm2{ _mm_add_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}


template<>
inline auto operator-(Vector3 const& left, Vector3 const& right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{_mm_maskload_ps(left.GetData(), memMask)};
	auto const xmm1{_mm_maskload_ps(right.GetData(), memMask)};
	auto const xmm2{ _mm_sub_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm2);
	return ret;
}


template<>
inline auto operator-(Vector4 const& left, Vector4 const& right) noexcept -> Vector4 {
	auto const xmm0{_mm_loadu_ps(left.GetData())};
	auto const xmm1{_mm_loadu_ps(right.GetData())};
	auto const xmm2{ _mm_sub_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator-=(Vector3& left, Vector3 const& right) noexcept -> Vector3& {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{_mm_maskload_ps(left.GetData(), memMask)};
	auto const xmm1{_mm_maskload_ps(right.GetData(), memMask)};
	auto const xmm2{ _mm_sub_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), memMask, xmm2);
	return left;
}


template<>
inline auto operator-=(Vector4& left, Vector4 const& right) noexcept -> Vector4& {
	auto const xmm0{_mm_loadu_ps(left.GetData())};
	auto const xmm1{_mm_loadu_ps(right.GetData())};
	auto const xmm2{ _mm_sub_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}


template<>
inline auto operator*(Vector3 const& left, float const right) noexcept -> Vector3 {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), mask) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), mask, xmm2);
	return ret;
}


template<>
inline auto operator*(Vector4 const& left, float const right) noexcept -> Vector4 {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator*(float const left, Vector3 const& right) noexcept -> Vector3 {
	return right * left;
}


template<>
inline auto operator*(float const left, Vector4 const& right) noexcept -> Vector4 {
	return right * left;
}


template<>
inline auto operator*(Vector3 const& left, Vector3 const& right) noexcept -> Vector3 {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), mask) };
	auto const xmm1{ _mm_maskload_ps(right.GetData(), mask) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), mask, xmm2);
	return ret;
}


template<>
inline auto operator*(Vector4 const& left, Vector4 const& right) noexcept -> Vector4 {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_loadu_ps(right.GetData()) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator*=(Vector3& left, Vector3 const& right) noexcept -> Vector3& {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), mask) };
	auto const xmm1{ _mm_maskload_ps(right.GetData(), mask) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), mask, xmm2);
	return left;
}


template<>
inline auto operator*=(Vector4& left, Vector4 const& right) noexcept -> Vector4& {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_loadu_ps(right.GetData()) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}


template<>
inline auto operator*=(Vector3& left, float const right) noexcept -> Vector3& {
	auto const mask{ _mm_set_epi32(0, 0x80000000, 0x80000000, 0x80000000) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), mask) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), mask, xmm2);
	return left;
}


template<>
inline auto operator*=(Vector4& left, float const right) noexcept -> Vector4& {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_mul_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}


template<>
inline auto operator/(Vector3 const& left, float const right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), memMask) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm2);
	return ret;
}


template<>
inline auto operator/(float const left, Vector3 const& right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{ _mm_maskload_ps(right.GetData(), memMask) };
	auto const xmm1{ _mm_broadcast_ss(&left) };
	auto const xmm2{ _mm_div_ps(xmm1, xmm0) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm2);
	return ret;
}


template<>
inline auto operator/(Vector3 const& left, Vector3 const& right) noexcept -> Vector3 {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), memMask) };
	auto const xmm1{ _mm_maskload_ps(right.GetData(), memMask) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	Vector3 ret;
	_mm_maskstore_ps(ret.GetData(), memMask, xmm2);
	return ret;
}


template<>
inline auto operator/(Vector4 const& left, float const right) noexcept -> Vector4 {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator/(float const left, Vector4 const& right) noexcept -> Vector4 {
	auto const xmm0{ _mm_loadu_ps(right.GetData()) };
	auto const xmm1{ _mm_broadcast_ss(&left) };
	auto const xmm2{ _mm_div_ps(xmm1, xmm0) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator/(Vector4 const& left, Vector4 const& right) noexcept -> Vector4 {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_loadu_ps(right.GetData()) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	Vector4 ret;
	_mm_storeu_ps(ret.GetData(), xmm2);
	return ret;
}


template<>
inline auto operator/=(Vector3& left, float const right) noexcept -> Vector3& {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), memMask) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), memMask, xmm2);
	return left;
}


template<>
inline auto operator/=(Vector3& left, Vector3 const& right) noexcept -> Vector3& {
	auto const memMask{ _mm_set_epi32(0, 1 << 31, 1 << 31, 1 << 31) };
	auto const xmm0{ _mm_maskload_ps(left.GetData(), memMask) };
	auto const xmm1{ _mm_maskload_ps(right.GetData(), memMask) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	_mm_maskstore_ps(left.GetData(), memMask, xmm2);
	return left;
}


template<>
inline auto operator/=(Vector4& left, float const right) noexcept -> Vector4& {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_broadcast_ss(&right) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}


template<>
inline auto operator/=(Vector4& left, Vector4 const& right) noexcept -> Vector4& {
	auto const xmm0{ _mm_loadu_ps(left.GetData()) };
	auto const xmm1{ _mm_loadu_ps(right.GetData()) };
	auto const xmm2{ _mm_div_ps(xmm0, xmm1) };
	_mm_storeu_ps(left.GetData(), xmm2);
	return left;
}
#endif


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Determinant() const noexcept -> T requires (N == M) {
  Matrix<T, N, M> tmp{*this};
  for (size_t i = 1; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      tmp[j][0] += tmp[j][i];
    }
  }
  for (size_t i = 1; i < N; i++) {
    tmp[i] -= tmp[0];
  }
  auto ret{static_cast<T>(1)};
  for (size_t i = 0; i < N; i++) {
    ret *= tmp[i][i];
  }
  return ret;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Transpose() const noexcept -> Matrix<T, M, N> {
  Matrix<T, M, N> ret;
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < M; j++) {
      ret[j][i] = mData[i][j];
    }
  }
  return ret;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Inverse() const noexcept -> Matrix<T, N, M> requires (N == M) {
  Matrix<T, N, M> left{*this};
  Matrix<T, N, M> right{Matrix<T, N, M>::Identity()};

  // Iterate over the main diagonal/submatrices
  for (auto i = 0; i < N; i++) {
    // Index of the row with the largest absolute value element in the ith column
    auto pivotInd{i};

    // Find index of the row with the largest absolute value element
    for (auto j = pivotInd + 1; j < N; j++) {
      if (std::abs(left[j][i]) > std::abs(left[pivotInd][i])) {
        pivotInd = j;
      }
    }

    // swap pivot row with the ith
    auto tmp{left[i]};
    left[i] = left[pivotInd];
    left[pivotInd] = tmp;

    tmp = right[i];
    right[i] = right[pivotInd];
    right[pivotInd] = tmp;

    // If the main diagonal element is non-zero
    // 1. Normalize the row so that the element is 1
    // 2. Reduce all rows below so that its elements under are zero
    // If the main diagonal element is zero, this will produce NAN, but that's fine
    // Because then zero is the number with the highest absolute value in the column
    // So all other elements are also zero in the column
    // The matrix is singular and we return garbage
    auto const div{left[i][i]};
    left[i] /= div;
    right[i] /= div;

    for (auto j = i + 1; j < N; j++) {
      auto const mult{left[j][i]}; // Theoretically left[i][i] is 1 so the multiplier is the element itself
      left[j] -= mult * left[i];
      right[j] -= mult * right[i];
    }
  }

  // Left is in reduced echelon form
  // Now eliminate the remaining non-zeros outside the main diagonal
  for (auto i = 0; i < N; i++) {
    for (auto j = i + 1; j < N; j++) {
      // We subtract the jth row from the ith one
      // Because it is guaranteed to have 0s before the main diagonal
      // And thus it won't mess up the element in the ith row before the jth element
      auto const mult{left[i][j]}; // left[j][j] is 1 so the multiplier is the element itself
      left[i] -= mult * left[j];
      right[i] -= mult * right[j];
    }
  }
  return right;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Trace() const noexcept -> float requires (N == M) {
  float ret{0};
  for (auto i{0}; i < N; i++) {
    for (auto j{0}; j < M; j++) {
      ret += mData[i][j];
    }
  }
  return ret;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::operator[](size_t const index) const noexcept -> Vector<T, M> const& {
  return mData[index];
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::operator[](size_t const index) noexcept -> Vector<T, M>& {
  return mData[index];
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::GetData() noexcept -> T* {
  return mData[0].GetData();
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::GetData() const noexcept -> T const* {
  return mData[0].GetData();
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Identity() noexcept -> Matrix<T, N, M> requires (N == M) {
  return Matrix<T, N, M>{1};
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Translate(Vector<T, 3> const& vector) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  Matrix<T, 4, 4> ret = Identity();
  for (size_t i = 0; i < 3; i++) {
    ret[3][i] = vector[i];
  }
  return ret;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Scale(Vector<T, 3> const& vector) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  Matrix<T, 4, 4> ret = Identity();
  for (size_t i = 0; i < 3; i++) {
    ret[i][i] = vector[i];
  }
  return ret;
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::LookTo(Vector<T, 3> const& position, Vector<T, 3> const& direction,
                                       Vector<T, 3> const& worldUp) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  Vector<T, 3> z{Normalized(direction)};
  Vector<T, 3> x{Normalized(Cross(worldUp, z))};
  Vector<T, 3> y{Cross(z, x)};
  return Matrix<T, 4, 4>{
    x[0], y[0], z[0], 0,
    x[1], y[1], z[1], 0,
    x[2], y[2], z[2], 0,
    -Dot(x, position), -Dot(y, position), -Dot(z, position), 1
  };
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::LookAt(Vector<T, 3> const& position, Vector<T, 3> const& target,
                                       Vector<T, 3> const& worldUp) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  return LookTo(position, target - position, worldUp);
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::OrthographicOffCenter(T const left, T const right, T const top, T const bottom,
                                                      T const zNear,
                                                      T const zFar) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  return Matrix<T, 4, 4>{
    2 / (right - left), 0, 0, 0,
    0, 2 / (top - bottom), 0, 0,
    0, 0, 1 / (zFar - zNear), 0,
    (left + right) / (left - right), (top + bottom) / (bottom - top), zNear / (zNear - zFar), 1
  };
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Orthographic(T const width, T const height, T const zNear,
                                             T const zFar) noexcept -> Matrix<T, 4, 4> requires (N == 4 && M == 4) {
  auto const halfWidth{width / 2};
  auto const halfHeight{height / 2};
  return OrthographicOffCenter(-halfWidth, halfWidth, halfHeight, -halfHeight, zNear, zFar);
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::PerspectiveOffCenter(T const left, T const right, T const top, T const bottom,
                                                     T const zNear,
                                                     T const zFar) noexcept -> Matrix<T, 4, 4> requires (
  N == 4 && M == 4) {
  return Matrix<T, 4, 4>{
    2 * zNear / (right - left), 0, 0, 0,
    0, 2 * zNear / (top - bottom), 0, 0,
    (left + right) / (left - right), (top + bottom) / (bottom - top), zFar / (zFar - zNear), 1,
    0, 0, zNear * zFar / (zNear - zFar), 0
  };
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Perspective(T const width, T const height, T const zNear,
                                            T const zFar) noexcept -> Matrix<T, 4, 4> requires (N == 4 && M == 4) {
  auto const halfWidth{width / static_cast<T>(2)};
  auto const halfHeight{height / static_cast<T>(2)};
  return PerspectiveOffCenter(-halfWidth, halfWidth, halfHeight, -halfHeight, zNear, zFar);
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
auto Matrix<T, N, M>::PerspectiveFov(T const fovVertRad, T const aspectRatio, T const zNear,
                                     T const zFar) noexcept -> Matrix<T, 4, 4> requires (N == 4 && M == 4) {
  auto const halfFov{fovVertRad / 2};
  auto const yScale{std::cos(halfFov) / std::sin(halfFov)};
  auto const xScale{yScale / aspectRatio};
  return Matrix<T, 4, 4>{
    xScale, 0, 0, 0,
    0, yScale, 0, 0,
    0, 0, zFar / (zFar - zNear), 1,
    0, 0, -zNear * zFar / (zFar - zNear), 0
  };
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Zero() noexcept -> Matrix {
  return Matrix{};
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr Matrix<T, N, M>::Matrix(T const value) noexcept {
  for (size_t i = 0; i < std::min(N, M); i++) {
    mData[i][i] = value;
  }
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
constexpr auto Matrix<T, N, M>::Diagonal(T const value) noexcept -> Matrix {
  return Matrix{value};
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<std::convertible_to<T>... Args> requires (sizeof...(Args) == std::min(N, M))
constexpr Matrix<T, N, M>::Matrix(Args&&... args) noexcept {
  [this, &args...] <int... Is>(std::index_sequence<Is...>) noexcept {
    (static_cast<void>(mData[Is][Is] = static_cast<T>(args)), ...);
  }(std::index_sequence_for<Args...>{});
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<std::convertible_to<T>... Args> requires (sizeof...(Args) == std::min(N, M))
constexpr auto Matrix<T, N, M>::Diagonal(Args&&... args) noexcept -> Matrix {
  return Matrix{std::forward<Args>(args)...};
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<int K> requires (K == std::min(N, M))
constexpr Matrix<T, N, M>::Matrix(Vector<T, K> const& vec) noexcept {
  for (size_t i = 0; i < K; i++) {
    mData[i][i] = vec[i];
  }
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<int K> requires (K == std::min(N, M))
constexpr auto Matrix<T, N, M>::Diagonal(Vector<T, K> const& vec) noexcept -> Matrix {
  return Matrix{vec};
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<std::convertible_to<T>... Args> requires (sizeof...(Args) == static_cast<unsigned long long>(N * M))
constexpr Matrix<T, N, M>::Matrix(Args&&... args) noexcept {
  [this, ...args = std::forward<Args>(args)]<auto... Is>(std::index_sequence<Is...>) noexcept {
    (static_cast<void>(mData[Is / M][Is % M] = static_cast<T>(args)), ...);
  }(std::index_sequence_for<Args...>{});
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<int N1, int M1> requires (N1 < N && M1 < M)
constexpr Matrix<T, N, M>::Matrix(Matrix<T, N1, M1> const& other) noexcept {
  for (auto i = 0; i < N1; i++) {
    mData[i] = Vector<T, M>{other[i], 0};
  }

  for (int i = N1; i < N; ++i) {
    mData[i] = Vector<T, M>{0};
  }

  mData[N - 1][M - 1] = static_cast<T>(1);
}


template<typename T, int N, int M> requires (N > 1 && M > 1)
template<int N1, int M1> requires (N1 > N && M1 > M)
constexpr Matrix<T, N, M>::Matrix(Matrix<T, N1, M1> const& other) noexcept {
  for (auto i = 0; i < N; i++) {
    mData[i] = Vector<T, M>{other[i]};
  }
}


template<typename T, int N, int M>
constexpr auto operator+(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M> {
  Matrix<T, N, M> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] + right[i];
  }
  return ret;
}


template<typename T, int N, int M>
constexpr auto operator+=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>& {
  for (auto i = 0; i < N; i++) {
    left[i] += right[i];
  }
  return left;
}


template<typename T, int N, int M>
constexpr auto operator-(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M> {
  Matrix<T, N, M> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] - right[i];
  }
  return ret;
}


template<typename T, int N, int M>
constexpr auto operator-=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M>& {
  for (auto i = 0; i < N; i++) {
    left[i] -= right[i];
  }
  return left;
}


template<typename T, int N, int M, std::convertible_to<T> T1>
constexpr auto operator*(Matrix<T, N, M> const& left, T1 const& right) noexcept -> Matrix<T, N, M> {
  Matrix<T, N, M> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = left[i] * static_cast<T>(right);
  }
  return ret;
}


template<typename T, int N, int M, std::convertible_to<T> T1>
constexpr auto operator*(T1 const& left, Matrix<T, N, M> const& right) noexcept -> Matrix<T, N, M> {
  Matrix<T, N, M> ret;
  for (auto i = 0; i < N; i++) {
    ret[i] = static_cast<T>(left) * right[i];
  }
  return ret;
}


template<typename T, int N, int M>
constexpr auto operator*(Matrix<T, N, M> const& left, Vector<T, M> const& right) noexcept -> Vector<T, N> {
  Vector<T, N> ret;
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < M; j++) {
      ret[i] += left[i][j] * right[j];
    }
  }
  return ret;
}


template<typename T, int N, int M>
constexpr auto operator*(Vector<T, N> const& left, Matrix<T, N, M> const& right) noexcept -> Vector<T, M> {
  Vector<T, M> ret;
  for (size_t j = 0; j < M; j++) {
    for (size_t i = 0; i < N; i++) {
      ret[j] += left[i] * right[i][j];
    }
  }
  return ret;
}


template<typename T, int N, int M, int P>
constexpr auto operator*(Matrix<T, N, M> const& left, Matrix<T, M, P> const& right) noexcept -> Matrix<T, N, P> {
  Matrix<T, N, P> ret;
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < P; j++) {
      for (size_t k = 0; k < M; k++) {
        ret[i][j] += left[i][k] * right[k][j];
      }
    }
  }
  return ret;
}


template<typename T, int N, int M, std::convertible_to<T> T1>
constexpr auto operator*=(Matrix<T, N, M>& left, T1 const& right) noexcept -> Matrix<T, N, M>& {
  for (auto i = 0; i < N; i++) {
    left[i] *= static_cast<T>(right);
  }
  return left;
}


template<typename T, int N>
constexpr auto operator*=(Vector<T, N>& left, Matrix<T, N, N> const& right) noexcept -> Vector<T, N>& {
  return left = left * right;
}


template<typename T, int N, int M>
constexpr auto operator*=(Matrix<T, N, M>& left, Matrix<T, M, M> const& right) noexcept -> Matrix<T, N, M>& {
  return left = left * right;
}


template<typename T, int N, int M>
auto operator<<(std::ostream& stream, Matrix<T, N, M> const& matrix) -> std::ostream& {
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < M; j++) {
      stream << matrix[i][j];
      if (j != M - 1) {
        stream << " ";
      }
    }
    if (i != N - 1) {
      stream << std::endl;
    }
  }
  return stream;
}


#ifdef LEOPPH_MATH_USE_INTRINSICS
template<>
inline auto operator*(Matrix4 const& left, Matrix4 const& right) noexcept -> Matrix4 {
	__m128 rows[4];
	__m128 cols[4];

	auto const colOffsetIndices{ _mm_set_epi32(12, 8, 4, 0) };
	auto constexpr colOffsetIndexScale{ 4 };

	for (int i = 0; i < 4; i++) {
		rows[i] = _mm_loadu_ps(left[i].GetData());
		cols[i] = _mm_i32gather_ps(right[0].GetData(), _mm_add_epi32(colOffsetIndices, _mm_set1_epi32(i)), colOffsetIndexScale);
	}

	Matrix4 ret;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret[i][j] = _mm_cvtss_f32(_mm_dp_ps(rows[i], cols[j], 0b11110001));
		}
	}

	return ret;
}
#endif

constexpr Quaternion::Quaternion(float const w, float const x, float const y, float const z) noexcept :
  x{x},
  y{y},
  z{z},
  w{w} {}


inline Quaternion::Quaternion(Vector3 const& axis, float const angleDegrees) noexcept {
  auto const angleHalfRadians = ToRadians(angleDegrees) / 2.0f;
  auto vec = sorcery::Normalized(axis) * std::sin(angleHalfRadians);

  w = std::cos(angleHalfRadians);
  x = vec[0];
  y = vec[1];
  z = vec[2];
}


inline auto Quaternion::FromEulerAngles(float const x, float const y, float const z) noexcept -> Quaternion {
  auto const pitch = ToRadians(x);
  auto const yaw = ToRadians(y);
  auto const roll = ToRadians(z);

  auto const cp = std::cos(pitch * 0.5f);
  auto const sp = std::sin(pitch * 0.5f);
  auto const cy = std::cos(yaw * 0.5f);
  auto const sy = std::sin(yaw * 0.5f);
  auto const cr = std::cos(roll * 0.5f);
  auto const sr = std::sin(roll * 0.5f);

  return Quaternion
  {
    cp * cy * cr + sp * sy * sr,
    sp * cy * cr - cp * sy * sr,
    cp * sy * cr + sp * cy * sr,
    cp * cy * sr - sp * sy * cr,
  };
}


inline auto Quaternion::FromEulerAngles(Vector3 const& euler) noexcept -> Quaternion {
  return FromEulerAngles(euler[0], euler[1], euler[2]);
}


inline auto Quaternion::ToEulerAngles() const noexcept -> Vector3 {
  Vector3 angles;

  auto const pitch_y{2 * (w * x + y * z)};
  auto const pitch_x{w * w - x * x - y * y + z * z};
  angles[0] = std::atan2(pitch_y, pitch_x);

  angles[1] = std::asin(std::clamp(-2 * (x * z - w * y), -1.0f, 1.0f));

  auto const roll_y{2 * (w * z + x * y)};
  auto const roll_x{w * w + x * x - y * y - z * z};
  angles[2] = std::atan2(roll_y, roll_x);

  for (auto i = 0; i < 3; i++) {
    angles[i] = ToDegrees(angles[i]);
  }

  return angles;
}


constexpr auto Quaternion::ToRotationMatrix() const noexcept -> Matrix4 {
  return static_cast<Matrix4>(*this);
}


inline auto Quaternion::FromAxisAngle(Vector3 const& axis, float const angleDeg) noexcept -> Quaternion {
  return Quaternion{axis, angleDeg};
}


inline auto Quaternion::ToAxisAngle(Vector3& axis, float& angle) const noexcept -> void {
  axis = Vector3{x, y, z};
  float const vectorLength = Length(axis);
  sorcery::Normalize(axis);
  angle = 2 * std::atan2(vectorLength, w);
}


inline auto Quaternion::FromTo(Vector3 const& from, Vector3 const& to) noexcept -> Quaternion {
  auto const crossProduct{Cross(from, to)};
  return Quaternion{
    std::sqrt(std::pow(Length(from), 2.f) * std::pow(Length(to), 2.f)) + Dot(from, to),
    crossProduct[0],
    crossProduct[1],
    crossProduct[2],
  }.Normalize();
}


constexpr auto Quaternion::NormSquared() const noexcept -> float {
  return w * w + x * x + y * y + z * z;
}


inline auto Quaternion::Norm() const noexcept -> float {
  return std::sqrt(NormSquared());
}


inline auto Quaternion::Normalized() const noexcept -> Quaternion {
  return Quaternion{*this}.Normalize();
}


inline auto Quaternion::Normalize() noexcept -> Quaternion& {
  auto const norm = Norm();
  w /= norm;
  x /= norm;
  y /= norm;
  z /= norm;
  return *this;
}


constexpr auto Quaternion::Conjugate() const noexcept -> Quaternion {
  return Quaternion{*this}.ConjugateInPlace();
}


constexpr auto Quaternion::ConjugateInPlace() noexcept -> Quaternion& {
  x = -x;
  y = -y;
  z = -z;
  return *this;
}


constexpr auto Quaternion::Inverse() const noexcept -> Quaternion {
  return Quaternion{*this}.Invert();
}


constexpr auto Quaternion::Invert() noexcept -> Quaternion& {
  auto const normSquared = NormSquared();
  w /= normSquared;
  x = -x / normSquared;
  y = -y / normSquared;
  z = -z / normSquared;
  return *this;
}


constexpr Quaternion::operator Matrix4() const noexcept {
  return Matrix4
  {
    1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w), 0,
    2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w), 0,
    2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y), 0,
    0, 0, 0, 1
  };
}


template<typename T>
constexpr auto Quaternion::Rotate(Vector<T, 3> const& vec) const noexcept -> Vector<T, 3> {
  auto const retQuat = *this * Quaternion{0, vec[0], vec[1], vec[2]} * Conjugate();
  return Vector<T, 3>{retQuat.x, retQuat.y, retQuat.z};
}


constexpr auto operator+(Quaternion const& left, Quaternion const& right) -> Quaternion {
  return Quaternion{left.w + right.w, left.x + right.x, left.y + right.y, left.z + right.z};
}


constexpr auto operator+=(Quaternion& left, Quaternion const& right) -> Quaternion& {
  return left = left + right;
}


constexpr auto operator*(Quaternion const& left, Quaternion const& right) noexcept -> Quaternion {
  return Quaternion{
    left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
    left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
    left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
    left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w
  };
}


constexpr auto operator*(Quaternion const& left, float const right) -> Quaternion {
  return Quaternion{left.w * right, left.x * right, left.y * right, left.z * right};
}


constexpr auto operator*(float const left, Quaternion const& right) -> Quaternion {
  return right * left;
}


constexpr auto operator*=(Quaternion& left, Quaternion const& right) noexcept -> Quaternion& {
  return left = left * right;
}


constexpr auto operator*=(Quaternion& left, float const right) -> Quaternion& {
  return left = left * right;
}


inline auto operator<<(std::ostream& os, Quaternion const& q) -> std::ostream& {
  return os << "(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
}


inline auto Slerp(Quaternion const& from, Quaternion const& to, float const amount) -> Quaternion {
  auto const cos_angle{from.w * to.w + from.x * to.x + from.y * to.y + from.z * to.z};

  if (std::abs(cos_angle) >= 1) {
    return from;
  }

  auto const angle{std::acos(cos_angle)};
  auto const sin_angle{std::sqrt(1 - cos_angle * cos_angle)};

  if (std::abs(sin_angle) < std::numeric_limits<float>::epsilon()) {
    return 0.5f * from + 0.5f * to;
  }

  return std::sin((1 - amount) * angle) / sin_angle * from + std::sin(amount * angle) / sin_angle * to;
}
}
