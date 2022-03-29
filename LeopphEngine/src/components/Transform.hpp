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
			[[nodiscard]] constexpr
			auto Position() const noexcept -> auto&;

			// Get the Local Space Position relative to the parent.
			// If parent is null, this has the same value as Transform::Position.
			[[nodiscard]] constexpr
			auto LocalPosition() const noexcept -> auto&;

			// Set the World Space Position.
			// This adjusts the local Position so that the Transform has the desired World Space Position.
			LEOPPHAPI
			auto Position(Vector3 const& newPos) -> void;

			// Set the Local Space Position relative to the parent.
			// If parent is null, this has the same effect as Transform::Position.
			LEOPPHAPI
			auto LocalPosition(Vector3 const& newPos) -> void;

			// Get the World Space orientation.
			// If parent is null, this has the same value as Transform::LocalRotation.
			[[nodiscard]] constexpr
			auto Rotation() const noexcept -> auto&;

			// Get the Local Space orientation relative to the parent.
			// If parent is null, this has the same value as Transform::Rotation.
			[[nodiscard]] constexpr
			auto LocalRotation() const noexcept -> auto&;

			// Set the World Space orientation.
			// This adjusts the local rotation so that the Transform has the desired World Space orientation.
			LEOPPHAPI
			auto Rotation(Quaternion const& newRot) -> void;

			// Set the Local Space orientation.
			// If parent is null, this has the same effect as Transform::Rotation.
			LEOPPHAPI
			auto LocalRotation(Quaternion const& newRot) -> void;

			// Get the World Space scale.
			// If parent is null, this has the same value as Transform::LocalScale.
			[[nodiscard]] constexpr
			auto Scale() const noexcept -> auto&;

			// Get the Local Space scale relative to the parent.
			// If parent is null, this has the same value as Transform::Scale.
			[[nodiscard]] constexpr
			auto LocalScale() const noexcept -> auto&;

			// Set the World Space scale.
			// This adjusts local scale so that the Transform has the desired World Space scale.
			LEOPPHAPI
			auto Scale(Vector3 const& newScale) -> void;

			// Set the Local Space scale.
			// If parent is null, this has the same effect as Transform::Scale.
			LEOPPHAPI
			auto LocalScale(Vector3 const& newScale) -> void;

			// Move the Transform relative to its current Position by the input Vector.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local Position so that the World Space delta matches the desired value.
			LEOPPHAPI
			auto Translate(Vector3 const& vector, Space base = Space::World) -> void;

			// Move the Transform relative to its current Position by the input values.
			// The fourth parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local Position so that the World Space delta matches the desired value.
			LEOPPHAPI
			auto Translate(float x, float y, float z, Space base = Space::World) -> void;

			// Rotate the Transform from its current orientation by the input Quaternion.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local rotation so that the World Space delta matches the desired value.
			LEOPPHAPI
			auto Rotate(Quaternion const& rotation, Space base = Space::World) -> void;

			// Rotate the Transform from its current orientation on the specified axis by the specified amount.
			// The third parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local rotation so that the World Space delta matches the desired value.
			// The rotation amount is in degrees.
			LEOPPHAPI
			auto Rotate(Vector3 const& axis, float amountDegrees, Space base = Space::World) -> void;

			// Scale the Transform relative to its current scaling by the input Vector.
			// The second parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local scale so that the World Space delta matches the desired value.
			LEOPPHAPI
			auto Rescale(Vector3 const& scaling, Space base = Space::World) -> void;

			// Scale the Transform relative to its current scaling by the input values.
			// The fourth parameter defines the Space in which the input is interpreted to be.
			// World Space values adjust the local scale so that the World Space delta matches the desired value.
			LEOPPHAPI
			auto Rescale(float x, float y, float z, Space base = Space::World) -> void;

			// Get the Transform's current forward, Z vector in World Space.
			[[nodiscard]] constexpr
			auto Forward() const noexcept -> Vector3 const&;

			// Get the Transform's current right, X vector in World Space.
			[[nodiscard]] constexpr
			auto Right() const noexcept -> Vector3 const&;

			// Get the Transform's current up, Y vector in World Space.
			[[nodiscard]] constexpr
			auto Up() const noexcept -> Vector3 const&;

			// Get the Transform's parent.
			[[nodiscard]] constexpr
			auto Parent() const noexcept;

			// Set the Transform's parent using pointer-to-entity.
			LEOPPHAPI
			auto Parent(Entity const* parent) -> void;

			// Set the Transform's parent using pointer-to-transform.
			// Parenting to an unattached Transform is ignored.
			LEOPPHAPI
			auto Parent(Transform* parent) -> void;

			// Set the Transform's parent using shared_ptr-to-transform.
			// Parenting to an unattached Transform is ignored.
			LEOPPHAPI
			auto Parent(std::shared_ptr<Transform> const& parent) -> void;

			// Unchild the Transform
			LEOPPHAPI
			auto Parent(std::nullptr_t null) -> void;

			// Get a collection of pointers to all of the Transform's children.
			// The elements are guaranteed to be non-null.
			[[nodiscard]] constexpr
			auto Children() const noexcept -> auto&;

			// Returns the Matrices used by LeopphEngine during rendering.
			// Calling this may cause a recursive recalculation of matrices on all descendants of the Transform.
			// Calling this in client code is not recommended.
			LEOPPHAPI
			auto Matrices() const -> std::pair<Matrix4, Matrix4> const&;

			// A Transform's owner cannot be changed after attaching.
			LEOPPHAPI
			auto Owner(Entity* entity) -> void override;
			using Component::Owner;

			// Transforms cannot be activated or deactivated.
			LEOPPHAPI
			auto Active(bool active) -> void override;
			using Component::Active;

			LEOPPHAPI explicit Transform(Vector3 const& pos = Vector3{},
			                             Quaternion const& rot = Quaternion{},
			                             Vector3 const& scale = Vector3{1, 1, 1});

			// Creates a new Transform that has the same local properties.
			// Doesn't copy parents or children.
			LEOPPHAPI
			Transform(Transform const& other);

			// Copies the other's local properties.
			// Doesn't copy parents or children.
			LEOPPHAPI
			auto operator=(Transform const& other) -> Transform&;

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			Transform(Transform&&) = delete;
			auto operator=(Transform&&) -> void = delete;

			// When a transform is destroyed, all its children are unparented.
			// The children keep their local transformations.
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

			Transform* m_Parent{nullptr};
			std::vector<Transform*> m_Children;

			mutable bool m_Changed{true};

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


	constexpr auto Transform::Forward() const noexcept -> Vector3 const&
	{
		return m_Forward;
	}


	constexpr auto Transform::Right() const noexcept -> Vector3 const&
	{
		return m_Right;
	}


	constexpr auto Transform::Up() const noexcept -> Vector3 const&
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
