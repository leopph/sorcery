#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"
#include "../math/Matrix.hpp"
#include "../math/Quaternion.hpp"
#include "../math/Vector.hpp"

#include <cstddef>
#include <utility>
#include <vector>


namespace leopph
{
	// The Space enum helps determine how transformation information should be interpreted.
	enum class Space
	{
		World,
		Local
	};


	// Transforms are special Components that are automatically created on new Entities.
	// Transforms provide Entities with spatial properties such as Position, orientation and scale.
	// They define a hierarchy in which each child inherits its parent's properties.
	class Transform final : public Component
	{
		public:
			// Get the World Space Position.
			// If parent is null, this has the same value as Transform::LocalPosition.
			[[nodiscard]] constexpr auto Position() const noexcept -> auto&;

			// Get the Local Space Position relative to the parent.
			// If parent is null, this has the same value as Transform::Position.
			[[nodiscard]] constexpr auto LocalPosition() const noexcept -> auto&;

			// Set the World Space Position.
			// This adjusts the local Position so that the Transform has the desired World Space Position.
			LEOPPHAPI auto Position(const Vector3& newPos) -> void;

			// Set the Local Space Position relative to the parent.
			// If parent is null, this has the same effect as Transform::Position.
			LEOPPHAPI auto LocalPosition(const Vector3& newPos) -> void;

			// Get the World Space orientation.
			// If parent is null, this has the same value as Transform::LocalRotation.
			[[nodiscard]] constexpr auto Rotation() const noexcept -> auto&;

			// Get the Local Space orientation relative to the parent.
			// If parent is null, this has the same value as Transform::Rotation.
			[[nodiscard]] constexpr auto LocalRotation() const noexcept -> auto&;

			// Set the World Space orientation.
			// This adjusts the local rotation so that the Transform has the desired World Space orientation.
			LEOPPHAPI auto Rotation(const Quaternion& newRot) -> void;

			// Set the Local Space orientation.
			// If parent is null, this has the same effect as Transform::Rotation.
			LEOPPHAPI auto LocalRotation(const Quaternion& newRot) -> void;

			// Get the World Space scale.
			// If parent is null, this has the same value as Transform::LocalScale.
			[[nodiscard]] constexpr auto Scale() const noexcept -> auto&;

			// Get the Local Space scale relative to the parent.
			// If parent is null, this has the same value as Transform::Scale.
			[[nodiscard]] constexpr auto LocalScale() const noexcept -> auto&;

			// Set the World Space scale.
			// This adjusts local scale so that the Transform has the desired World Space scale.
			LEOPPHAPI auto Scale(const Vector3& newScale) -> void;

			// Set the Local Space scale.
			// If parent is null, this has the same effect as Transform::Scale.
			LEOPPHAPI auto LocalScale(const Vector3& newScale) -> void;

			// Move the Transform relative to its current Position by the input Vector.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local Position so that the World Space delta matches the desired value.
			LEOPPHAPI auto Translate(const Vector3& vector, Space base = Space::World) -> void;

			// Move the Transform relative to its current Position by the input values.
			// The fourth parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local Position so that the World Space delta matches the desired value.
			LEOPPHAPI auto Translate(float x, float y, float z, Space base = Space::World) -> void;

			// Rotate the Transform from its current orientation by the input Quaternion.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local rotation so that the World Space delta matches the desired value.
			LEOPPHAPI auto Rotate(const Quaternion& rotation, Space base = Space::World) -> void;

			// Rotate the Transform from its current orientation on the specified axis by the specified amount.
			// The third parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local rotation so that the World Space delta matches the desired value.
			// The rotation amount is in degrees.
			LEOPPHAPI auto Rotate(const Vector3& axis, float amountDegrees, Space base = Space::World) -> void;

			// Scale the Transform relative to its current scaling by the input Vector.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local scale so that the World Space delta matches the desired value.
			LEOPPHAPI auto Rescale(const Vector3& scaling, Space base = Space::World) -> void;

			// Scale the Transform relative to its current scaling by the input values.
			// The fourth parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local scale so that the World Space delta matches the desired value.
			LEOPPHAPI auto Rescale(float x, float y, float z, Space base = Space::World) -> void;

			// Get the Transform's current forward, Z vector in World Space.
			[[nodiscard]] constexpr auto Forward() const noexcept -> const Vector3&;

