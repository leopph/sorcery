#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStage.hpp"

#include <vector>


namespace leopph::impl
{
	class SkyboxShader final : public ShaderProgram
	{
		public:
			SkyboxShader();


		private:
			static std::vector<ShaderStage> GetStages();
	};
}