#include "OrthographicCamera.hpp"

#include "../util/Logger.hpp"

#include <string>


namespace leopph
{
	auto OrthographicCamera::Size(Side const side) const noexcept -> float
	{
		if (side == Side::Horizontal)
		{
			return m_HorizontalSize;
		}

		if (side == Side::Vertical)
		{
			return m_HorizontalSize / AspectRatio();
		}

		internal::Logger::Instance().Critical("Invalid side \"" + std::to_string(static_cast<int>(side)) + "\" while returning camera size. Returning 0.");
		return 0;
	}


	auto OrthographicCamera::Size(float const size, Side const side) noexcept -> void
	{
		if (side == Side::Horizontal)
		{
			m_HorizontalSize = size;
		}
		else if (side == Side::Vertical)
		{
			m_HorizontalSize = size * AspectRatio();
		}
		else
		{
			internal::Logger::Instance().Error("Invalid side \"" + std::to_string(static_cast<int>(side)) + "\" while setting camera size. Ignoring.");
		}
	}


	auto OrthographicCamera::ProjectionMatrix() const -> Matrix4
	{
		auto static constexpr half = 1.f / 2.f;
		auto const x = m_HorizontalSize * half;
		auto const y = m_HorizontalSize / AspectRatio() * half;
		return Matrix4::Ortographic(-x, x, y, -y, NearClipPlane(), FarClipPlane());
	}


	auto OrthographicCamera::Frustum() const -> leopph::Frustum
	{
		auto static constexpr half = 1.f / 2.f;
		auto const x = m_HorizontalSize * half;
		auto const y = m_HorizontalSize / AspectRatio() * half;
		return leopph::Frustum
		{
			.NearTopLeft{-x, y, NearClipPlane()},
			.NearBottomLeft{-x, -y, NearClipPlane()},
			.NearBottomRight{x, -y, NearClipPlane()},
			.NearTopRight{x, y, NearClipPlane()},
			.FarTopLeft{-x, y, FarClipPlane()},
			.FarBottomLeft{-x, -y, FarClipPlane()},
			.FarBottomRight = {x, -y, FarClipPlane()},
			.FarTopRight = {x, y, FarClipPlane()}
		};
	}
}
