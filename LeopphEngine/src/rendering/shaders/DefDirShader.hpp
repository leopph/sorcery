#pragma once

#include "DefLightShader.hpp"
#include "ShaderStage.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../math/Vector.hpp"

#include <string>
#include <vector>



namespace leopph::impl
{
	class DefDirShader : public DefLightShader
	{
		public:
			void SetCameraPosition(const Vector3& pos) const;
			void SetDirLight(const DirectionalLight& dirLight) const;
			void SetCascadeCount(unsigned count) const;
			void SetShadowMaps(const std::vector<int>& shadowMaps) const;
			void SetCascadeFarBounds(const std::vector<float>& bounds) const;
			void SetLightClipMatrices(const std::vector<Matrix4>& mats) const;


		protected:
			DefDirShader(const std::vector<std::string>& vertFlags, const std::vector<std::string>& fragFlags);


		private:
			static std::vector<ShaderStage> GetStages(const std::vector<std::string>& vertFlags, const std::vector<std::string>& fragFlags);

			const static std::string s_ShadowMapArrName;
			const static std::string s_CamPosName;
			const static std::string s_DirLightDirName;
			const static std::string s_DirLightDiffName;
			const static std::string s_DirLightSpecName;
			const static std::string s_CascCountName;
			const static std::string s_ClipMatName;
			const static std::string s_CascBoundsName;

			const int m_ShadowMapArrLoc;
			const int m_CamPosLoc;
			const int m_DirLightDirLoc;
			const int m_DirLightDiffLoc;
			const int m_DirLightSpecLoc;
			const int m_CascCountLoc;
			const int m_ClipMatLoc;
			const int m_CascBoundsLoc;
	};
}
