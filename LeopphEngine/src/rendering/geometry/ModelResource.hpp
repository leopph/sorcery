#pragma once

#include "../../data/managed/UniqueResource.hpp"
#include "../../events/handling/EventReceiver.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>



namespace leopph::impl
{
	class AssimpModel;



	class ModelResource final : public UniqueResource
	{
		public:
			explicit ModelResource(const std::filesystem::path& path);

			ModelResource(const ModelResource& other) = delete;
			ModelResource(ModelResource&& other) = delete;
			ModelResource& operator=(const ModelResource& other) = delete;
			ModelResource& operator=(ModelResource&& other) = delete;

			~ModelResource() override;

			void DrawShaded(ShaderProgram& shader, const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices, std::size_t nextFreeTextureUnit);
			void DrawDepth(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices);

			[[nodiscard]] bool CastsShadow() const;
			void CastsShadow(bool value);


		private:
			AssimpModel* const m_AssimpModel;
			bool m_CastsShadow;
	};
}
