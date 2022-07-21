#include "Camera.hpp"

#include "Entity.hpp"
#include "InternalContext.hpp"
#include "WindowEvent.hpp"
#include "windowing/WindowImpl.hpp"

#include <utility>


namespace leopph
{
	auto Camera::Current() -> Camera*
	{
		return s_Current;
	}


	auto Camera::MakeCurrent() -> void
	{
		if (Active())
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
			internal::GetWindowImpl()->ClearColor(static_cast<Vector4>(static_cast<Vector3>(std::get<Color>(m_Background))));
		}
	}


	auto Camera::ViewMatrix() const -> Matrix4
	{
		// inv(T) * inv(R)
		return Matrix4::Translate(-Owner()->get_transform().get_position()) * Matrix4{Owner()->get_transform().get_rotation()}.Transposed();
	}


	auto Camera::TransformToViewport(Vector3 const& vector) const noexcept -> Vector2
	{
		Vector4 homoPos{vector, 1};
		homoPos *= ViewMatrix() * ProjectionMatrix();
		Vector2 ret{homoPos};
		ret /= homoPos[3];
		ret *= 0.5f;
		ret += Vector2{0.5f};
		return ret;
	}


	auto Camera::Owner(Entity* entity) -> void
	{
		Component::Owner(entity);

		if (!InUse() && s_Current == this)
		{
			s_Current = nullptr;
		}
	}


	auto Camera::Active(bool const active) -> void
	{
		Component::Active(active);

		if (!InUse() && s_Current == this)
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
		m_AspectRatio{internal::GetWindowImpl()->AspectRatio()},
		m_Background{Color{static_cast<Vector3>(internal::GetWindowImpl()->ClearColor())}}
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
