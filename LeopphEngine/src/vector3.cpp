#include "vector3.h"

namespace leopph
{
	Vector3::Vector3() :
		m_Vec{ 0.0f, 0.0f, 0.0f }
	{

	}

	Vector3::Vector3(const Vector3& other) :
		m_Vec{ other.m_Vec }
	{

	}

	Vector3::Vector3(float xyz) :
		m_Vec{ xyz, xyz, xyz }
	{

	}

	Vector3::Vector3(float x, float y, float z) :
		m_Vec{ x, y, z }
	{

	}




	Vector3& Vector3::operator=(const Vector3& other)
	{
		if (&other == this)
			return *this;

		m_Vec = other.m_Vec;
		return *this;
	}




	float Vector3::x() const
	{
		return m_Vec.x;
	}

	float Vector3::y() const
	{
		return m_Vec.y;
	}

	float Vector3::z() const
	{
		return m_Vec.z;
	}


	void Vector3::x(float x)
	{
		m_Vec.x = x;
	}

	void Vector3::y(float y)
	{
		m_Vec.y = y;
	}

	void Vector3::z(float z)
	{
		m_Vec.z = z;
	}





	Vector3 Vector3::Up()
	{
		return Vector3{ 0.0f, 1.0f, 0.0f };
	}

	Vector3 Vector3::Down()
	{
		return Vector3{ 0.0f, -1.0f, 0.0f };
	}

	Vector3 Vector3::Left()
	{
		return Vector3{ -1.0f, 0.0f, 0.0f };
	}

	Vector3 Vector3::Right()
	{
		return Vector3{ 1.0f, 0.0f, 0.0f };
	}

	Vector3 Vector3::Forward()
	{
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	Vector3 Vector3::Backward()
	{
		return Vector3{ 0.0f, 0.0f, -1.0f };
	}




	float Vector3::Dot(const Vector3& left, const Vector3& right)
	{
		return glm::dot(left.m_Vec, right.m_Vec);
	}

	Vector3 Vector3::Cross(const Vector3& left, const Vector3& right)
	{
		Vector3 ret;
		ret.m_Vec = glm::cross(left.m_Vec, right.m_Vec);
		return ret;
	}




	Vector3 Vector3::Project(const Vector3& vector, const Vector3& normal)
	{
		return vector - Vector3::Dot(vector, normal) * normal;
	}






	Vector3 operator+(const Vector3& left, const Vector3& right)
	{
		return Vector3(left.x() + right.x(), left.y() + right.y(), left.z() + right.z());
	}

	Vector3& operator+=(Vector3& left, const Vector3& right)
	{
		left = left + right;
		return left;
	}


	Vector3 operator-(const Vector3& left, const Vector3& right)
	{
		return Vector3(left.x() - right.x(), left.y() - right.y(), left.z() - right.z());
	}

	Vector3& operator-=(Vector3& left, const Vector3& right)
	{
		left = left - right;
		return left;
	}


	Vector3 operator*(const Vector3& left, float right)
	{
		return Vector3(left.x() * right, left.y() * right, left.z() * right);
	}

	Vector3 operator*(float left, const Vector3& right)
	{
		return right * left;
	}

	Vector3& operator*=(Vector3& left, float right)
	{
		left = left * right;
		return left;
	}

	Vector3& operator*=(float left, Vector3& right)
	{
		right = right * left;
		return right;
	}


	Vector3 operator*(const Vector3& left, const Vector3& right)
	{
		return Vector3(left.x() * right.x(), left.y() * right.y(), left.z() * right.z());
	}

	Vector3& operator*=(Vector3& left, const Vector3& right)
	{
		left = left * right;
		return left;
	}


	Vector3 operator/(const Vector3& left, float right)
	{
		return Vector3(left.x() / right, left.y() / right, left.z() / right);
	}

	Vector3& operator/=(Vector3& left, float right)
	{
		left = left / right;
		return left;
	}


	Vector3 operator/(const Vector3& left, const Vector3& right)
	{
		return Vector3(left.x() / right.x(), left.y() / right.y(), left.z() / right.z());
	}

	Vector3& operator/=(Vector3& left, const Vector3& right)
	{
		left = left / right;
		return left;
	}
}