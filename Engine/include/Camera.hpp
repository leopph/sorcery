#pragma once

#include "Color.hpp"
#include "Component.hpp"
#include "EventReceiver.hpp"
#include "Frustum.hpp"
#include "Matrix.hpp"
#include "Skybox.hpp"
#include "WindowEvent.hpp"

#include <variant>


namespace leopph
{
	// Cameras are special Components that define the image that gets rendered.
	class Camera : public Component, public EventReceiver<internal::WindowEvent>
	{
		public:
			// Used for specifying parameters that affect the shape of the camera.
			enum class Side
			{
				Vertical,
				Horizontal
			};


			// The current camera that is used to render the scene.
			[[nodiscard]] LEOPPHAPI static Camera* Current();

			// Set this Camera to be the current one.
			// The current Camera is used to render scene.
			// If no Camera instance exists, a newly created one will automatically be made current.
			// Only active Cameras can be made current.
			LEOPPHAPI void MakeCurrent();

			// Set the near clip plane distance.
			// The near clip plane is the plane closest to the Camera, where rendering begins.
			// Objects closer to the Camera than this value will not be visible.
			LEOPPHAPI void NearClipPlane(float newPlane);

			// Get the near clip plane distance.
			// The near clip plane is the plane closest to the Camera, where rendering begins.
			// Objects closer to the Camera than this value will not be visible.
			[[nodiscard]] LEOPPHAPI float NearClipPlane() const;

			// Set the far clip plane distance.
			// The far clip plane is the plane farthest from the Camera, where rendering ends.
			// Objects farther from the Camera than this value will not be visible.
			LEOPPHAPI void FarClipPlane(float newPlane);

			// Get the far clip plane distance.
			// The far clip plane is the plane farthest from the Camera, where rendering ends.
			// Objects farther from the Camera than this value will not be visible.
			[[nodiscard]] LEOPPHAPI float FarClipPlane() const;

			// Get the Camera's background.
			// The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to.
			[[nodiscard]] LEOPPHAPI std::variant<Color, Skybox> const& Background() const;

			// Set the Camera's background.
			// The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to.
			LEOPPHAPI void Background(std::variant<Color, Skybox> background);

			// Matrix that translates world positions to Camera-space.
			// Used during rendering.
			[[nodiscard]] LEOPPHAPI Matrix4 ViewMatrix() const;

			// Matrix that projects Camera-space coordinates to Clip-space.
			// Used during rendering.
			[[nodiscard]] virtual Matrix4 ProjectionMatrix() const = 0;

			// The current frustum of the Camera in view space.
			// Used for internal calculations.
			[[nodiscard]] virtual Frustum Frustum() const = 0;

			// Transforms the passed vector (interpreted as a point) to viewport space.
			// Viewport space ranges from [0, 1] on both axes. Values outside of that are not visible.
			[[nodiscard]] LEOPPHAPI Vector2 TransformToViewport(Vector3 const& vector) const noexcept;

			// Detaching the current Camera will set it to nullptr.
			LEOPPHAPI void Owner(Entity* entity) final;
			using Component::Owner;

			LEOPPHAPI void Active(bool active) final;
			using Component::Active;

			Camera(Camera&&) = delete;
			void operator=(Camera&&) = delete;

			LEOPPHAPI ~Camera() override;

		protected:
			LEOPPHAPI Camera();

			Camera(Camera const&) = default;
			Camera& operator=(Camera const&) = default;

			[[nodiscard]]
			float AspectRatio() const;

		private:
			LEOPPHAPI void OnEventReceived(EventParamType event) override;

			static Camera* s_Current;

			float m_AspectRatio;
			float m_NearClip{0.1f};
			float m_FarClip{100.f};
			std::variant<Color, Skybox> m_Background;
	};
}
