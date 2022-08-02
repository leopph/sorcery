#pragma once

#include "Types.hpp"

#include <vector>


namespace leopph
{
	class CubeMesh
	{
		public:
			CubeMesh();

			CubeMesh(CubeMesh const&) = delete;
			void operator=(CubeMesh const&) = delete;

			CubeMesh(CubeMesh&&) = delete;
			void operator=(CubeMesh&&) = delete;

			~CubeMesh();

			void draw() const;

		private:
			u32 mVao;
			u32 mVbo;
			u32 mIbo;

			static std::vector<f32> const sVertices;
			static std::vector<u8> const sIndices;
	};
}
