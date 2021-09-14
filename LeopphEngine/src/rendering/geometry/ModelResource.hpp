#pragma once

#include "../../data/managed/UniqueResource.hpp"
#include "../../events/EventReceiver.hpp"
#include "../../events/ModelCountChangedEvent.hpp"

#include "../../math/Matrix.hpp"
#include "../shaders/Shader.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>


namespace leopph::impl
{
	class AssimpModel;


	class ModelResource final : public UniqueResource, public EventReceiver<ModelCountChangedEvent>
	{
	public:
		explicit ModelResource(const std::filesystem::path& path);

		ModelResource(const ModelResource& other) = delete;
		ModelResource(ModelResource&& other) = delete;
		ModelResource& operator=(const ModelResource& other) = delete;
		ModelResource& operator=(ModelResource&& other) = delete;

		~ModelResource() override;

		void DrawShaded(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const;
		void DrawDepth(const std::vector<Matrix4>& modelMatrices) const;

	private:
		const AssimpModel* const m_AssimpModel;

		void OnEventReceived(const ModelCountChangedEvent& event) override;
	};
}