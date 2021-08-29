#pragma once

#include "../api/leopphapi.h"
#include "../events/EventReceiver.hpp"
#include "../events/DisplayResolutionChangedEvent.hpp"
#include "../math/Matrix.hpp"
#include "../misc/camerabackground.h"
#include "Component.hpp"

namespace leopph
{
	class Object;

	/*-------------------------------------------------------------------------------------------------------------
	The Camera component is used to determine the image to be rendered.
	Attach this to your Objects and switch between them to provide different viewing angles dynamically at runtime.
	See "object.h" and "component.h" for more information.
	-------------------------------------------------------------------------------------------------------------*/
	class Camera final : public Component, public EventReceiver<impl::DisplayResolutionChangedEvent>
	{
	public:
		/* Get the current active camera that is used to render the scene */
		LEOPPHAPI static Camera* Active();

		LEOPPHAPI explicit Camera(Object& owner);
		LEOPPHAPI ~Camera() override;

		Camera(const Camera&) = delete;
		Camera(Camera&&) = delete;
		void operator=(const Camera&) = delete;
		void operator=(Camera&&) = delete;

		/* Enum to help determining FOV conversion details */
		enum : unsigned char { FOV_HORIZONTAL, FOV_VERTICAL };

		/* The plane closest to the Camera where rendering begins */
		LEOPPHAPI void NearClipPlane(float newPlane);
		LEOPPHAPI float NearClipPlane() const;

		/* The plane farthest from the Camera where rendering ends */
		LEOPPHAPI void FarClipPlane(float newPlane);
		LEOPPHAPI float FarClipPlane() const;

		/* Current Field of View of the Camera.
		Use FOV_HORIZONTAL or FOV_VERTICAL to help interpret the provided input */
		LEOPPHAPI void FOV(float fov, unsigned char direction);
		LEOPPHAPI float FOV(unsigned char direction) const;

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
		static Camera* s_Active;

		float m_AspectRatio;
		float m_HorizontalFOVDegrees;
		float m_NearClip;
		float m_FarClip;

		CameraBackground m_Background;

		enum : unsigned char { VERTICAL_TO_HORIZONTAL, HORIZONTAL_TO_VERTICAL };
		[[nodiscard]] float ConvertFOV(float fov, unsigned char conversion) const;

		void OnEventReceived(EventParamType event) override;
	};
}