#pragma once

#include "Core.hpp"
#include "Math.hpp"

#include <bitset>
#include <string>
#include <string_view>


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


	class Guid {
	private:
		leopph::u64 data0;
		leopph::u64 data1;

	public:
		[[nodiscard]] LEOPPHAPI static auto Generate() -> Guid;
		[[nodiscard]] LEOPPHAPI static auto Parse(std::string_view str) -> Guid;

		[[nodiscard]] LEOPPHAPI auto ToString() const->std::string;
		[[nodiscard]] LEOPPHAPI auto operator==(Guid const& other) -> bool;
	};
}