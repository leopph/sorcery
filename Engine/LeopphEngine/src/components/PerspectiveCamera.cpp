#include "PerspectiveCamera.hpp"

#include "Logger.hpp"

#include <string>


namespace leopph
{
	float PerspectiveCamera::Fov(Side const side) const noexcept
	{
		if (side == Side::Horizontal)
		{
			return m_HorizontalFovDegrees;
		}

		if (side == Side::Vertical)
		{
			return ConvertFov(m_HorizontalFovDegrees, Conversion::HorizontalToVertical);
		}

		internal::Logger::Instance().Critical("Invalid side \"" + std::to_string(static_cast<int>(side)) + "\" while returning camera field of view. Returning 0.");
		return 0;
	}


	void PerspectiveCamera::Fov(float const degrees, Side const side) noexcept
	{
		if (side == Side::Horizontal)
		{
			m_HorizontalFovDegrees = degrees;
		}
		else if (side == Side::Vertical)
		{
			m_HorizontalFovDegrees = ConvertFov(degrees, Conversion::VerticalToHorizontal);
		}
		else
		{
			internal::Logger::Instance().Error("Invalid side \"" + std::to_string(static_cast<int>(side)) + "\" while setting camera field of view. Ignoring.");
		}
	}


	Matrix4 PerspectiveCamera::ProjectionMatrix() const
	{
		auto const fov{math::ToRadians(ConvertFov(m_HorizontalFovDegrees, Conversion::HorizontalToVertical))};
		return Matrix4::Perspective(fov, AspectRatio(), NearClipPlane(), FarClipPlane());
	}


	leopph::Frustum PerspectiveCamera::Frustum() const
	{
		auto const tanHalfHorizFov{math::Tan(math::ToRadians(Fov(Side::Horizontal)) / 2.0f)};
		auto const tanHalfVertFov{math::Tan(math::ToRadians(Fov(Side::Vertical)) / 2.0f)};

		auto const xn = NearClipPlane() * tanHalfHorizFov;
		auto const xf = FarClipPlane() * tanHalfHorizFov;
		auto const yn = NearClipPlane() * tanHalfVertFov;
		auto const yf = FarClipPlane() * tanHalfVertFov;

		return leopph::Frustum
		{
			.NearTopLeft{-xn, yn, NearClipPlane()},
			.NearBottomLeft{-xn, -yn, NearClipPlane()},
			.NearBottomRight{xn, -yn, NearClipPlane()},
			.NearTopRight{xn, yn, NearClipPlane()},
			.FarTopLeft{-xf, yf, FarClipPlane()},
			.FarBottomLeft{-xf, -yf, FarClipPlane()},
			.FarBottomRight{xf, -yf, FarClipPlane()},
			.FarTopRight{xf, yf, FarClipPlane()}
		};
	}


	ComponentPtr<> PerspectiveCamera::Clone() const
	{
		return CreateComponent<PerspectiveCamera>(*this);
	}


	float PerspectiveCamera::ConvertFov(float const fov, Conversion const conversion) const
	{
		if (conversion == Conversion::VerticalToHorizontal)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * AspectRatio()));
		}

		if (conversion == Conversion::HorizontalToVertical)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / AspectRatio()));
		}

		internal::Logger::Instance().Critical("Invalid direction \"" + std::to_string(static_cast<int>(conversion)) + "\" while converting camera field of view. Returning 0.");
		return 0;
	}
}
