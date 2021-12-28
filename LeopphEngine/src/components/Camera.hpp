#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"
#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "../misc/Color.hpp"
#include "../rendering/Skybox.hpp"

#include <variant>


namespace leopph
{
	// Cameras are special Components that define the image that gets rendered.
	class Camera final : public Component, public EventReceiver<internal::ScreenResolutionEvent>
	{
		public:
			// The currently active camera that is used to render the scene.
			LEOPPHAPI static auto Active() -> Camera*;

			// Set the near clip plane distance.
			// The near clip plane is the plane closest to the Camera, where rendering begins.
			// Objects closer to the Camera than this value will not be visible.
			LEOPPHAPI auto NearClipPlane(float newPlane) -> void;

			// Get the near clip plane distance.
			// The near clip plane is the plane closest to the Camera, where rendering begins.
			// Objects closer to the Camera than this value will not be visible.
			LEOPPHAPI auto NearClipPlane() const -> float;

			// Set the far clip plane distance.
			// The far clip plane is the plane farthest from the Camera, where rendering ends.
			// Objects farther from the Camera than this value will not be visible.
			LEOPPHAPI auto FarClipPlane(float newPlane) -> void;

			// Get the far clip plane distance.
			// The far clip plane is the plane farthest from the Camera, where rendering ends.
			// Objects farther from the Camera than this value will not be visible.
			LEOPPHAPI auto FarClipPlane() const -> float;

			// When adjusting FOV, this is used to specify which direction
			// the FOV value should be interpreted in for non-symmetric viewing volumes.
			enum class FovDirection
			{
				Horizontal,
				Vertical
			};


			// Set the current FOV value of the Camera in degrees.
			// For non-symmetric viewing volumes, direction specifies the interpretation.
			LEOPPHAPI auto Fov(float degrees, FovDirection direction) -> void;

			// Get the current FOV value of the Camera in degrees.
			// For non-symmetric viewing volumes, direction specifies the interpretation.
			LEOPPHAPI auto Fov(FovDirection direction) const -> float;

			// Matrix that translates world positions to Camera-space.
			// Used during rendering.
			LEOPPHAPI auto ViewMatrix() const -> Matrix4;

			// Matrix that projects Camera-space coordinates to Clip-space.
			// Used during rendering.
			LEOPPHAPI auto ProjectionMatrix() const -> Matrix4;

			// Set this Camera as the currently active one.
			// The active Camera will be used to render scene.
			// If no Camera instance exists, a newly created one will automatically be activated.
			LEOPPHAPI auto Activate() -> void;

			// Get the Camera's background.
			// The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to.
			LEOPPHAPI auto Background() const -> const std::variant<Color, Skybox>&;

			// Set the Camera's background.
			// The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to.
			LEOPPHAPI auto Background(std::variant<Color, Skybox> background) -> void;

			LEOPPHAPI explicit Camera(leopph::Entity* entity);

			Camera(const Camera&) = delete;
			auto operator=(const Camera&) -> void = delete;

			Camera(Camera&&) = delete;
			auto operator=(Camera&&) -> void = delete;

			LEOPPHAPI ~Camera() override;

		private:
			enum class FovConversionDirection
			{
				VerticalToHorizontal,
				HorizontalToVertical
			};


			[[nodiscard]] auto ConvertFov(float fov, FovConversionDirection conversion) const -> float;
			auto OnEventReceived(EventParamType event) -> void override;

			static Camera* s_Active;

			float m_AspectRatio;
			float m_HorizontalFovDegrees{100.f};
			float m_NearClip{0.1f};
			float m_FarClip{100.f};

			std::variant<Color, Skybox> m_Background;
	};
}
