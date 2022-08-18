#include "PerspectiveCamera.hpp"

#include "Logger.hpp"
#include "Math.hpp"

#include <format>


namespace leopph
{
	f32 PerspectiveCamera::get_fov(Side const side) const
	{
		if (side == Side::Horizontal)
		{
			return mHorizFovDeg;
		}

		if (side == Side::Vertical)
		{
			return convert_fov(mHorizFovDeg, Conversion::HorizontalToVertical);
		}

		Logger::get_instance().warning(std::format("Invalid side [{}] while querying camera field of view. Returning 0.", static_cast<int>(side)));
		return 0;
	}



	void PerspectiveCamera::set_fov(f32 const degrees, Side const side)
	{
		if (side == Side::Horizontal)
		{
			mHorizFovDeg = degrees;
		}
		else if (side == Side::Vertical)
		{
			mHorizFovDeg = convert_fov(degrees, Conversion::VerticalToHorizontal);
		}
		else
		{
			Logger::get_instance().warning(std::format("Invalid side [{}] while setting camera field of view. Ignoring.", static_cast<int>(side)));
		}
	}



	Matrix4 PerspectiveCamera::build_projection_matrix() const
	{
		auto const fov{to_radians(convert_fov(mHorizFovDeg, Conversion::HorizontalToVertical))};
		return Matrix4::perspective(fov, get_aspect_ratio(), get_near_clip_plane(), get_far_clip_plane());
	}



	Frustum PerspectiveCamera::build_frustum() const
	{
		auto const tanHalfHorizFov{std::tan(to_radians(get_fov(Side::Horizontal)) / 2.0f)};
		auto const tanHalfVertFov{std::tan(to_radians(get_fov(Side::Vertical)) / 2.0f)};

		auto const near = get_near_clip_plane();
		auto const far = get_far_clip_plane();

		auto const xn = near * tanHalfHorizFov;
		auto const xf = far * tanHalfHorizFov;
		auto const yn = near * tanHalfVertFov;
		auto const yf = far * tanHalfVertFov;

		return Frustum
		{
			.rightTopNear{xn, yn, near},
			.leftTopNear{-xn, yn, near},
			.leftBottomNear{-xn, -yn, near},
			.rightBottomNear{xn, -yn, near},
			.rightTopFar{xf, yf, far},
			.leftTopFar{-xf, yf, far},
			.leftBottomFar{-xf, -yf, far},
			.rightBottomFar{xf, -yf, far},
		};
	}



	f32 PerspectiveCamera::convert_fov(f32 const fov, Conversion const conversion) const
	{
		if (conversion == Conversion::VerticalToHorizontal)
		{
			return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) * get_aspect_ratio()));
		}

		if (conversion == Conversion::HorizontalToVertical)
		{
			return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) / get_aspect_ratio()));
		}

		Logger::get_instance().warning(std::format("Invalid direction [{}] while converting camera field of view. Returning 0.", static_cast<int>(conversion)));
		return 0;
	}
}
