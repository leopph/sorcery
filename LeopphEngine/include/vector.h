#pragma once

#include <type_traits>
#include <ostream>


namespace leopph
{
	template<class T, size_t N>
	class Vector
	{
		static_assert(N >= 2, "Vector dimension must be at least 2!");


	private:
		T* m_Data;


	public:
		// constructors
		Vector() :
			m_Data{ new T[N]{} }
		{}

		Vector(const T& value) :
			m_Data{ new T[N]{} }
		{
			for (size_t i = 0; i < N; i++)
				m_Data[i] = value;
		}

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == N, bool> = false>
		Vector(const T1&... args) :
			m_Data{ new T[N]{ static_cast<T>(args)... } }
		{}

		Vector(const Vector<T, N>& other) :
			m_Data{ new T[N] }
		{
			for (size_t i = 0; i < N; i++)
				m_Data[i] = other.m_Data[i];
		}


		 // destructor
		~Vector()
		{
			delete[] m_Data;
		}




		// quick access factories
		static Vector<T, N> Up()
		{
			Vector<T, N> ret;
			ret[1] = 1;
			return ret;
		}

		static Vector<T, N> Down()
		{
			Vector<T, N> ret;
			ret[1] = -1;
			return ret;
		}

		static Vector<T, N> Left()
		{
			Vector<T, N> ret;
			ret[0] = -1;
			return ret;
		}

		static Vector<T, N> Right()
		{
			Vector<T, N> ret;
			ret[0] = 1;
			return ret;
		}

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Forward()
		{
			Vector<T, N> ret;
			ret[2] = 1;
			return ret;
		}

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Backward()
		{
			Vector<T, N> ret;
			ret[2] = -1;
			return ret;
		}



		// member operators
		Vector<T, N>& operator=(const Vector<T, N>& other)
		{
			if (this == &other)
				return *this;

			for (size_t i = 0; i < N; i++)
				m_Data[i] = other.m_Data[i];

			return *this;
		}

		const T& operator[](size_t index) const
		{
			return m_Data[index];
		}

		T& operator[](size_t index)
		{
			return const_cast<T&>(const_cast<const Vector<T, N>*>(this)->operator[](index));
		}




		// magnitude
		float Length() const
		{
			float sqrSum{};

			for (size_t i = 0; i < N; i++)
				sqrSum += std::powf(m_Data[i], 2);

			return std::sqrtf(sqrSum);
		}

		// normalized copy
		Vector<T, N> Normalized() const
		{
			float length = Length();
			Vector<T, N> ret{ *this };

			for (size_t i = 0; i < N; i++)
				ret[i] /= length;

			return ret;
		}

		// in place normalize
		Vector<T, N>& Normalize()
		{
			float length = Length();

			for (size_t i = 0; i < N; i++)
				m_Data[i] /= length;

			return *this;
		}


		// dot product
		static T Dot(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			T ret{};

			for (size_t i = 0; i < N; i++)
				ret += left[i] * right[i];

			return ret;
		}

		// cross product, only for 3d vector
		template<size_t N1 = N, std::enable_if_t<N1 == 3 && N1 == N, bool> = false>
		static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			return Vector<T, N> { left[1] * right[2] - left[2] * right[1],
				left[2] * right[0] - left[0] * right[2],
				left[0] * right[1] - left[1] * right[0] };
		}
	};






	// non member operators
	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& operand)
	{
		Vector<T, N> ret{ *this };

		for (size_t i = 0; i < N; i++)
			ret[i] = -ret[i];

		return ret;
	}




	template<class T, size_t N>
	Vector<T, N> operator+(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		Vector<T, N> ret;

		for (size_t i = 0; i < N; i++)
			ret[i] = left[i] + right[i];

		return ret;
	}

	template<class T, size_t N>
	Vector<T, N> operator+=(Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left = left + right;
	}




	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left + -right;
	}

	template<class T, size_t N>
	Vector<T, N> operator-=(Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left = left - right;
	}




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right)
	{
		Vector<T1, N> ret{ left };

		for (size_t i = 0; i < N; i++)
			ret[i] *= right;

		return ret;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right)
	{
		return right * left;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(Vector<T1, N>& left, const T2& right)
	{
		return left = left * right;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(const T2& left, Vector<T1, N>& right)
	{
		return right = left * right;
	}




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right)
	{
		return left * (static_cast<T1>(1) / right);
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/=(Vector<T1, N>& left, const T2& right)
	{
		return left = left * (static_cast<T1>(1) / right);
	}




	template<class T, size_t N>
	bool operator==(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		for (size_t i = 0; i < N; i++)
			if (left[i] != right[i])
				return false;

		return true;
	}

	template<class T, size_t N>
	bool operator!=(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		return !(left == right);
	}




	template<class T, size_t N>
	std::ostream& operator<<(std::ostream& stream, const Vector<T, N>& vector)
	{
		stream << "(";

		for (size_t i = 0; i < N; i++)
		{
			stream << vector[i];

			if (i != N - 1)
				stream << ", ";
		}

		stream << ")";

		return stream;
	}



	// instantiation imports/exports
	template class Vector<float, 4>;
	template class Vector<float, 3>;
	template class Vector<float, 2>;

	// aliasing
	using Vector4 = Vector<float, 4>;
	using Vector3 = Vector<float, 3>;
	using Vector2 = Vector<float, 2>;
}