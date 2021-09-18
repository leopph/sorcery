#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStage.hpp"

#include <vector>


namespace leopph::impl
{
	class ForObjShader final : public ShaderProgram
	{
		public:
			ForObjShader();


		private:
			static std::vector<ShaderStage> GetStages();
	};
}