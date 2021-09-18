#pragma once

#include "DefLightShader.hpp"
#include "ShaderStage.hpp"
#include "../../math/Vector.hpp"

#include <string>


namespace leopph::impl
{
	class DefAmbShader final : public DefLightShader
	{
		public:
			DefAmbShader();

			void SetAmbientLight(const Vector3& ambientLight) const;


		private:
			static std::vector<ShaderStage> GetStages();

			static const std::string s_AmbLightName;

			const int m_AmbLightLoc;
	};
}