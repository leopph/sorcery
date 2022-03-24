#include "PerspectiveCamera.hpp"

#include "../util/Logger.hpp"


namespace leopph
{
	auto PerspectiveCamera::Fov(float const degrees, FovDirection const direction) -> void
	{
		if (direction == FovDirection::Horizontal)
		{
			m_HorizontalFovDegrees = degrees;
		}
		else if (direction == FovDirection::Vertical)
		{
			m_HorizontalFovDegrees = ConvertFov(degrees, FovConversionDirection::VerticalToHorizontal);
		}
	}


	auto PerspectiveCamera::Fov(FovDirection const direction) const -> float
	{
		if (direction == FovDirection::Horizontal)
		{
			return m_HorizontalFovDegrees;
		}
		if (direction == FovDirection::Vertical)
		{
			return ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical);
		}
		auto const errMsg{"Invalid FOV direction."};
		internal::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}


	auto PerspectiveCamera::ProjectionMatrix() const -> Matrix4
	{
		auto const fov{math::ToRadians(ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical))};
		return Matrix4::Perspective(fov, AspectRatio(), NearClipPlane(), FarClipPlane());
	}


	auto PerspectiveCamera::Frustum() const -> leopph::Frustum
	{
		auto const tanHalfHorizFov{math::Tan(math::ToRadians(Fov(FovDirection::Horizontal)) / 2.0f)};
		auto const tanHalfVertFov{math::Tan(math::ToRadians(Fov(FovDirection::Vertical)) / 2.0f)};

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


	auto PerspectiveCamera::ConvertFov(float const fov, FovConversionDirection const conversion) const -> float
	{
		if (conversion == FovConversionDirection::VerticalToHorizontal)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * AspectRatio()));
		}
		if (conversion == FovConversionDirection::HorizontalToVertical)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / AspectRatio()));
		}
		auto const errMsg{"Invalid FOV conversion direction."};
		internal::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}
}
