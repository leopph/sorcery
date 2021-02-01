#pragma once

#include <type_traits>

#include "leopphapi.h"


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
		Vector();
		Vector(const T& value);

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == N, bool> = false>
		Vector(const T1&... args);

		Vector(const Vector<T, N>& other);

		//Vector(Vector<T, N>&& other) = delete;


		 // destructor
		~Vector();




		// quick access factories
		static Vector<T, N> Up();

		static Vector<T, N> Down();

		static Vector<T, N> Left();

		static Vector<T, N> Right();

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Forward();

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Backward();



		// member operators
		Vector<T, N>& operator=(const Vector<T, N>& other);
		const T& operator[](size_t index) const;
		T& operator[](size_t index);




		// magnitude
		float Length() const;

		// normalized copy
		Vector<T, N> Normalized() const;

		// in place normalize
		void Normalize();


		// dot product
		static T Dot(const Vector<T, N>& left, const Vector<T, N>& right);

		// cross product, only for 3d vector
		template<size_t N1 = N, std::enable_if_t<N1 == 3 && N1 == N, bool> = false>
		static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right);
	};






	// non member operators
	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& operand);




	template<class T, size_t N>
	Vector<T, N> operator+(const Vector<T, N>& left, const Vector<T, N>& right);

	template<class T, size_t N>
	Vector<T, N> operator+=(Vector<T, N>& left, const Vector<T, N>& right);




	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& left, const Vector<T, N>& right);

	template<class T, size_t N>
	Vector<T, N> operator-=(Vector<T, N>& left, const Vector<T, N>& right);




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right);

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right);

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(Vector<T1, N>& left, const T2& right);

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(const T2& left, Vector<T1, N>& right);




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right);

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/=(Vector<T1, N>& left, const T2& right);




	template<class T, size_t N>
	bool operator==(const Vector<T, N>& left, const Vector<T, N>& right);

	template<class T, size_t N>
	bool operator!=(const Vector<T, N>& left, const Vector<T, N>& right);




	template<class T, size_t N>
	std::ostream& operator<<(std::ostream& stream, const Vector<T, N>& vector);



	// instantiation imports/exports
	template class LEOPPHAPI Vector<float, 4>;
	template class LEOPPHAPI Vector<float, 3>;
	template class LEOPPHAPI Vector<float, 2>;

	// aliasing
	using Vector4 = Vector<float, 4>;
	using Vector3 = Vector<float, 3>;
	using Vector2 = Vector<float, 2>;
}