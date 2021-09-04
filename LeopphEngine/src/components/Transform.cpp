#include "Transform.hpp"

#include "../hierarchy/Object.hpp"
#include "../data/DataManager.hpp"
#include "../util/logger.h"



namespace leopph
{
	Transform::Transform(Object& owner, const Vector3& pos, Quaternion rot, const Vector3& scale) :
		Component{ owner },
		m_Position{ pos }, m_Rotation{ rot }, m_Scale{ scale },
		m_Forward{ Vector3::Forward() }, m_Right{ Vector3::Right() }, m_Up{ Vector3::Up() },
		m_WasAltered{true}, WasAltered{m_WasAltered}
	{
		CalculateLocalAxes();
	}


	Transform::~Transform()
	{
		impl::DataManager::DiscardMatrices(this);
	}


	const Vector3& Transform::Position() const
	{
		return m_Position;
	}


	void Transform::Position(const Vector3& newPos)
	{
		m_Position = newPos;
		m_WasAltered = true;
	}


	const Quaternion& Transform::Rotation() const
	{
		return m_Rotation;
	}


	void Transform::Rotation(const Quaternion newRot)
	{
		m_Rotation = newRot;
		CalculateLocalAxes();
		m_WasAltered = true;
	}


	const Vector3& Transform::Scale() const
	{
		return m_Scale;
	}


	void Transform::Scale(const Vector3& newScale)
	{
		m_Scale = newScale;
		m_WasAltered = true;
	}


	void Transform::Translate(const Vector3& vector)
	{
		Position(Position() + vector);
	}


	void Transform::Translate(const float x, const float y, const float z)
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


	void Transform::Rescale(const float x, const float y, const float z)
	{
		Vector3 scale{Scale()};
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


	void Transform::CalculateLocalAxes()
	{
		const auto rotMatrix = static_cast<Matrix3>(static_cast<Matrix4>(m_Rotation));
		m_Forward = Vector3::Forward() * rotMatrix;
		m_Right = Vector3::Right() * rotMatrix;
		m_Up = Vector3::Up() * rotMatrix;
	}


	void Transform::OnEventReceived(const impl::FrameEndedEvent&)
	{
		m_WasAltered = false;
	}
}
