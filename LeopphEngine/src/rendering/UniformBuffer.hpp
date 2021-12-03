#pragma once

#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"

#include <cstddef>
#include <vector>


namespace leopph::impl
{
	class UniformBuffer
	{
		UniformBuffer();
		UniformBuffer(const UniformBuffer& other) = delete;
		UniformBuffer(UniformBuffer&& other) = delete;

		~UniformBuffer();

		UniformBuffer& operator=(const UniformBuffer& other) = delete;
		UniformBuffer& operator=(UniformBuffer&& other) = delete;

		const unsigned& Name;
		const std::size_t& Size;

		void Store(int i, std::size_t offset) const;
		void Store(unsigned u, std::size_t offset) const;
		void Store(float f, std::size_t offset) const;
		void Store(const Vector3& vec, std::size_t offset) const;
		void Store(const Matrix4& mat, std::size_t offset) const;
		void Store(const std::vector<int>& vec, std::size_t offset) const;
		void Store(const std::vector<unsigned>& vec, std::size_t offset) const;
		void Store(const std::vector<float>& vec, std::size_t offset) const;
		void Store(const std::vector<Vector3>& vec, std::size_t offset) const;
		void Store(const std::vector<Matrix4>& vec, std::size_t offset) const;

		void Bind(int bindingIndex, int offset, std::size_t size) const;


	private:
		unsigned m_Name;
		std::size_t m_Size;
	};
}