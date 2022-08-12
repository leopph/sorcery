#pragma once

#include "LeopphApi.hpp"
#include "Matrix.hpp"
#include "Types.hpp"
#include "Vector.hpp"

#include <ostream>


namespace leopph
{
	class Quaternion
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_w() const;
			LEOPPHAPI void set_w(f32 w);

			[[nodiscard]] f32 get_x() const;
			LEOPPHAPI void set_x(f32 x);

			[[nodiscard]] f32 get_y() const;
			LEOPPHAPI void set_y(f32 y);

			[[nodiscard]] f32 get_z() const;
			LEOPPHAPI void set_z(f32 z);


			[[nodiscard]] LEOPPHAPI Vector3 to_euler_angles() const;


			[[nodiscard]] LEOPPHAPI f32 get_norm() const;
			[[nodiscard]] LEOPPHAPI Quaternion normalized() const;
			LEOPPHAPI Quaternion& normalize();


			[[nodiscard]] LEOPPHAPI Quaternion conjugate() const;
			LEOPPHAPI Quaternion& conjugate_in_place();


			[[nodiscard]] LEOPPHAPI Quaternion inverse() const;
			LEOPPHAPI Quaternion& invert();


			[[nodiscard]] LEOPPHAPI explicit operator Matrix4() const;


			template<class T>
			[[nodiscard]] auto rotate(Vector<T, 3> const& vec) const;


			LEOPPHAPI explicit Quaternion(f32 w = 1.0f, f32 x = 0.0f, f32 y = 0.0f, f32 z = 0.0f);
			LEOPPHAPI explicit Quaternion(Vector3 const& axis, f32 angleDegrees);

			Quaternion(Quaternion const& other) = default;
			Quaternion(Quaternion&& other) noexcept = default;

			Quaternion& operator=(Quaternion const& other) = default;
			Quaternion& operator=(Quaternion&& other) noexcept = default;

			~Quaternion() = default;


		private:
			f32 mW;
			f32 mX;
			f32 mY;
			f32 mZ;
	};


	[[nodiscard]] LEOPPHAPI Quaternion operator*(Quaternion const& left, Quaternion const& right);
	LEOPPHAPI Quaternion& operator*=(Quaternion& left, Quaternion const& right);

	LEOPPHAPI std::ostream& operator<<(std::ostream& os, Quaternion const& q);



	template<class T>
	auto Quaternion::rotate(Vector<T, 3> const& vec) const
	{
		auto const retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} * conjugate()};
		return Vector<T, 3>{retQuat.mX, retQuat.mY, retQuat.mZ};
	}
}
