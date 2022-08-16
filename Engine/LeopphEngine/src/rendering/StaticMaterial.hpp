#pragma once

#include "PersistentMappedBuffer.hpp"
#include "StaticModelData.hpp"
#include "Texture2D.hpp"
#include "Types.hpp"

#include <memory>
#include <vector>


namespace leopph
{
	class StaticMaterial
	{
		public:
			void bind_and_set_renderstate(u32 index) const;


			StaticMaterial(MaterialData const& data, std::span<std::shared_ptr<Texture2D const> const> textures);

			StaticMaterial(StaticMaterial const& other) = delete;
			StaticMaterial(StaticMaterial&& other) = delete;

			StaticMaterial& operator=(StaticMaterial const& other) = delete;
			StaticMaterial& operator=(StaticMaterial&& other) = delete;

			~StaticMaterial() = default;


		private:
			PersistentMappedBuffer mBuffer;
			bool mCullBackFace;
			std::vector<std::shared_ptr<Texture2D const>> mTextures;
	};
}
