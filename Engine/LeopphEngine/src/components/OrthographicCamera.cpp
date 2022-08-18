#include "OrthographicCamera.hpp"

#include "Logger.hpp"

#include <format>


namespace leopph
{
	f32 OrthographicCamera::get_size(Side const side) const
	{
		if (side == Side::Horizontal)
		{
			return mHorizSize;
		}

		if (side == Side::Vertical)
		{
			return mHorizSize / get_aspect_ratio();
		}

		Logger::get_instance().warning(std::format("Invalid side [{}] while returning camera size. Returning 0.", static_cast<int>(side)));
		return 0;
	}



	void OrthographicCamera::size(f32 const size, Side const side)
	{
		if (side == Side::Horizontal)
		{
			mHorizSize = size;
		}
		else if (side == Side::Vertical)
		{
			mHorizSize = size * get_aspect_ratio();
		}
		else
		{
			Logger::get_instance().warning(std::format("Invalid side [{}] while setting camera size. Ignoring.", static_cast<int>(side)));
		}
	}



	Matrix4 OrthographicCamera::build_projection_matrix() const
	{
		auto static constexpr half = 1.f / 2.f;
		auto const x = mHorizSize * half;
		auto const y = mHorizSize / get_aspect_ratio() * half;
		return Matrix4::orthographic(-x, x, y, -y, get_near_clip_plane(), get_far_clip_plane());
	}



	Frustum OrthographicCamera::build_frustum() const
	{
		auto static constexpr half = 1.f / 2.f;

		auto const x = mHorizSize * half;
		auto const y = mHorizSize / get_aspect_ratio() * half;

		auto const near = get_near_clip_plane();
		auto const far = get_far_clip_plane();

		return Frustum
		{
			.rightTopNear{x, y, near},
			.leftTopNear{-x, y, near},
			.leftBottomNear{-x, -y, near},
			.rightBottomNear{x, -y, near},
			.rightTopFar = {x, y, far},
			.leftTopFar{-x, y, far},
			.leftBottomFar{-x, -y, far},
			.rightBottomFar = {x, -y, far},
		};
	}
}
