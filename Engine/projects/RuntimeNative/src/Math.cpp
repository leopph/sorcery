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


	Quaternion Quaternion::FromEulerAngles(f32 const x, f32 const y, f32 const z)
	{
		auto const roll = to_radians(x);
		auto const pitch = to_radians(y);
		auto const yaw = to_radians(z);

		auto const cr = cosf(roll * 0.5f);
		auto const sr = sinf(roll * 0.5f);
		auto const cp = cosf(pitch * 0.5f);
		auto const sp = sinf(pitch * 0.5f);
		auto const cy = cosf(yaw * 0.5f);
		auto const sy = sinf(yaw * 0.5f);

		return Quaternion
		{
			cr * cp * cy + sr * sp * sy,
			sr * cp * cy - cr * sp * sy,
			cr * sp * cy + sr * cp * sy,
			cr * cp * sy - sr * sp * cy,
		};
	}


    Quaternion Quaternion::FromEulerAngles(Vector3 const& euler)
    {
		return FromEulerAngles(euler[0], euler[1], euler[2]);
    }


	Vector3 Quaternion::ToEulerAngles() const
	{
		Vector3 angles;

		// roll (x-axis rotation)
		auto const sinr_cosp = 2 * (w * x + y * z);
		auto const cosr_cosp = 1 - 2 * (x * x + y * y);
		angles[0] = std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		auto const sinp = 2 * (w * y - z * x);
		if (std::abs(sinp) >= 1)
			angles[1] = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
		else
			angles[1] = std::asin(sinp);

		// yaw (z-axis rotation)
		auto const siny_cosp = 2 * (w * z + x * y);
		auto const cosy_cosp = 1 - 2 * (y * y + z * z);
		angles[2] = std::atan2(siny_cosp, cosy_cosp);

		for (int i = 0; i < 2; i++) {
			angles[i] = to_degrees(angles[i]);
		}

		return angles;
	}


	auto Quaternion::ToRotationMatrix() const noexcept -> Matrix4 {
		return static_cast<Matrix4>(*this);
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
