#include "Camera.hpp"

#include "../entity/Entity.hpp"
#include "../events/WindowEvent.hpp"
#include "../windowing/WindowImpl.hpp"

#include <utility>


namespace leopph
{
	auto Camera::Current() -> Camera*
	{
		return s_Current;
	}


	auto Camera::MakeCurrent() -> void
	{
		if (IsActive())
		{
			s_Current = this;
		}
	}


	auto Camera::NearClipPlane(float const newPlane) -> void
	{
		m_NearClip = newPlane;
	}


	auto Camera::NearClipPlane() const -> float
	{
		return m_NearClip;
	}


	auto Camera::FarClipPlane(float const newPlane) -> void
	{
		m_FarClip = newPlane;
	}


	auto Camera::FarClipPlane() const -> float
	{
		return m_FarClip;
	}


	auto Camera::Background() const -> std::variant<Color, Skybox> const&
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


	auto Camera::ViewMatrix() const -> Matrix4
	{
		// inv(T) * inv(R)
		return Matrix4::Translate(-Entity()->Transform()->Position()) * Matrix4{Entity()->Transform()->Rotation()}.Transposed();
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


	auto Camera::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		Component::Detach();

		if (s_Current == this)
		{
			s_Current = nullptr;
		}
	}


	Camera::~Camera()
	{
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


	auto Camera::AspectRatio() const -> float
	{
		return m_AspectRatio;
	}


	auto Camera::OnEventReceived(EventParamType event) -> void
	{
		m_AspectRatio = static_cast<float>(event.Width) / event.Height;
	}


	Camera* Camera::s_Current;
}
