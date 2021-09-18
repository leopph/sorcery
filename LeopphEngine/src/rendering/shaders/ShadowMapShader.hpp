#pragma once

#include "ShaderProgram.hpp"
#include "../../math/Matrix.hpp"

#include <string>
#include <vector>


namespace leopph::impl
{
	class ShadowMapShader final : public ShaderProgram
	{
		public:
			ShadowMapShader();

			void SetLightWorldToClipMatrix(const Matrix4& mat) const;


		private:
			static std::vector<ShaderStage> GetStages();

			static const std::string s_WorldToClipMatName;

			const int m_WorldToClipMatLoc;
	};
}