#pragma once

#include "leopphapi.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "component.h"

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)



	// CLASS REPRESENTING A CONTROLLABLE CAMERA
	class LEOPPHAPI Camera : public Component
	{
	public:
		static Camera* Active();

		Camera(leopph::Object& object);
		~Camera();

		enum : unsigned char { FOV_HORIZONTAL, FOV_VERTICAL };
		enum class Movement { FORWARD, BACKWARD, LEFT, RIGHT, DOWN, UP };

		void AspectRatio(float newRatio);
		void AspectRatio(int width, int height);
		float AspectRatio() const;

		void NearClipPlane(float newPlane);
		float NearClipPlane() const;

		void FarClipPlane(float newPlane);
		float FarClipPlane() const;

		void FOV(float fov, unsigned char direction);
		float FOV(unsigned char direction);

		Matrix4 ViewMatrix() const;
		Matrix4 ProjectionMatrix() const;

		void Activate();



	private:
		static Camera* s_Active;

		float m_AspectRatio;
		float m_HorizontalFOVDegrees;
		float m_NearClip;
		float m_FarClip;

		enum : unsigned char { VERTICAL_TO_HORIZONTAL, HORIZONTAL_TO_VERTICAL };
		float ConvertFOV(float fov, unsigned char conversion) const;
	};



#pragma warning(pop)
}