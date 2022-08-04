#pragma once

#include "MaterialData.hpp"
#include "PersistentMappedBuffer.hpp"
#include "Texture2D.hpp"
#include "Types.hpp"

#include <memory>
#include <vector>


namespace leopph
{
	class StaticMaterial
	{
		public:
			explicit StaticMaterial(MaterialData const& data);

			StaticMaterial(StaticMaterial const& other) = delete;
			StaticMaterial& operator=(StaticMaterial const& other) = delete;

			StaticMaterial(StaticMaterial&& other) = delete;
			StaticMaterial& operator=(StaticMaterial&& other) = delete;

			~StaticMaterial() = default;

			void bind_and_set_renderstate(u32 index) const;

		private:
			PersistentMappedBuffer mBuffer;
			std::vector<std::unique_ptr<Texture2D>> mTextures;
			bool mCullBackFace;
	};
}
