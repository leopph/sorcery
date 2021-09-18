#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStage.hpp"

#include <vector>


namespace leopph::impl
{
	class TextureShader final : public ShaderProgram
	{
		public:
			TextureShader();


		private:
			static std::vector<ShaderStage> GetStages();
	};
}