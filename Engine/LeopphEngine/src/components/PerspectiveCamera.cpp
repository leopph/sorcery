#include "PerspectiveCamera.hpp"

#include "Logger.hpp"

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

		internal::Logger::Instance().Warning(std::format("Invalid side [{}] while querying camera field of view. Returning 0.", static_cast<int>(side)));
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
			internal::Logger::Instance().Warning(std::format("Invalid side [{}] while setting camera field of view. Ignoring.", static_cast<int>(side)));
		}
	}



	Matrix4 PerspectiveCamera::build_projection_matrix() const
	{
		auto const fov{to_radians(convert_fov(mHorizFovDeg, Conversion::HorizontalToVertical))};
		return Matrix4::Perspective(fov, get_aspect_ratio(), get_near_clip_plane(), get_far_clip_plane());
	}



	Frustum PerspectiveCamera::build_frustum() const
	{
		auto const tanHalfHorizFov{std::tan(to_radians(get_fov(Side::Horizontal)) / 2.0f)};
		auto const tanHalfVertFov{std::tan(to_radians(get_fov(Side::Vertical)) / 2.0f)};

		auto const xn = get_near_clip_plane() * tanHalfHorizFov;
		auto const xf = get_far_clip_plane() * tanHalfHorizFov;
		auto const yn = get_near_clip_plane() * tanHalfVertFov;
		auto const yf = get_far_clip_plane() * tanHalfVertFov;

		return Frustum
		{
			.NearTopLeft{-xn, yn, get_near_clip_plane()},
			.NearBottomLeft{-xn, -yn, get_near_clip_plane()},
			.NearBottomRight{xn, -yn, get_near_clip_plane()},
			.NearTopRight{xn, yn, get_near_clip_plane()},
			.FarTopLeft{-xf, yf, get_far_clip_plane()},
			.FarBottomLeft{-xf, -yf, get_far_clip_plane()},
			.FarBottomRight{xf, -yf, get_far_clip_plane()},
			.FarTopRight{xf, yf, get_far_clip_plane()}
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

		internal::Logger::Instance().Warning(std::format("Invalid direction [{}] while converting camera field of view. Returning 0.", static_cast<int>(conversion)));
		return 0;
	}
}
