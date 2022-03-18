#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/FileMeshGroup.hpp"

#include <utility>


namespace leopph
{
	Model::Model(std::filesystem::path path) :
		RenderComponent{GetMeshGroup(path)},
		m_Path{std::move(path)}
	{}


	auto Model::GetMeshGroup(const std::filesystem::path& path) const -> std::shared_ptr<internal::MeshGroup>
	{
		if (auto p{internal::DataManager::Instance().FindMeshGroup(path.generic_string())};
			p != nullptr)
		{
			return p;
		}
		return std::make_shared<internal::FileMeshGroup>(path.generic_string());
	}
}
