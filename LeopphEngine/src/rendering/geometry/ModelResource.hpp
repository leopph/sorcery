#pragma once

#include "../../data/managed/UniqueResource.hpp"
#include "../../events/ModelInstanceCountEvent.hpp"
#include "../../events/handling/EventReceiver.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>



namespace leopph::impl
{
	class AssimpModel;



	class ModelResource final : public UniqueResource, public EventReceiver<ModelInstanceCountEvent>
	{
		public:
			explicit ModelResource(const std::filesystem::path& path);

			ModelResource(const ModelResource& other) = delete;
			ModelResource(ModelResource&& other) = delete;
			ModelResource& operator=(const ModelResource& other) = delete;
			ModelResource& operator=(ModelResource&& other) = delete;

			~ModelResource() override;

			void DrawShaded(ShaderProgram& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const;
			void DrawDepth(const std::vector<Matrix4>& modelMatrices) const;

			[[nodiscard]] bool CastsShadow() const;
			void CastsShadow(bool value);


		private:
			const AssimpModel* const m_AssimpModel;
			bool m_CastsShadow;

			void OnEventReceived(const ModelInstanceCountEvent& event) override;
	};
}
