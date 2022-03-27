#include "Transform.hpp"

#include "../entity/Entity.hpp"
#include "../util/Logger.hpp"

#include <algorithm>
#include <cstddef>
#include <string>


namespace leopph
{
	Transform::Transform(Vector3 const& pos, Quaternion const& rot, Vector3 const& scale) :
		m_WorldPosition{pos},
		m_WorldRotation{rot},
		m_WorldScale{scale},
		m_LocalPosition{pos},
		m_LocalRotation{rot},
		m_LocalScale{scale},
		m_Forward{Vector3::Forward()},
		m_Right{Vector3::Right()},
		m_Up{Vector3::Up()}
	{
		CalculateLocalAxes();
	}


	Transform::Transform(Transform const& other) :
		Component{other},
		m_WorldPosition{other.m_LocalPosition},
		m_WorldRotation{other.m_LocalRotation},
		m_WorldScale{other.m_LocalScale},
		m_LocalPosition{other.m_LocalPosition},
		m_LocalRotation{other.m_LocalRotation},
		m_LocalScale{other.m_LocalScale}
	{
		CalculateLocalAxes();
	}


	auto Transform::operator=(Transform const& other) -> Transform&
	{
		if (this == &other)
		{
			return *this;
		}

		Component::operator=(other);

		m_LocalPosition = other.m_LocalPosition;
		m_LocalRotation = other.m_LocalRotation;
		m_LocalScale = other.m_LocalScale;
		m_Changed = true;

		CalculateWorldPosition();
		CalculateWorldRotation();
		CalculateWorldScale();
		CalculateLocalAxes();

		return *this;
	}


	Transform::~Transform()
	{
		if (m_Parent != nullptr)
		{
			std::erase(m_Parent->m_Children, this);
		}

		std::ranges::for_each(m_Children, [](auto const& child)
		{
			// Can't call parent setter, because that would remove child from this's m_Children, and the loop would crash.
			child->m_Parent = nullptr;
			child->CalculateWorldPosition();
			child->CalculateWorldRotation();
			child->CalculateWorldScale();
		});
	}


	auto Transform::Position(Vector3 const& newPos) -> void
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


	auto Transform::LocalPosition(Vector3 const& newPos) -> void
	{
		m_LocalPosition = newPos;
		CalculateWorldPosition();
	}


	auto Transform::Rotation(Quaternion const& newRot) -> void
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


	auto Transform::LocalRotation(Quaternion const& newRot) -> void
	{
		m_LocalRotation = newRot;
		CalculateWorldRotation();
	}


	auto Transform::Scale(Vector3 const& newScale) -> void
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


	auto Transform::LocalScale(Vector3 const& newScale) -> void
	{
		m_LocalScale = newScale;
		CalculateWorldScale();
	}


	auto Transform::Translate(Vector3 const& vector, Space const base) -> void
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


	auto Transform::Translate(float const x, float const y, float const z, Space const base) -> void
	{
		Translate(Vector3{x, y, z}, base);
	}


	auto Transform::Rotate(Quaternion const& rotation, Space const base) -> void
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


	auto Transform::Rotate(Vector3 const& axis, float amountDegrees, Space const base) -> void
	{
		Rotate(Quaternion{axis, amountDegrees}, base);
	}


	auto Transform::Rescale(Vector3 const& scaling, Space const base) -> void
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


	auto Transform::Rescale(float const x, float const y, float const z, Space const base) -> void
	{
		Rescale(Vector3{x, y, z}, base);
	}


	auto Transform::Parent(leopph::Entity const* const parent) -> void
	{
		Parent(parent->Transform());
	}


	auto Transform::Parent(Transform* const parent) -> void
	{
		// If we're given a valid pointer to an unattached transform
		if (parent != nullptr && !parent->Attached())
		{
			internal::Logger::Instance().Warning("Ignoring attempt to parent Transform on Entity [" + Component::Owner()->Name() + "] to an unattached Transform.");
			return;
		}

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


	auto Transform::Parent(std::shared_ptr<Transform> const& parent) -> void
	{
		return Parent(parent.get());
	}


	auto Transform::Parent(std::nullptr_t const null) -> void
	{
		Parent(static_cast<Transform*>(null));
	}


	auto Transform::Matrices() const -> std::pair<Matrix4, Matrix4> const&
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

		auto const worldScaleRecip{1.f / m_WorldScale};

		Matrix4 normalMatrix;
		normalMatrix[0] = Vector4{m_Right * worldScaleRecip[0], -m_WorldPosition[0]};
		normalMatrix[1] = Vector4{m_Up * worldScaleRecip[1], -m_WorldPosition[1]};
		normalMatrix[2] = Vector4{m_Forward * worldScaleRecip[2], -m_WorldPosition[2]};
		normalMatrix[3][3] = 1;

		m_Matrices = std::make_pair(modelMatrix, normalMatrix);
		m_Changed = false;

		return m_Matrices;
	}


	auto Transform::Owner(Entity* entity) -> void
	{
		auto const& logger{internal::Logger::Instance()};

		if (Attached())
		{
			auto const errMsg = entity
				                    ? Component::Owner() == entity
					                      ? "Ignoring attempt to reattach Transform on Entity \"" + entity->Name() + "\"."
					                      : "Ignoring attempt to change owner of Transform on Entity \"" + Component::Owner()->Name() + "\" to \"" + entity->Name() + "\"."
				                    : "Ignoring attempt to detach Transform on Entity \"" + Component::Owner()->Name() + "\".";
			logger.Warning(errMsg);
			return;
		}

		if (!entity)
		{
			logger.Warning("Ignoring attempt to detach unattached Transform.");
			return;
		}

		if (entity->Transform()->Attached())
		{
			logger.Warning("Ignoring attempt to attach a second Transform to Entity \"" + entity->Name() + "\".");
			return;
		}

		Component::Owner(entity);
	}


	auto Transform::Active(bool const active) -> void
	{
		internal::Logger::Instance().Warning((std::string{"Ignoring attempt to "} += active ? "activate" : "deactivate") += " Transform. Transforms cannot be activated or deactivated.");
	}


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
