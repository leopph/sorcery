#pragma once

#include <glm/glm.hpp>

namespace leopph
{
	class Vector3
	{
	private:
		glm::vec3 m_Vec;

	public:
		Vector3();
		Vector3(const Vector3& other);

		Vector3(float xyz);
		Vector3(float x, float y, float z);

		Vector3& operator=(const Vector3& other);

		float x() const;
		float y() const;
		float z() const;

		void x(float x);
		void y(float y);
		void z(float z);


		static Vector3 Up();
		static Vector3 Down();
		static Vector3 Left();
		static Vector3 Right();
		static Vector3 Forward();
		static Vector3 Backward();

		static float Dot(const Vector3& left, const Vector3& right);
		static Vector3 Cross(const Vector3& left, const Vector3& right);

		static Vector3 Project(const Vector3& vector, const Vector3& normal);
	};



	Vector3 operator+(const Vector3& left, const Vector3& right);
	Vector3& operator+=(Vector3& left, const Vector3& right);

	Vector3 operator-(const Vector3& left, const Vector3& right);
	Vector3& operator-=(Vector3& left, const Vector3& right);

	Vector3 operator*(const Vector3& left, float right);
	Vector3 operator*(float left, const Vector3& right);
	Vector3& operator*=(Vector3& left, float right);
	Vector3& operator*=(float left, Vector3& right);

	Vector3 operator*(const Vector3& left, const Vector3& right);
	Vector3& operator*=(Vector3& left, const Vector3& right);

	Vector3 operator/(const Vector3& left, float right);
	Vector3& operator/=(Vector3& left, float right);

	Vector3 operator/(const Vector3& left, const Vector3& right);
	Vector3& operator/=(Vector3& left, const Vector3& right);
}