#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"
#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "../misc/CameraBackground.hpp"


namespace leopph
{
	/* Cameras are special Components that determine the images to be rendered.
	 * Multiple Cameras can exist, but only one is used at a time.
	 * You can dynamically switch between them at runtime. */
	class Camera final : public Component, public EventReceiver<impl::ScreenResolutionEvent>
	{
		public:
			// The currently active camera that is used to render the scene.
			LEOPPHAPI static Camera* const& Active;

			/* When adjusting FOV, this is used to specify which direction
			 * the FOV value should be interpreted in for non-symmetric viewing volumes. */
			enum class FovDirection
			{
				Horizontal,
				Vertical
			};

			/* Set the near clip plane distance.
			 * The near clip plane is the plane closest to the Camera, where rendering begins.
			 * Objects closer to the Camera than this value will not be visible. */
			LEOPPHAPI void NearClipPlane(float newPlane);

			/* Get the near clip plane distance.
			 * The near clip plane is the plane closest to the Camera, where rendering begins.
			 * Objects closer to the Camera than this value will not be visible. */
			LEOPPHAPI float NearClipPlane() const;

			/* Set the far clip plane distance.
			 * The far clip plane is the plane farthest from the Camera, where rendering ends.
			 * Objects farther from the Camera than this value will not be visible. */
			LEOPPHAPI void FarClipPlane(float newPlane);

			/* Get the far clip plane distance.
			 * The far clip plane is the plane farthest from the Camera, where rendering ends.
			 * Objects farther from the Camera than this value will not be visible. */
			LEOPPHAPI float FarClipPlane() const;

			/* Set the current FOV value of the Camera in degrees.
			 * For non-symmetric viewing volumes, direction specifies the interpretation. */
			LEOPPHAPI void Fov(float degrees, FovDirection direction);

			/* Get the current FOV value of the Camera in degrees.
			 * For non-symmetric viewing volumes, direction specifies the interpretation. */
			LEOPPHAPI float Fov(FovDirection direction) const;

			/* Matrix that translates world positions to Camera-space.
			 * Used during rendering. */
			LEOPPHAPI Matrix4 ViewMatrix() const;

			/* Matrix that projects Camera-space coordinates to Clip-space.
			 * Used during rendering. */
			LEOPPHAPI Matrix4 ProjectionMatrix() const;

			/* Set this Camera as the currently active one.
			 * The active Camera will be used to render scene. */
			LEOPPHAPI void Activate();

			/* Get the Camera's background.
			 * The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to. */
			LEOPPHAPI const CameraBackground& Background() const;

			/* Set the Camera's background.
			 * The Camera's background determines the visuals that the Camera "sees" where no Objects have been drawn to. */
			LEOPPHAPI void Background(CameraBackground&& background);


			LEOPPHAPI explicit Camera(leopph::Entity& owner);
			Camera(const Camera&) = delete;
			Camera(Camera&&) = delete;

			LEOPPHAPI ~Camera() override;

			void operator=(const Camera&) = delete;
			void operator=(Camera&&) = delete;


		private:
			enum class FovConversionDirection
			{
				VerticalToHorizontal,
				HorizontalToVertical
			};

			static Camera* s_Active;

			float m_AspectRatio;
			float m_HorizontalFovDegrees;
			float m_NearClip;
			float m_FarClip;

			CameraBackground m_Background;

			[[nodiscard]] float ConvertFov(float fov, FovConversionDirection conversion) const;
			void OnEventReceived(EventParamType event) override;
	};
}
