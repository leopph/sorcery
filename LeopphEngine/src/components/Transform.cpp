#include "Transform.hpp"

#include "../data/DataManager.hpp"
#include "../entity/Entity.hpp"

#include <algorithm>


namespace leopph
{
	Transform::Transform(leopph::Entity& owner, const Vector3& pos, const Quaternion& rot, const Vector3& scale) :
		Component{owner},
		WasAltered{m_WasAltered},
		m_LocalPosition{pos},
		m_LocalRotation{rot},
		m_LocalScale{scale},
		m_GlobalPosition{pos},
		m_GlobalRotation{rot},
		m_GlobalScale{scale},
		m_Forward{Vector3::Forward()},
		m_Right{Vector3::Right()},
		m_Up{Vector3::Up()},
		m_Parent{nullptr},
		m_WasAltered{true}
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
			child->CalculateGlobalPosition();
			child->CalculateGlobalRotation();
			child->CalculateGlobalScale();
		});
	}

	const Vector3& Transform::Position() const
	{
		return m_GlobalPosition;
	}

	const Vector3& Transform::LocalPosition() const
	{
		return m_LocalPosition;
	}

	void Transform::Position(const Vector3& newPos)
	{
		m_LocalPosition = newPos;
		CalculateGlobalPosition();
	}

	const Quaternion& Transform::Rotation() const
	{
		return m_GlobalRotation;
	}

	const Quaternion& Transform::LocalRotation() const
	{
		return m_LocalRotation;
	}

	void Transform::Rotation(const Quaternion newRot)
	{
		m_LocalRotation = newRot;
		CalculateGlobalRotation();
	}

	const Vector3& Transform::Scale() const
	{
		return m_GlobalScale;
	}

	const Vector3& Transform::LocalScale() const
	{
		return m_LocalScale;
	}

	void Transform::Scale(const Vector3& newScale)
	{
		m_LocalScale = newScale;
		CalculateGlobalScale();
	}

	void Transform::Translate(const Vector3& vector)
	{
		Position(m_LocalPosition + vector);
	}

	void Transform::Translate(const float x, const float y, const float z)
	{
		Position(m_LocalPosition + Vector3{x, y, z});
	}

	void Transform::RotateLocal(const Quaternion& rotation)
	{
		Rotation(m_LocalRotation * rotation);
	}

	void Transform::RotateGlobal(const Quaternion& rotation)
	{
		Rotation(rotation * m_LocalRotation);
	}

	void Transform::Rescale(const float x, const float y, const float z)
	{
		auto scale{m_LocalScale};
		scale[0] *= x;
		scale[1] *= y;
		scale[2] *= z;
		Scale(scale);
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

		CalculateGlobalPosition();
		CalculateGlobalRotation();
		CalculateGlobalScale();
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
		if (!WasAltered)
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

		m_WasAltered = false;
	}
	
	void Transform::CalculateLocalAxes()
	{
		const auto rotMatrix = static_cast<Matrix3>(static_cast<Matrix4>(m_GlobalRotation));
		m_Forward = Vector3::Forward() * rotMatrix;
		m_Right = Vector3::Right() * rotMatrix;
		m_Up = Vector3::Up() * rotMatrix;
	}

	void Transform::OnEventReceived(const impl::FrameEndedEvent&)
	{
		m_WasAltered = false;
	}

	void Transform::CalculateGlobalPosition()
	{
		m_GlobalPosition = m_Parent != nullptr ? m_Parent->m_GlobalPosition + m_LocalPosition : m_LocalPosition;
		m_WasAltered = true;
		std::ranges::for_each(m_Children, &Transform::CalculateGlobalPosition);
	}

	void Transform::CalculateGlobalRotation()
	{
		m_GlobalRotation = m_Parent != nullptr ? m_Parent->m_GlobalRotation * m_LocalRotation : m_LocalRotation;
		CalculateLocalAxes();
		m_WasAltered = true;
		std::ranges::for_each(m_Children, &Transform::CalculateGlobalRotation);
	}

	void Transform::CalculateGlobalScale()
	{
		m_GlobalScale = m_Parent != nullptr ? m_Parent->m_GlobalScale * m_LocalScale : m_LocalScale;
		m_WasAltered = true;
		std::ranges::for_each(m_Children, &Transform::CalculateGlobalScale);
	}
}