			// Get the Transform's current right, X vector in World Space.
			[[nodiscard]] constexpr auto Right() const noexcept -> const Vector3&;

			// Get the Transform's current up, Y vector in World Space.
			[[nodiscard]] constexpr auto Up() const noexcept -> const Vector3&;

			// Get the Transform's parent.
			[[nodiscard]] constexpr auto Parent() const noexcept;

			// Set the Transform's parent using pointer-to-entity.
			LEOPPHAPI auto Parent(const leopph::Entity* parent) -> void;

			// Set the Transform's parent using reference-of-entity.
			LEOPPHAPI auto Parent(const leopph::Entity& parent) -> void;

			// Set the Transform's parent using pointer-to-transform.
			LEOPPHAPI auto Parent(Transform* parent) -> void;

			// Set the Transform's parent using reference-of-transform.
			LEOPPHAPI auto Parent(Transform& parent) -> void;

			// Unchild the Transform
			LEOPPHAPI auto Parent(std::nullptr_t null) -> void;

			// Get a collection of pointers to all of the Transform's children.
			// The elements are guaranteed to be non-null.
			[[nodiscard]] constexpr auto Children() const noexcept -> auto&;

			// Returns the Matrices used by LeopphEngine during rendering.
			// Calling this may cause a recursive recalculation of matrices on all descendants of the Transform.
			// Calling this in client code is not recommended.
			LEOPPHAPI auto Matrices() const -> const std::pair<Matrix4, Matrix4>&;

			// Transforms cannot be activated, nor deactivated.
			auto Activate() -> void override;
			// Transforms cannot be activated, nor deactivated.
			auto Deactivate() -> void override;

			LEOPPHAPI explicit Transform(leopph::Entity* entity, const Vector3& pos = Vector3{}, const Quaternion& rot = Quaternion{}, const Vector3& scale = Vector3{1, 1, 1});

			Transform(const Transform&) = delete;
			auto operator=(const Transform&) -> void = delete;

			Transform(Transform&&) = delete;
			auto operator=(Transform&&) -> void = delete;

			LEOPPHAPI ~Transform() override;

		private:
			Vector3 m_WorldPosition;
			Quaternion m_WorldRotation;
			Vector3 m_WorldScale;

			Vector3 m_LocalPosition;
			Quaternion m_LocalRotation;
			Vector3 m_LocalScale;

			Vector3 m_Forward;
			Vector3 m_Right;
			Vector3 m_Up;

			Transform* m_Parent;
			std::vector<Transform*> m_Children;

			mutable bool m_Changed;

			// First is model, second is normal.
			mutable std::pair<Matrix4, Matrix4> m_Matrices;

			// Calculates local bases using the global rotation.
			auto CalculateLocalAxes() -> void;

			// Calculates global Position, sets Changed, then updates children recursively.
			auto CalculateWorldPosition() -> void;
			// Calculates global rotation, calculates local axes, sets Changed, then updates children recursively.
			auto CalculateWorldRotation() -> void;
			// Calculates global scale, then updates children recursively.
			auto CalculateWorldScale() -> void;
	};


	constexpr auto Transform::Position() const noexcept -> auto&
	{
		return m_WorldPosition;
	}


	constexpr auto Transform::LocalPosition() const noexcept -> auto&
	{
		return m_LocalPosition;
	}


	constexpr auto Transform::Rotation() const noexcept -> auto&
	{
		return m_WorldRotation;
	}


	constexpr auto Transform::LocalRotation() const noexcept -> auto&
	{
		return m_LocalRotation;
	}


	constexpr auto Transform::Scale() const noexcept -> auto&
	{
		return m_WorldScale;
	}


	constexpr auto Transform::LocalScale() const noexcept -> auto&
	{
		return m_LocalScale;
	}


	constexpr auto Transform::Forward() const noexcept -> const Vector3&
	{
		return m_Forward;
	}


	constexpr auto Transform::Right() const noexcept -> const Vector3&
	{
		return m_Right;
	}


	constexpr auto Transform::Up() const noexcept -> const Vector3&
	{
		return m_Up;
	}


	constexpr auto Transform::Parent() const noexcept
	{
		return m_Parent;
	}


	constexpr auto Transform::Children() const noexcept -> auto&
	{
		return m_Children;
	}
}
