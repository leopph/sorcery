#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"
#include "../events/FrameEndEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Quaternion.hpp"
#include "../math/Vector.hpp"


namespace leopph
{
	/* Transforms are special Components that are automatically created on new Entities.
	 * Transforms provide Entities with spatial properties such as position, orientation and scale. */
	class Transform final : public Component, public EventReceiver<impl::FrameEndedEvent>
	{
	public:
		// Get the spatial position in 3D space.
		[[nodiscard]] LEOPPHAPI const Vector3& Position() const;

		// Set the spatial position in 3D space.
		LEOPPHAPI void Position(const Vector3& newPos);

		// Get the spatial orientation in 3D space.
		[[nodiscard]] LEOPPHAPI const Quaternion& Rotation() const;

		// Set the spatial orientation in 3D space.
		LEOPPHAPI void Rotation(Quaternion newRot);

		// Get the spatial scale in 3D space.
		[[nodiscard]] LEOPPHAPI const Vector3& Scale() const;

		// Set the spatial scale in 3D space.
		LEOPPHAPI void Scale(const Vector3& newScale);

		// Move the Transform relative to its current position by the input vector.
		LEOPPHAPI void Translate(const Vector3& vector);

		// Move the Transform relative to its current position by the input values.
		LEOPPHAPI void Translate(float x, float y, float z);

		/* Rotate the Transform from its current rotation specified by the input Quaternion.
		 * The rotation will be interpreted in Local-space, so X, Y, and Z will mean the Transforms respective axes. */
		LEOPPHAPI void RotateLocal(const Quaternion& rotation);

		/* Rotate the Transform from its current rotation specified by the input Quaternion.
		 * The rotation will be interpreted in World-space, so X, Y, and Z will mean the world's respective axes. */
		LEOPPHAPI void RotateGlobal(const Quaternion& rotation);

		// Scale the Transform relative to its current scaling by the input values.
		LEOPPHAPI void Rescale(float x, float y, float z);

		// Get the Transform's current forward, or Z vector.
		[[nodiscard]] LEOPPHAPI auto Forward() const -> const Vector3&;

		// Get the Transform's current right, or X vector.
		[[nodiscard]] LEOPPHAPI auto Right() const -> const Vector3&;

		// Get the Transform's current up, or Y vector.
		[[nodiscard]] LEOPPHAPI auto Up() const -> const Vector3&;

		/* A flag representing whether the Transform's matrices have changed since their last calculations.
		 * If this flag is set, new matrix calculations will take place during the rendering of the current frame. */
		const bool& WasAltered;


		LEOPPHAPI explicit Transform(leopph::Entity& owner,
									 const Vector3& pos = Vector3{},
									 const Quaternion& rot = Quaternion{},
									 const Vector3& scale = Vector3{1, 1, 1});
		Transform(const Transform&) = delete;
		Transform(Transform&&) = delete;

		LEOPPHAPI ~Transform() override;

		void operator=(const Transform&) = delete;
		void operator=(Transform&&) = delete;


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