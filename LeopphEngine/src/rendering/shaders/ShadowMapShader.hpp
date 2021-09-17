#pragma once

#include "Shader.hpp"
#include "../../math/Matrix.hpp"

#include <string>


namespace leopph::impl
{
	class ShadowMapShader final : public Shader
	{
		public:
			ShadowMapShader();

			void SetLightWorldToClipMatrix(const Matrix4& mat) const;


		private:
			static const std::string s_WorldToClipMatName;

			const int m_WorldToClipMatLoc;
	};
}