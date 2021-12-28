#include "Camera.hpp"

#include "../entity/Entity.hpp"
#include "../events/ScreenResolutionEvent.hpp"
#include "../math/LeopphMath.hpp"
#include "../util/logger.h"
#include "../windowing/WindowBase.hpp"

#include <stdexcept>
#include <utility>


namespace leopph
{
	Camera* Camera::s_Active;


	auto Camera::Activate() -> void
	{
		s_Active = this;
	}


	auto Camera::Background() const -> const std::variant<Color, Skybox>&
	{
		return m_Background;
	}


	auto Camera::Background(std::variant<Color, Skybox> background) -> void
	{
		m_Background = std::move(background);

		if (std::holds_alternative<Color>(m_Background))
		{
			internal::WindowBase::Get().ClearColor(static_cast<Vector4>(static_cast<Vector3>(std::get<Color>(m_Background))));
		}
	}


	Camera::Camera(leopph::Entity* const entity) :
		Component{entity},
		m_AspectRatio{internal::WindowBase::Get().AspectRatio()},
		m_Background{Color{static_cast<Vector3>(internal::WindowBase::Get().ClearColor())}}
	{
		if (s_Active == nullptr)
		{
			Activate();
		}
	}


	Camera::~Camera()
	{
		if (s_Active == this)
		{
			s_Active = nullptr;
		}
	}


	auto Camera::ConvertFov(const float fov, const FovConversionDirection conversion) const -> float
	{
		if (conversion == FovConversionDirection::VerticalToHorizontal)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * m_AspectRatio));
		}
		if (conversion == FovConversionDirection::HorizontalToVertical)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / m_AspectRatio));
		}
		const auto errMsg{"Invalid FOV conversion direction."};
		internal::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}


	auto Camera::Active() -> Camera*
	{
		return s_Active;
	}


	auto Camera::NearClipPlane(const float newPlane) -> void
	{
		m_NearClip = newPlane;
	}


	auto Camera::NearClipPlane() const -> float
	{
		return m_NearClip;
	}


	auto Camera::FarClipPlane(const float newPlane) -> void
	{
		m_FarClip = newPlane;
	}


	auto Camera::FarClipPlane() const -> float
	{
		return m_FarClip;
	}


	auto Camera::Fov(const float degrees, const FovDirection direction) -> void
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


	auto Camera::Fov(const FovDirection direction) const -> float
	{
		if (direction == FovDirection::Horizontal)
		{
			return m_HorizontalFovDegrees;
		}
		if (direction == FovDirection::Vertical)
		{
			return ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical);
		}
		const auto errMsg{"Invalid FOV direction."};
		internal::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}


	auto Camera::ViewMatrix() const -> Matrix4
	{
		return (static_cast<Matrix4>(Entity()->Transform()->Rotation()) * Matrix4::Translate(Entity()->Transform()->Position())).Inverse();
	}


	auto Camera::ProjectionMatrix() const -> Matrix4
	{
		const auto fov{math::ToRadians(ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical))};
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}


	auto Camera::OnEventReceived(EventParamType event) -> void
	{
		m_AspectRatio = event.NewResolution[0] / event.NewResolution[1];
	}
}
