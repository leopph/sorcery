#include "Transform.hpp"

#include "../hierarchy/Object.hpp"

#include "../util/logger.h"

namespace leopph
{
	Transform::Transform(Object& owner, const Vector3& pos, Quaternion rot, const Vector3& scale) :
		Component{ owner },
		m_Position{ pos }, m_Rotation{ rot }, m_Scale{ scale },
		m_Forward{ Vector3::Forward() }, m_Right{ Vector3::Right() }, m_Up{ Vector3::Up() }
	{
		CalculateLocalAxes();
	}

	const Vector3& Transform::Position() const
	{
		return m_Position;
	}

	void Transform::Position(const Vector3& newPos)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to set position on static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Position = newPos;
	}

	const Quaternion& Transform::Rotation() const
	{
		return m_Rotation;
	}

	void Transform::Rotation(const Quaternion newRot)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to set rotation on static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Rotation = newRot;
		CalculateLocalAxes();
	}

	const Vector3& Transform::Scale() const
	{
		return m_Scale;
	}

	void Transform::Scale(const Vector3& newScale)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to set scale on static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Scale = newScale;
	}

	void Transform::Translate(const Vector3& vector)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to translate position on static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Position += vector;
	}

	void Transform::Translate(const float x, const float y, const float z)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to translate position on static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Position += Vector3{ x, y, z };
	}

	void Transform::RotateLocal(const Quaternion& rotation)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to rotate static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Rotation *= rotation;
		CalculateLocalAxes();
	}

	void Transform::RotateGlobal(const Quaternion& rotation)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to rotate static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Rotation = rotation * Rotation();
		CalculateLocalAxes();
	}

	void Transform::Rescale(const float x, const float y, const float z)
	{
		if (object.isStatic)
		{
			impl::Logger::Instance().Warning("Trying to rescale static object [" + object.name + "]. Ignoring...");
			return;
		}

		m_Scale[0] *= x;
		m_Scale[1] *= y;
		m_Scale[2] *= z;
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

	void Transform::CalculateLocalAxes()
	{
		const auto rotMatrix = static_cast<Matrix3>(static_cast<Matrix4>(m_Rotation));
		m_Forward = Vector3::Forward() * rotMatrix;
		m_Right = Vector3::Right() * rotMatrix;
		m_Up = Vector3::Up() * rotMatrix;
	}

}
