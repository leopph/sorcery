#pragma once

#include "../api/LeopphApi.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

#include <cstddef>
#include <ostream>


namespace leopph
{
	class LEOPPHAPI Quaternion
	{
	public:
		explicit Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);
		explicit Quaternion(const Vector3& axis, float angleDegrees);
		Quaternion(const Quaternion& other);
		Quaternion(Quaternion&& other) noexcept;

		~Quaternion() = default;

		Quaternion& operator=(const Quaternion& other);
		Quaternion& operator=(Quaternion&& other) noexcept;

		float& W, & X, & Y, & Z;

		const float& operator[](std::size_t index) const;
		float& operator[](std::size_t index);

		[[nodiscard]]
		Vector3 EulerAngles() const;

		[[nodiscard]]
		float Norm() const;

		[[nodiscard]]
		Quaternion Conjugate() const;

		Quaternion& ConjugateInPlace();

		explicit operator Matrix4() const;

		template<class T>
		impl::Vector<T, 3> Rotate(const impl::Vector<T, 3>& vec) const;


	private:
		float m_W;
		float m_X;
		float m_Y;
		float m_Z;
	};


	LEOPPHAPI Quaternion operator*(const Quaternion& left, const Quaternion& right);
	LEOPPHAPI Quaternion& operator*=(Quaternion& left, const Quaternion& right);
	LEOPPHAPI std::ostream& operator<<(std::ostream& os, const Quaternion& q);

	template<class T>
	impl::Vector<T, 3> Quaternion::Rotate(const impl::Vector<T, 3>& vec) const
	{
		const auto retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} * Conjugate()};
		return impl::Vector<T, 3>{retQuat.m_X, retQuat.m_Y, retQuat.m_Z};
	}
}