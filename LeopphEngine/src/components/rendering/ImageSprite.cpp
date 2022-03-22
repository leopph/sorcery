#include "ImageSprite.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	ImageSprite::ImageSprite(std::filesystem::path const& src, unsigned const ppi) :
		RenderComponent{GetMeshGroup(src, ppi)}
	{}


	auto ImageSprite::GetMeshGroup(std::filesystem::path const& src, unsigned const ppi) -> std::shared_ptr<internal::MeshGroup const>
	{
		auto& dataManager = internal::DataManager::Instance();
		auto const meshId{GetMeshId(src, ppi)};

		if (auto meshGroup = dataManager.FindMeshGroup(meshId))
		{
			return meshGroup;
		}

		auto texture = std::make_shared<Texture>(Image{}); // TODO

		// The width of the required rectangle.
		auto const rectW = static_cast<float>(texture->Width()) / static_cast<float>(ppi);
		// The height of the required rectangle.
		auto const rectH = static_cast<float>(texture->Height()) / static_cast<float>(ppi);

		std::vector vertices
		{
			internal::Vertex{Vector3{-rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{0, 1}}, // top left
			internal::Vertex{Vector3{-rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{0, 0}}, // bottom left
			internal::Vertex{Vector3{rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{1, 0}}, // bottom right
			internal::Vertex{Vector3{rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{1, 1}} // top right
		};

		std::vector<unsigned> indices
		{
			0, 1, 3, 1, 2, 3
		};

		auto material = std::make_shared<Material>(Color{255, 255, 255}, Color{0, 0, 0}, std::move(texture), nullptr, nullptr, 0.f, 1.f, false);

		auto meshes = std::make_shared<std::vector<internal::Mesh>>();
		meshes->emplace_back(std::move(vertices), std::move(indices), std::move(material));
		auto meshGroup = std::make_shared<internal::MeshGroup>(meshId, std::move(meshes));
		dataManager.RegisterMeshGroup(meshGroup);
		return meshGroup;
	}


	auto ImageSprite::GetMeshId(std::filesystem::path const& src, unsigned const ppi) -> std::string
	{
		return src.generic_string() + "+ppi=" + std::to_string(ppi);
	}
}
