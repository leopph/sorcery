#include "Math.hpp"

#include <cmath>


namespace leopph
{
	f32 const PI = 2 * std::acosf(0);



	f32 to_radians(f32 const degrees)
	{
		return degrees * PI / 180.0f;
	}



	f32 to_degrees(f32 const radians)
	{
		return radians * 180.0f / PI;
	}



	bool is_power_of_two(u32 const value)
	{
		return value != 0 && (value & (value - 1)) == 0;
	}



	u32 next_power_of_two(u32 const value)
	{
		if (is_power_of_two(value))
		{
			return value;
		}

		u32 ret{1};

		while (ret < value)
		{
			ret <<= 1;
		}

		return ret;
	}



	f32 lerp(f32 const from, f32 const to, f32 const t)
	{
		return (1 - t) * from + t * to;
	}



	u8 num_binary_digits(u32 const number)
	{
		return static_cast<u8>(std::log2(static_cast<f32>(number))) + 1;
	}



	f32 Quaternion::get_w() const
	{
		return mW;
	}



	void Quaternion::set_w(f32 const w)
	{
		mW = w;
	}



	f32 Quaternion::get_x() const
	{
		return mX;
	}



	void Quaternion::set_x(f32 const x)
	{
		mX = x;
	}



	f32 Quaternion::get_y() const
	{
		return mY;
	}



	void Quaternion::set_y(f32 const y)
	{
		mY = y;
	}



	f32 Quaternion::get_z() const
	{
		return mZ;
	}



	void Quaternion::set_z(f32 const z)
	{
		mZ = z;
	}



	Vector3 Quaternion::to_euler_angles() const
	{
		// ???
		auto secondComponent{2 * (mW * mY - mZ * mX)};
		if (std::abs(secondComponent) > 1)
		{
			secondComponent = std::copysign(PI / 2, secondComponent);
		}
		else
		{
			secondComponent = std::asin(secondComponent);
		}
		return Vector3
		{
			to_degrees(std::atan2(2 * (mW * mX + mY * mZ), 1 - 2 * (std::powf(mX, 2) + std::powf(mY, 2)))),
			to_degrees(secondComponent),
			to_degrees(std::atan2(2 * (mW * mZ + mX * mY), 1 - 2 * (std::powf(mY, 2) + std::powf(mZ, 2))))
		};
	}



	f32 Quaternion::get_norm() const
	{
		return std::sqrt(std::powf(mW, 2) + std::powf(mX, 2) + std::powf(mY, 2) + std::powf(mZ, 2));
	}



	Quaternion Quaternion::normalized() const
	{
		auto const norm{get_norm()};
		return Quaternion{mW / norm, mX / norm, mY / norm, mZ / norm};
	}



	Quaternion& Quaternion::normalize()
	{
		auto const norm{get_norm()};
		mW /= norm;
		mX /= norm;
		mY /= norm;
		mZ /= norm;
		return *this;
	}



	Quaternion Quaternion::conjugate() const
	{
		return Quaternion{mW, -mX, -mY, -mZ};
	}



	Quaternion& Quaternion::conjugate_in_place()
	{
		mX = -mX;
		mY = -mY;
		mZ = -mZ;
		return *this;
	}



	Quaternion Quaternion::inverse() const
	{
		auto const normSquared{std::powf(mW, 2) + std::powf(mX, 2) + std::powf(mY, 2) + std::powf(mZ, 2)};
		return Quaternion{mW / normSquared, -mX / normSquared, -mY / normSquared, -mZ / normSquared};
	}



	Quaternion& Quaternion::invert()
	{
		auto const normSquared{std::powf(mW, 2) + std::powf(mX, 2) + std::powf(mY, 2) + std::powf(mZ, 2)};
		mW /= normSquared;
		mX = -mX / normSquared;
		mY = -mY / normSquared;
		mZ = -mZ / normSquared;
		return *this;
	}



	Quaternion::operator Matrix4() const
	{
		return Matrix4
		{
			1 - 2 * (std::powf(mY, 2) + std::powf(mZ, 2)), 2 * (mX * mY + mZ * mW), 2 * (mX * mZ - mY * mW), 0,
			2 * (mX * mY - mZ * mW), 1 - 2 * (std::powf(mX, 2) + std::powf(mZ, 2)), 2 * (mY * mZ + mX * mW), 0,
			2 * (mX * mZ + mY * mW), 2 * (mY * mZ - mX * mW), 1 - 2 * (std::powf(mX, 2) + std::powf(mY, 2)), 0,
			0, 0, 0, 1
		};
	}



	Quaternion::Quaternion(f32 const w, f32 const x, f32 const y, f32 const z) :
		mW{w},
		mX{x},
		mY{y},
		mZ{z}
	{}



	Quaternion::Quaternion(Vector3 const& axis, f32 const angleDegrees)
	{
		auto normalizedAxis{axis.normalized()};
		auto const angleHalfRadians = to_radians(angleDegrees) / 2.0f;
		mW = std::cos(angleHalfRadians);
		normalizedAxis *= std::sin(angleHalfRadians);
		mX = normalizedAxis[0];
		mY = normalizedAxis[1];
		mZ = normalizedAxis[2];
	}



	Quaternion operator*(Quaternion const& left, Quaternion const& right)
	{
		return Quaternion
		{
			left.get_w() * right.get_w() - left.get_x() * right.get_x() - left.get_y() * right.get_y() - left.get_z() * right.get_z(),
			left.get_w() * right.get_x() + left.get_x() * right.get_w() + left.get_y() * right.get_z() - left.get_z() * right.get_y(),
			left.get_w() * right.get_y() - left.get_x() * right.get_z() + left.get_y() * right.get_w() + left.get_z() * right.get_x(),
			left.get_w() * right.get_z() + left.get_x() * right.get_y() - left.get_y() * right.get_x() + left.get_z() * right.get_w()
		};
	}



	Quaternion& operator*=(Quaternion& left, Quaternion const& right)
	{
		return left = left * right;
	}



	std::ostream& operator<<(std::ostream& os, Quaternion const& q)
	{
		os << "(" << q.get_w() << ", " << q.get_x() << ", " << q.get_y() << ", " << q.get_y() << ")";
		return os;
	}
}
