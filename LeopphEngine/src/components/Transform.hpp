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
		/* Get the World Space position.
		 * If parent is null, this has the same value as Transform::LocalPosition. */
		[[nodiscard]]
		LEOPPHAPI const Vector3& Position() const;

		/* Get the Local Space position relative to the parent.
		 * If parent is null, this has the same value as Transform::Position. */
		[[nodiscard]]
		LEOPPHAPI const Vector3& LocalPosition() const;

		/* Set the World Space position.
		 * This adjusts the local position so that the Transform has the desired World Space position. */
		LEOPPHAPI void Position(const Vector3& newPos);

		/* Set the Local Space position relative to the parent.
		 * If parent is null, this has the same effect as Transform::Position. */
		LEOPPHAPI void LocalPosition(const Vector3& newPos);

		/* Get the World Space orientation.
		 * If parent is null, this has the same value as Transform::LocalRotation. */
		[[nodiscard]]
		LEOPPHAPI const Quaternion& Rotation() const;

		/* Get the Local Space orientation relative to the parent.
		 * If parent is null, this has the same value as Transform::Rotation. */
		[[nodiscard]]
		LEOPPHAPI const Quaternion& LocalRotation() const;

		/* Set the World Space orientation.
		 * This adjusts the local rotation so that the Transform has the desired World Space orientation. */
		LEOPPHAPI void Rotation(const Quaternion& newRot);

		/* Set the Local Space orientation.
		 * If parent is null, this has the same effect as Transform::Rotation. */
		LEOPPHAPI void LocalRotation(const Quaternion& newRot);

		/* Get the World Space scale.
		 * If parent is null, this has the same value as Transform::LocalScale. */
		[[nodiscard]]
		LEOPPHAPI const Vector3& Scale() const;

		/* Get the Local Space scale relative to the parent.
		 * If parent is null, this has the same value as Transform::Scale. */
		[[nodiscard]]
		LEOPPHAPI const Vector3& LocalScale() const;

		/* Set the World Space scale.
		 * This adjusts local scale so that the Transform has the desired World Space scale. */
		LEOPPHAPI void Scale(const Vector3& newScale);

		/* Set the Local Space scale.
		 * If parent is null, this has the same effect as Transform::Scale. */
		LEOPPHAPI void LocalScale(const Vector3& newScale);

		/* Move the Transform relative to its current position by the input Vector.
		 * The second parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local position so that the World Space delta matches the desired value. */
		LEOPPHAPI void Translate(const Vector3& vector, Space base = Space::World);

		/* Move the Transform relative to its current position by the input values.
		 * The fourth parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local position so that the World Space delta matches the desired value. */
		LEOPPHAPI void Translate(float x, float y, float z, Space base = Space::World);

		/* Rotate the Transform from its current orientation by the input Quaternion.
		 * The second parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local rotation so that the World Space delta matches the desired value. */
		LEOPPHAPI void Rotate(const Quaternion& rotation, Space base = Space::World);

		/* Rotate the Transform from its current orientation on the specified axis by the specified amount.
		 * The third parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local rotation so that the World Space delta matches the desired value.
		 * The rotation amount is in degrees. */
		LEOPPHAPI void Rotate(const Vector3& axis, float amountDegrees, Space base = Space::World);

		/* Scale the Transform relative to its current scaling by the input Vector.
		 * The second parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local scale so that the World Space delta matches the desired value. */
		LEOPPHAPI void Rescale(const Vector3& scaling, Space base = Space::World);

		/* Scale the Transform relative to its current scaling by the input values.
		 * The fourth parameter defines the Space in which the input is interpreted to be.
		 * World Space values adjust the local scale so that the World Space delta matches the desired value. */
		LEOPPHAPI void Rescale(float x, float y, float z, Space base = Space::World);

		// Get the Transform's current forward, Z vector in World Space.
		[[nodiscard]]
		LEOPPHAPI const Vector3& Forward() const;

		// Get the Transform's current right, X vector in World Space.
		[[nodiscard]]
		LEOPPHAPI const Vector3& Right() const;

		// Get the Transform's current up, Y vector in World Space.
		[[nodiscard]]
		LEOPPHAPI const Vector3& Up() const;

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