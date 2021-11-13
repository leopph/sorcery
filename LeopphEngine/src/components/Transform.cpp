#include "Transform.hpp"

#include "../data/DataManager.hpp"
#include "../entity/Entity.hpp"

#include <algorithm>


namespace leopph
{
	Transform::Transform(leopph::Entity& owner, const Vector3& pos, const Quaternion& rot, const Vector3& scale) :
		Component{owner},
		Changed{m_Changed},
		m_LocalPosition{pos},
		m_LocalRotation{rot},
		m_LocalScale{scale},
		m_WorldPosition{pos},
		m_WorldRotation{rot},
		m_WorldScale{scale},
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
		impl::DataManager::DiscardMatrices(this);

		if (m_Parent != nullptr)
		{
			m_Parent->m_Children.erase(this);
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

	const Vector3& Transform::Position() const
	{
		return m_WorldPosition;
	}

	const Vector3& Transform::LocalPosition() const
	{
		return m_LocalPosition;
	}

	void Transform::Position(const Vector3& newPos)
	{
		if (m_Parent != nullptr)
		{
			m_LocalPosition = newPos * static_cast<Matrix3>(static_cast<Matrix4>(m_Parent->m_WorldRotation).Transpose()) - m_Parent->m_WorldPosition;
		}
		else
		{
			m_LocalPosition = newPos;
		}
		CalculateWorldPosition();
	}

	void Transform::LocalPosition(const Vector3& newPos)
	{
		m_LocalPosition = newPos;
		CalculateWorldPosition();
	}

	const Quaternion& Transform::Rotation() const
	{
		return m_WorldRotation;
	}

	const Quaternion& Transform::LocalRotation() const
	{
		return m_LocalRotation;
	}

	void Transform::Rotation(const Quaternion& newRot)
	{
		// TODO
	}

	void Transform::LocalRotation(const Quaternion& newRot)
	{
		m_LocalRotation = newRot;
		CalculateWorldRotation();
	}

	const Vector3& Transform::Scale() const
	{
		return m_WorldScale;
	}

	const Vector3& Transform::LocalScale() const
	{
		return m_LocalScale;
	}

	void Transform::Scale(const Vector3& newScale)
	{
		// TODO
	}

	void Transform::LocalScale(const Vector3& newScale)
	{
		m_LocalScale = newScale;
		CalculateWorldScale();
	}

	void Transform::Translate(const Vector3& vector, const Space base)
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

	void Transform::Translate(const float x, const float y, const float z, const Space base)
	{
		Translate(Vector3{ x, y, z });
	}

	void Transform::Rotate(const Quaternion& rotation, const Space base)
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

	void Transform::Rotate(const Vector3& axis, float amountDegrees, const Space base)
	{
		Rotate(Quaternion{axis, amountDegrees}, base);
	}

	void Transform::Rescale(const Vector3& scaling, const Space base)
	{
		if (base == Space::World)
		{
			// TODO
		}
		else if (base == Space::Local)
		{
			LocalScale(m_LocalScale * scaling);
		}
	}

	void Transform::Rescale(const float x, const float y, const float z, const Space base)
	{
		Rescale(Vector3{x, y, z}, base);
	}

	const Vector3& Transform::Forward() const
	{
		return m_Forward;
	}

	const Vector3& Transform::Right() const
	{
		return m_Right;
	}

	const Vector3& Transform::Up() const
	{
		return m_Up;
	}

	Transform* Transform::Parent() const
	{
		return m_Parent;
	}

	void Transform::Parent(leopph::Entity* parent)
	{
		Parent(parent->Transform);
	}

	void Transform::Parent(leopph::Entity& parent)
	{
		Parent(parent.Transform);
	}

	void Transform::Parent(Transform* parent)
	{
		if (m_Parent != nullptr)
		{
			m_Parent->m_Children.erase(this);
		}

		m_Parent = parent;
		if (parent != nullptr)
		{
			parent->m_Children.insert(this);
		}

		CalculateWorldPosition();
		CalculateWorldRotation();
		CalculateWorldScale();
	}

	void Transform::Parent(Transform& parent)
	{
		Parent(&parent);
	}

	void Transform::Parent(std::nullptr_t parent)
	{
		Parent(static_cast<Transform*>(parent));
	}

	const std::unordered_set<Transform*>& Transform::Children() const
	{
		return m_Children;
	}

	void Transform::CalculateMatrices()
	{
		if (!Changed)
		{
			return;
		}

		auto modelMatrix{Matrix4::Scale(m_LocalScale)};
		modelMatrix *= static_cast<Matrix4>(m_LocalRotation);
		modelMatrix *= Matrix4::Translate(m_LocalPosition);

		if (m_Parent != nullptr)
		{
			m_Parent->CalculateMatrices();
			modelMatrix *= impl::DataManager::GetMatrices(m_Parent).first;
		}

		/* OpenGL accepts our matrices transposed,
		 * that's why this is done this way. Weird, eh? */
		impl::DataManager::StoreMatrices(this, modelMatrix.Transposed(), modelMatrix.Inverse());

		m_Changed = false;
	}
	
	void Transform::CalculateLocalAxes()
	{
		const auto rotMatrix = static_cast<Matrix3>(static_cast<Matrix4>(m_WorldRotation));
		m_Forward = Vector3::Forward() * rotMatrix;
		m_Right = Vector3::Right() * rotMatrix;
		m_Up = Vector3::Up() * rotMatrix;
	}

	void Transform::OnEventReceived(const impl::FrameEndedEvent&)
	{
		m_Changed = false;
	}

	void Transform::CalculateWorldPosition()
	{
		m_WorldPosition = m_Parent != nullptr ? (m_Parent->m_WorldPosition + m_LocalPosition) * static_cast<Matrix3>(static_cast<Matrix4>(m_Parent->m_WorldRotation)) : m_LocalPosition;
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldPosition);
	}

	void Transform::CalculateWorldRotation()
	{
		m_WorldRotation = m_Parent != nullptr ? m_Parent->m_WorldRotation * m_LocalRotation : m_LocalRotation;
		CalculateLocalAxes();
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldRotation);
	}

	void Transform::CalculateWorldScale()
	{
		m_WorldScale = m_Parent != nullptr ? m_Parent->m_WorldScale * m_LocalScale : m_LocalScale;
		m_Changed = true;
		std::ranges::for_each(m_Children, &Transform::CalculateWorldScale);
	}
}
