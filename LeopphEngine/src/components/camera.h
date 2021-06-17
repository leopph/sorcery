#pragma once

#include "../api/leopphapi.h"
#include "../math/matrix.h"
#include "component.h"

namespace leopph
{
	/*-------------------------------------------------------------------------------------------------------------
	The Camera component is used to determine the image to be rendered.
	Attach this to your Objects and switch between them to provide different viewing angles dynamically at runtime.
	See "object.h" and "component.h" for more information.
	-------------------------------------------------------------------------------------------------------------*/

	class Camera final : public Component
	{
	public:
		/* Get the current active camera that is used to render the scene */
		LEOPPHAPI static Camera* Active();

		/* Internally used constructor and destructor */
		LEOPPHAPI Camera();
		LEOPPHAPI ~Camera() override;

		/* Enum to help determining FOV conversion details */
		enum : unsigned char { FOV_HORIZONTAL, FOV_VERTICAL };

		/* Aspect ratio (width / height) of the Camera */
		LEOPPHAPI void AspectRatio(float newRatio);
		LEOPPHAPI void AspectRatio(int width, int height);
		LEOPPHAPI float AspectRatio() const;

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



	private:
		static Camera* s_Active;

		float m_AspectRatio;
		float m_HorizontalFOVDegrees;
		float m_NearClip;
		float m_FarClip;

		enum : unsigned char { VERTICAL_TO_HORIZONTAL, HORIZONTAL_TO_VERTICAL };
		float ConvertFOV(float fov, unsigned char conversion) const;
	};
}