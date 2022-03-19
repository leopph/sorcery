#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/ModelParser.hpp"

#include <utility>


namespace leopph
{
	Model::Model(std::filesystem::path path) :
		RenderComponent{GetMeshGroup(path)},
		m_Path{std::move(path)}
	{}


	auto Model::GetMeshGroup(std::filesystem::path const& path) const -> std::shared_ptr<internal::MeshGroup const>
	{
		auto const meshId = path.generic_string();

		if (auto meshGroup = internal::DataManager::Instance().FindMeshGroup(meshId))
		{
			return meshGroup;
		}

		auto meshGroup = std::make_shared<internal::MeshGroup const>(meshId, std::make_shared<std::vector<internal::Mesh>>(internal::ModelParser{}(path)));
		internal::DataManager::Instance().RegisterMeshGroup(meshGroup);
		return meshGroup;
	}
}
