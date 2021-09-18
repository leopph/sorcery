#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStage.hpp"

#include <string>
#include <vector>


namespace leopph::impl
{
	class DefLightShader : public ShaderProgram
	{
		public:
			void SetPositionTexture(int unit) const;
			void SetNormalTexture(int unit) const;
			void SetAmbientTexture(int unit) const;
			void SetDiffuseTexture(int unit) const;
			void SetSpecularTexture(int unit) const;
			void SetShineTexture(int unit) const;


		protected:
			DefLightShader(const std::vector<ShaderStage>& stages);


		private:
			const static std::string s_PosTexName;
			const static std::string s_NormTexName;
			const static std::string s_AmbTexName;
			const static std::string s_DiffTexName;
			const static std::string s_SpecTexName;
			const static std::string s_ShineTexName;

			const int m_PosTexLoc;
			const int m_NormTexLoc;
			const int m_AmbTexLoc;
			const int m_DiffTexLoc;
			const int m_SpecTexLoc;
			const int m_ShineTexLoc;
	};
}