#pragma once

#include "Types.hpp"

#include <vector>


namespace leopph
{
	class QuadMesh
	{
		public:
			QuadMesh();

			QuadMesh(QuadMesh const& other) = delete;
			QuadMesh& operator=(QuadMesh const& other) = delete;

			QuadMesh(QuadMesh&& other) = delete;
			QuadMesh& operator=(QuadMesh&& other) = delete;

			~QuadMesh();

			void draw() const;

		private:
			u32 mVao;
			u32 mVbo;
			u32 mIbo;

			static std::vector<f32> const sVertices;
			static std::vector<u8> const sIndices;
	};
}
