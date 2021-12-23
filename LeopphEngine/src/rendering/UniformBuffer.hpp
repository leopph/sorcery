#pragma once

#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"

#include <cstddef>
#include <vector>


namespace leopph::internal
{
	class UniformBuffer
	{
		public:
			UniformBuffer();

			UniformBuffer(const UniformBuffer& other) = delete;
			auto operator=(const UniformBuffer& other) -> UniformBuffer& = delete;

			UniformBuffer(UniformBuffer&& other) = delete;
			auto operator=(UniformBuffer&& other) -> UniformBuffer& = delete;

			~UniformBuffer();

			[[nodiscard]]
			auto Name() const -> unsigned;
			[[nodiscard]]
			auto Size() const -> std::size_t;

			auto Bind(int bindingIndex, int offset, std::size_t size) const -> void;

			auto Store(int i, std::size_t offset) const -> void;
			auto Store(unsigned u, std::size_t offset) const -> void;
			auto Store(float f, std::size_t offset) const -> void;
			auto Store(const Vector3& vec, std::size_t offset) const -> void;
			auto Store(const Matrix4& mat, std::size_t offset) const -> void;
			auto Store(const std::vector<int>& vec, std::size_t offset) const -> void;
			auto Store(const std::vector<unsigned>& vec, std::size_t offset) const -> void;
			auto Store(const std::vector<float>& vec, std::size_t offset) const -> void;
			auto Store(const std::vector<Vector3>& vec, std::size_t offset) const -> void;
			auto Store(const std::vector<Matrix4>& vec, std::size_t offset) const -> void;

		private:
			unsigned m_Name;
			std::size_t m_Size;
	};
}
