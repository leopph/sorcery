#include "Transform.hpp"

#include "../data/DataManager.hpp"
#include "../entity/Entity.hpp"

#include <algorithm>


namespace leopph
{
	Transform::Transform(leopph::Entity* const entity, const Vector3& pos, const Quaternion& rot, const Vector3& scale) :
		Component{entity},
		m_WorldPosition{pos},
		m_WorldRotation{rot},
		m_WorldScale{scale},
		m_LocalPosition{pos},
		m_LocalRotation{rot},
		m_LocalScale{scale},
		m_Forward{Vector3::Forward()},
		m_Right{Vector3::Right()},
		m_Up{Vector3::Up()},
		m_Parent{nullptr},
		m_Changed{true}
	{
		CalculateLocalAxes();
	}


	Transform::~Transform()
	{
		if (m_Parent != nullptr)
		{
			std::erase(m_Parent->m_Children, this);
		}

		std::ranges::for_each(m_Children, [](const auto& child)
		{
			// Can't call parent setter, because that would remove child from this's m_Children, and the loop would crash.
			child->m_Parent = nullptr;
			child->CalculateWorldPosition();
			child->CalculateWorldRotation();
			child->CalculateWorldScale();
		});
	}


	auto Transform::Position(const Vector3& newPos) -> void
	{
		if (m_Parent != nullptr)
		{
			m_LocalPosition = m_Parent->m_WorldRotation.Conjugate().Rotate(newPos) - m_Parent->m_WorldPosition;
		}
		else
		{
			m_LocalPosition = newPos;
		}
		CalculateWorldPosition();
	}


	auto Transform::LocalPosition(const Vector3& newPos) -> void
	{
		m_LocalPosition = newPos;
		CalculateWorldPosition();
	}


	auto Transform::Rotation(const Quaternion& newRot) -> void
	{
		if (m_Parent != nullptr)
		{
			m_LocalRotation = m_Parent->m_WorldRotation.Conjugate() * newRot;
		}
		else
		{
			m_LocalRotation = newRot;
		}
		CalculateWorldRotation();
	}


	auto Transform::LocalRotation(const Quaternion& newRot) -> void
	{
		m_LocalRotation = newRot;
		CalculateWorldRotation();
	}


	auto Transform::Scale(const Vector3& newScale) -> void
	{
		if (m_Parent != nullptr)
		{
			m_LocalScale = newScale / m_Parent->m_WorldScale;
		}
		else
		{
			m_LocalScale = newScale;
		}
		CalculateWorldScale();
	}


	auto Transform::LocalScale(const Vector3& newScale) -> void
	{
		m_LocalScale = newScale;
		CalculateWorldScale();
	}


	auto Transform::Translate(const Vector3& vector, const Space base) -> void
	{
		if (base == Space::World)
		{
			Position(m_WorldPosition + vector);
		}
		else if (base == Space::Local)
		{
			LocalPosition(m_LocalPosition + (vector * static_cast<Matrix3>(static_cast<Matrix4>(m_LocalRotation))));
		}
	}


	auto Transform::Translate(const float x, const float y, const float z, const Space base) -> void
	{
		Translate(Vector3{x, y, z}, base);
	}


	auto Transform::Rotate(const Quaternion& rotation, const Space base) -> void
	{
		if (base == Space::World)
		{
			LocalRotation(rotation * m_LocalRotation);
		}
		else if (base == Space::Local)
		{
			LocalRotation(m_LocalRotation * rotation);
		}
	}


	auto Transform::Rotate(const Vector3& axis, float amountDegrees, const Space base) -> void
	{
		Rotate(Quaternion{axis, amountDegrees}, base);
	}


	auto Transform::Rescale(const Vector3& scaling, const Space base) -> void
	{
		if (base == Space::World)
		{
			Scale(m_WorldScale * scaling);
		}
		else if (base == Space::Local)
		{
			LocalScale(m_LocalScale * scaling);
		}
	}


	auto Transform::Rescale(const float x, const float y, const float z, const Space base) -> void
	{
		Rescale(Vector3{x, y, z}, base);
	}


	auto Transform::Parent(const leopph::Entity* const parent) -> void
	{
		Parent(parent->Transform());
	}


	auto Transform::Parent(const leopph::Entity& parent) -> void
	{
		Parent(parent.Transform());
	}


	auto Transform::Parent(Transform* const parent) -> void
	{
		if (m_Parent != nullptr)
		{
			std::erase(m_Parent->m_Children, this);
		}

		m_Parent = parent;
		if (parent != nullptr)
		{
			parent->m_Children.push_back(this);
		}

		CalculateWorldPosition();
		CalculateWorldRotation();
		CalculateWorldScale();
	}


	auto Transform::Parent(Transform& parent) -> void
	{
		Parent(&parent);
	}


	auto Transform::Parent(const std::nullptr_t null) -> void
	{
		Parent(static_cast<Transform*>(null));
	}


	auto Transform::Matrices() const -> const std::pair<Matrix4, Matrix4>&
	{
		if (!m_Changed)
		{
			return m_Matrices;
		}

		Matrix4 modelMatrix;
		modelMatrix[0] = Vector4{m_Right * m_WorldScale[0], 0};
		modelMatrix[1] = Vector4{m_Up * m_WorldScale[1], 0};
		modelMatrix[2] = Vector4{m_Forward * m_WorldScale[2], 0};
		modelMatrix[3] = Vector4{m_WorldPosition};

		const auto worldScaleRecip{1.f / m_WorldScale};

		Matrix4 normalMatrix;
		normalMatrix[0] = Vector4{m_Right * worldScaleRecip[0], -m_WorldPosition[0]};
		normalMatrix[1] = Vector4{m_Up * worldScaleRecip[1], -m_WorldPosition[1]};
		normalMatrix[2] = Vector4{m_Forward * worldScaleRecip[2], -m_WorldPosition[2]};
		normalMatrix[3][3] = 1;

		m_Matrices = std::make_pair(modelMatrix, normalMatrix);
		m_Changed = false;

		return m_Matrices;
	}


	auto Transform::Activate() -> void
	{ }


	auto Transform::Deactivate() -> void
	{ }


	auto Transform::CalculateLocalAxes() -> void
	{
		m_Forward = m_WorldRotation.Rotate(Vector3::Forward());
		m_Right = m_WorldRotation.Rotate(Vector3::Right());
		m_Up = m_WorldRotation.Rotate(Vector3::Up());
	}


	auto Transform::CalculateWorldPosition() -> void
	{
		m_WorldPosition = m_Parent != nullptr ? m_Parent->m_WorldRotation.Rotate(m_Parent->m_WorldPosition + m_LocalPosition) : m_LocalPosition;
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldPosition);
	}


	auto Transform::CalculateWorldRotation() -> void
	{
		m_WorldRotation = m_Parent != nullptr ? m_Parent->m_WorldRotation * m_LocalRotation : m_LocalRotation;
		CalculateLocalAxes();
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldRotation);
	}


	auto Transform::CalculateWorldScale() -> void
	{
		m_WorldScale = m_Parent != nullptr ? m_Parent->m_WorldScale * m_LocalScale : m_LocalScale;
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldScale);
	}
}
