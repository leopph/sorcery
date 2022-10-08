#include "Math.hpp"


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


	Quaternion::Quaternion(f32 const w, f32 const x, f32 const y, f32 const z) :
		x{x}, y{y}, z{z}, w{w}
	{}


	Quaternion::Quaternion(Vector3 const& axis, f32 const angleDegrees)
	{
		auto const angleHalfRadians = to_radians(angleDegrees) / 2.0f;
		auto vec = axis.normalized() * std::sinf(angleHalfRadians);

		w = std::cos(angleHalfRadians);
		x = vec[0];
		y = vec[1];
		z = vec[2];
	}


	Vector3 Quaternion::to_euler_angles() const
	{
		// ???
		auto secondComponent{2 * (w * y - z * x)};
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
			to_degrees(std::atan2(2 * (w * x + y * z), 1 - 2 * (std::powf(x, 2) + std::powf(y, 2)))),
			to_degrees(secondComponent),
			to_degrees(std::atan2(2 * (w * z + x * y), 1 - 2 * (std::powf(y, 2) + std::powf(z, 2))))
		};
	}


	void Quaternion::to_axis_angle(Vector3& axis, float& angle) const
	{
		axis = {x, y, z};
		float const vectorLength = axis.length();
		axis.normalize();
		angle = 2 * std::atan2f(vectorLength, w);
	}


	f32 Quaternion::get_norm_squared() const
	{
		return std::powf(w, 2) + std::powf(x, 2) + std::powf(y, 2) + std::powf(z, 2);
	}


	f32 Quaternion::get_norm() const
	{
		return std::sqrtf(get_norm_squared());
	}


	Quaternion Quaternion::normalized() const
	{
		return Quaternion{*this}.normalize();
	}


	Quaternion& Quaternion::normalize()
	{
		auto const norm = get_norm();
		w /= norm;
		x /= norm;
		y /= norm;
		z /= norm;
		return *this;
	}



	Quaternion Quaternion::conjugate() const
	{
		return Quaternion{*this}.conjugate_in_place();
	}


	Quaternion& Quaternion::conjugate_in_place()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}


	Quaternion Quaternion::inverse() const
	{
		return Quaternion{*this}.invert();
	}


	Quaternion& Quaternion::invert()
	{
		auto const normSquared = get_norm_squared();
		w /= normSquared;
		x = -x / normSquared;
		y = -y / normSquared;
		z = -z / normSquared;
		return *this;
	}


	Quaternion::operator Matrix4() const
	{
		return Matrix4
		{
			1 - 2 * (std::powf(y, 2) + std::powf(z, 2)), 2 * (x * y + z * w), 2 * (x * z - y * w), 0,
			2 * (x * y - z * w), 1 - 2 * (std::powf(x, 2) + std::powf(z, 2)), 2 * (y * z + x * w), 0,
			2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (std::powf(x, 2) + std::powf(y, 2)), 0,
			0, 0, 0, 1
		};
	}


	Quaternion operator*(Quaternion const& left, Quaternion const& right)
	{
		return Quaternion
		{
			left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
			left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
			left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
			left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w
		};
	}


	Quaternion& operator*=(Quaternion& left, Quaternion const& right)
	{
		return left = left * right;
	}


	std::ostream& operator<<(std::ostream& os, Quaternion const& q)
	{
		os << "(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
		return os;
	}
}
