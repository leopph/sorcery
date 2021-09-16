#pragma once

#include "Component.hpp"
#include "../api/leopphapi.h"
#include "../events/DisplayResolutionChangedEvent.hpp"
#include "../events/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "../misc/camerabackground.h"



namespace leopph
{
	class Entity;

	/*-------------------------------------------------------------------------------------------------------------
	The Camera component is used to determine the image to be rendered.
	Attach this to your Entities and switch between them to provide different viewing angles dynamically at runtime.
	See "Entity.hpp" and "Component.hpp" for more information.
	-------------------------------------------------------------------------------------------------------------*/
	class Camera final : public Component, public EventReceiver<impl::DisplayResolutionChangedEvent>
	{
		public:
			/* Get the current active camera that is used to render the scene */
			LEOPPHAPI static Camera* Active();

			LEOPPHAPI explicit Camera(Entity& owner);
			LEOPPHAPI ~Camera() override;

			Camera(const Camera&) = delete;
			Camera(Camera&&) = delete;
			void operator=(const Camera&) = delete;
			void operator=(Camera&&) = delete;



			/* Enum to help determining FOV conversion details */
			enum class FovDirection { Horizontal, Vertical };



			/* The plane closest to the Camera where rendering begins */
			LEOPPHAPI void NearClipPlane(float newPlane);
			LEOPPHAPI float NearClipPlane() const;

			/* The plane farthest from the Camera where rendering ends */
			LEOPPHAPI void FarClipPlane(float newPlane);
			LEOPPHAPI float FarClipPlane() const;

			/* Current Field of View of the Camera. */
			LEOPPHAPI void Fov(float fov, FovDirection direction);
			LEOPPHAPI float Fov(FovDirection direction) const;

			/* Internally used matrices for the rendering engine */
			LEOPPHAPI Matrix4 ViewMatrix() const;
			LEOPPHAPI Matrix4 ProjectionMatrix() const;

			/* Set this Camera to be the used for rendering */
			LEOPPHAPI void Activate();

			/* Background graphics used by the Camera.
			You can use a combination of Colors and Skyboxes. */
			LEOPPHAPI const CameraBackground& Background() const;
			LEOPPHAPI void Background(CameraBackground&& background);


		private:
			enum class FovConversionDirection
			{
				VerticalToHorizontal, HorizontalToVertical
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
