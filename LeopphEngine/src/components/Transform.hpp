#pragma once

#include "../api/leopphapi.h"
#include "../math/Vector.hpp"
#include "../math/Quaternion.hpp"
#include "../events/EventReceiver.hpp"
#include "../events/FrameEndedEvent.hpp"
#include "Component.hpp"

namespace leopph
{
	class Object;


	class Transform final : public Component, public EventReceiver<impl::FrameEndedEvent>
	{
	public:
		LEOPPHAPI explicit Transform(Object& owner,
									 const Vector3& pos = Vector3{},
									 const Quaternion& rot = Quaternion{},
									 const Vector3& scale = Vector3{1, 1, 1});

		Transform(const Transform&) = delete;
		Transform(Transform&&) = delete;
		void operator=(const Transform&) = delete;
		void operator=(Transform&&) = delete;

		LEOPPHAPI ~Transform() override;

		/* Spatial position in 3D space */
		[[nodiscard]] LEOPPHAPI const Vector3& Position() const;
		LEOPPHAPI void Position(const Vector3& newPos);

		/* Spatial rotation */
		[[nodiscard]] LEOPPHAPI const Quaternion& Rotation() const;
		LEOPPHAPI void Rotation(Quaternion newRot);

		/* Spatial scaling */
		[[nodiscard]] LEOPPHAPI const Vector3& Scale() const;
		LEOPPHAPI void Scale(const Vector3& newScale);

		/* Move the DynamicTransform relative to its current position */
		LEOPPHAPI void Translate(const Vector3& vector);
		LEOPPHAPI void Translate(float x, float y, float z);

		/* Rotate the DynamicTransform around the specified axis */
		LEOPPHAPI void RotateLocal(const Quaternion& rotation);
		LEOPPHAPI void RotateGlobal(const Quaternion& rotation);

		/* Scale the DynamicTransform relative to its current scaling */
		LEOPPHAPI void Rescale(float x, float y, float z);

		/* Current local axes */
		[[nodiscard]] LEOPPHAPI auto Forward() const -> const Vector3&;
		[[nodiscard]] LEOPPHAPI auto Right() const -> const Vector3&;
		[[nodiscard]] LEOPPHAPI auto Up() const -> const Vector3&;


		/* This flag is set true if any property of the Transform
		 * was altered since the last time it's matrix was calculatted.
		 * This is mostly used internally but can come in handy in your
		 * logic too. */
		const bool& WasAltered;


	private:
		Vector3 m_Position;
		Quaternion m_Rotation;
		Vector3 m_Scale;

		Vector3 m_Forward;
		Vector3 m_Right;
		Vector3 m_Up;

		bool m_WasAltered;

		void CalculateLocalAxes();
		void OnEventReceived(const impl::FrameEndedEvent&) override;
	};
}