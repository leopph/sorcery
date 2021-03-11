#include "transform.h"

namespace leopph
{
	Transform::Transform() :
		m_Position{}, m_Rotation{}, m_Scale{ 1, 1, 1 },
		m_Forward{ Vector3::Forward() }, m_Right{ Vector3::Right() }, m_Up{ Vector3::Up() }
	{}




	const Vector3& Transform::Position() const
	{
		return m_Position;
	}

	void Transform::Position(Vector3 newPos)
	{
		m_Position = std::move(newPos);
	}

	const Quaternion& Transform::Rotation() const
	{
		return m_Rotation;
	}

	void Transform::Rotation(Quaternion newRot)
	{
		m_Rotation = std::move(newRot);

		auto rotMatrix = static_cast<Matrix3>(static_cast<Matrix4>(m_Rotation));
		m_Forward = Vector3::Forward() * rotMatrix;
		m_Right = Vector3::Right() * rotMatrix;
		m_Up = Vector3::Up() * rotMatrix;
	}

	const Vector3& Transform::Scale() const
	{
		return m_Scale;
	}

	void Transform::Scale(Vector3 newScale)
	{
		m_Scale = std::move(newScale);
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



	void Transform::Translate(const Vector3& vector)
	{
		Position(Position() + vector);
	}

	void Transform::Translate(float x, float y, float z)
	{
		Position(Position() + Vector3{ x, y, z });
	}

	void Transform::RotateLocal(const Quaternion& rotation)
	{
		Rotation(Rotation() * rotation);
	}

	void Transform::RotateGlobal(const Quaternion& rotation)
	{
		Rotation(rotation * Rotation());
	}

	void Transform::Rescale(float x, float y, float z)
	{
		m_Scale[0] *= x;
		m_Scale[1] *= y;
		m_Scale[2] *= z;
	}
}