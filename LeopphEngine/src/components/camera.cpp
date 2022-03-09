#include "Camera.hpp"

#include "../entity/Entity.hpp"
#include "../events/WindowEvent.hpp"
#include "../math/LeopphMath.hpp"
#include "../util/Logger.hpp"
#include "../windowing/WindowImpl.hpp"

#include <stdexcept>
#include <utility>


namespace leopph
{
	Camera* Camera::s_Current;


	auto Camera::MakeCurrent() -> void
	{
		if (IsActive())
		{
			s_Current = this;
		}
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
			static_cast<internal::WindowImpl*>(Window::Instance())->ClearColor(static_cast<Vector4>(static_cast<Vector3>(std::get<Color>(m_Background))));
		}
	}


	auto Camera::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Component::Deactivate();

		if (s_Current == this)
		{
			s_Current = nullptr;
		}
	}


	Camera::Camera() :
		m_AspectRatio{Window::Instance()->AspectRatio()},
		m_Background{Color{static_cast<Vector3>(static_cast<internal::WindowImpl*>(Window::Instance())->ClearColor())}}
	{
		if (s_Current == nullptr)
		{
			MakeCurrent();
		}
	}


	Camera::~Camera()
	{
		if (s_Current == this)
		{
			s_Current = nullptr;
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


	auto Camera::Current() -> Camera*
	{
		return s_Current;
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
		// inv(T) * inv(R)
		return Matrix4::Translate(-Entity()->Transform()->Position()) * Matrix4{Entity()->Transform()->Rotation()}.Transposed();
	}


	auto Camera::ProjectionMatrix() const -> Matrix4
	{
		const auto fov{math::ToRadians(ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical))};
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}


	auto Camera::OnEventReceived(EventParamType event) -> void
	{
		m_AspectRatio = event.Resolution[0] / event.Resolution[1];
	}
}
