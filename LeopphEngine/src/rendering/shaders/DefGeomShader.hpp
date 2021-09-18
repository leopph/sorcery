#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStage.hpp"
#include "../../math/Matrix.hpp"

#include <string>
#include <vector>


namespace leopph::impl
{
	class DefGeomShader final : public ShaderProgram
	{
		public:
			DefGeomShader();

			void SetViewProjectionMatrix(const Matrix4& viewProjMat) const;


		private:
			static std::vector<ShaderStage> GetStages();

			static const std::string s_ViewProjMatName;

			const int m_ViewProjMatLoc;
	};
}