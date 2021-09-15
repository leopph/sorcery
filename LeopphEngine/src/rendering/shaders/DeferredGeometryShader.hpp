#pragma once

#include "Shader.hpp"
#include "../../math/Matrix.hpp"

#include <string>


namespace leopph::impl
{
	class DeferredGeometryShader final : public Shader
	{
		public:
			DeferredGeometryShader();

			void SetViewProjectionMatrix(const Matrix4& viewProjMat) const;


		private:
			static const std::string s_ViewProjMatName;

			const int m_ViewProjMatLoc;
	};
}