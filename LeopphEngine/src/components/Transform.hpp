#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"
#include "../events/FrameEndEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Quaternion.hpp"
#include "../math/Vector.hpp"

#include <cstddef>
#include <unordered_set>


namespace leopph
{
	/// The Space enum helps determine how transformation information should be interpreted.
	enum class Space
	{
		World,
		Local
	};


	/* Transforms are special Components that are automatically created on new Entities.
	 * Transforms provide Entities with spatial properties such as position, orientation and scale.
	 * They define a hierarchy in which each child inherits its parent's properties. */
	class Transform final : public Component, public EventReceiver<impl::FrameEndedEvent>
	{
	public:
		// Get the absolute spatial position.
		[[nodiscard]]
		LEOPPHAPI const Vector3& Position() const;

		/* Get the spatial position relative to the parent.
		 * If parent is null, this has the same value as Position(). */
		[[nodiscard]]
		LEOPPHAPI const Vector3& LocalPosition() const;

		/* Set the spatial position relative to the parent.
		 * If parent is null, this will also be the absolute position. */
		LEOPPHAPI void Position(const Vector3& newPos);

		// Get the absolute spatial orientation.
		[[nodiscard]]
		LEOPPHAPI const Quaternion& Rotation() const;

		/* Get the spatial orientation relative to the parent.
		 * If parent is null, this has the same value as Rotation(). */
		[[nodiscard]]
		LEOPPHAPI const Quaternion& LocalRotation() const;

		/* Set the spatial orientation relative to the parent.
		 * If parent is null, this will also be the absolute rotation. */
		LEOPPHAPI void Rotation(Quaternion newRot);

		// Get the absolute spatial scale.
		[[nodiscard]]
		LEOPPHAPI const Vector3& Scale() const;

		/* Get the spatial scale relative to the parent.
		 * If parent is null, this has the same value as Scale(). */
		[[nodiscard]]
		LEOPPHAPI const Vector3& LocalScale() const;

		/* Set the spatial scale relative to the parent.
		 * If parent is null, this will also be the absolute scale. */
		LEOPPHAPI void Scale(const Vector3& newScale);

		/* Move the Transform relative to its current position by the input vector.
		 * The vector is interpreted to be in World Space. */
		LEOPPHAPI void Translate(const Vector3& vector);

		/* Move the Transform relative to its current position by the input values.
		 * The input values are interpreted to be in World Space. */
		LEOPPHAPI void Translate(float x, float y, float z);

		/* Rotate the Transform from its current rotation specified by the input Quaternion.
		 * The rotation will be interpreted in Local-space, so X, Y, and Z will mean the Transforms respective axes. */
		LEOPPHAPI void RotateLocal(const Quaternion& rotation);

		/* Rotate the Transform from its current rotation specified by the input Quaternion.
		 * The rotation will be interpreted in World-space, so X, Y, and Z will mean the world's respective axes. */
		LEOPPHAPI void RotateGlobal(const Quaternion& rotation);

		// Scale the Transform relative to its current local scaling by the input values.
		LEOPPHAPI void Rescale(float x, float y, float z);

		// Get the Transform's current forward, or Z vector.
		[[nodiscard]]
		LEOPPHAPI auto Forward() const -> const Vector3&;

		// Get the Transform's current right, or X vector.
		[[nodiscard]]
		LEOPPHAPI auto Right() const -> const Vector3&;

		// Get the Transform's current up, or Y vector.
		[[nodiscard]]
		LEOPPHAPI auto Up() const -> const Vector3&;

		/* A flag representing whether the Transform's matrices have changed since their last calculations.
		 * If this flag is set, new matrix calculations will take place during the rendering of the current frame. */
		const bool& WasAltered;

		// Get the Transform's parent.
		[[nodiscard]]
		LEOPPHAPI Transform* Parent() const;

		// Set the Transform's parent using pointer-to-entity.
		LEOPPHAPI void Parent(leopph::Entity* parent);

		// Set the Transform's parent using reference-of-entity.
		LEOPPHAPI void Parent(leopph::Entity& parent);

		// Set the Transform's parent using pointer-to-transform.
		LEOPPHAPI void Parent(Transform* parent);

		// Set the Transform's parent using reference-of-transform.
		LEOPPHAPI void Parent(Transform& parent);
		
		// Unchild the Transform
		LEOPPHAPI void Parent(std::nullptr_t null);

		/* Get a collection of pointers to all of the Transform's children.
		 * The elements are guaranteed to be non-null. */
		LEOPPHAPI const std::unordered_set<Transform*>& Children() const;

		/* Force calculate the Transform's matrices.
		 * LeopphEngine automatically calles this right before rendering, so there is usually no reason to do so explicitly.
		 * If the matrices are up-to-date, this function is NOP.
		 * If new matrices were calculated, WasAltered is set to false.
		 * If any ancestor's matrices are out of date, those, and all its descendants' matrices will be recalculated. */
		LEOPPHAPI void CalculateMatrices();


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
		Vector3 m_GlobalPosition;
		Quaternion m_GlobalRotation;
		Vector3 m_GlobalScale;

		Vector3 m_LocalPosition;
		Quaternion m_LocalRotation;
		Vector3 m_LocalScale;

		Vector3 m_Forward;
		Vector3 m_Right;
		Vector3 m_Up;

		bool m_WasAltered;

		Transform* m_Parent;
		std::unordered_set<Transform*> m_Children;

		void CalculateLocalAxes();
		void OnEventReceived(const impl::FrameEndedEvent&) override;

		// Calculates global position, sets WasAltered, then updates children recursively.
		void CalculateGlobalPosition();
		// Calculates global rotation, calculates local axes, sets WasAltered, then updates children recursively.
		void CalculateGlobalRotation();
		// Calculates global scale, then updates children recursively.
		void CalculateGlobalScale();
	};
}