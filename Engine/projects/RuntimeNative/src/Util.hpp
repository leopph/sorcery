#pragma once

#include "Core.hpp"
#include "Math.hpp"

#include <algorithm>
#include <bitset>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>


namespace leopph {
	template<typename T>
	struct Extent2D {
		T width;
		T height;
	};


	template<typename T>
	struct Point2D {
		T x;
		T y;
	};


	struct Viewport {
		Point2D<u32> position;
		Extent2D<u32> extent;
	};


	struct NormalizedViewport {
		Point2D<f32> position;
		Extent2D<f32> extent;
	};


	struct Frustum {
		Vector3 rightTopNear;
		Vector3 leftTopNear;
		Vector3 leftBottomNear;
		Vector3 rightBottomNear;
		Vector3 rightTopFar;
		Vector3 leftTopFar;
		Vector3 leftBottomFar;
		Vector3 rightBottomFar;
	};


	template<std::integral To, std::integral From>
	auto clamp_cast(From const what) -> To {
		if constexpr (std::cmp_less_equal(std::numeric_limits<To>::min(), std::numeric_limits<From>::min())) {
			if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max())) {
				return static_cast<To>(what);
			}
			else {
				return static_cast<To>(std::min<From>(what, std::numeric_limits<To>::max()));
			}
		}
		else {
			if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max())) {
				return static_cast<To>(std::max<From>(what, std::numeric_limits<To>::min()));
			}
			else {
				return static_cast<To>(std::clamp<From>(what, std::numeric_limits<To>::min(), std::numeric_limits<To>::max()));
			}
		}
	}


	template<auto& Obj, auto MemberFunc, typename... Args>
	auto Call(Args&&... args) noexcept(std::is_nothrow_invocable_v<decltype(MemberFunc), decltype(Obj), Args...>) {
		return (Obj.*MemberFunc)(std::forward<Args>(args)...);
	}


	template<typename T>
	using NonOwning = T;


	template<class T>
	concept Scalar = std::is_scalar_v<T>;
}
