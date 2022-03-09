#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/FileMeshDataGroup.hpp"

#include <utility>


namespace leopph
{
	Model::Model(std::filesystem::path path) :
		RenderComponent{GetMeshData(path)},
		m_Path{std::move(path)}
	{}


	auto Model::GetMeshData(const std::filesystem::path& path) const -> std::shared_ptr<internal::MeshDataGroup>
	{
		if (auto p{internal::DataManager::Instance().FindMeshDataGroup(path.generic_string())};
			p != nullptr)
		{
			return p;
		}
		return std::make_shared<internal::FileMeshDataGroup>(path.generic_string());
	}
}
