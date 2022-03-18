#include "ImageSprite.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	ImageSprite::ImageSprite(const std::filesystem::path& src, const unsigned ppi) :
		RenderComponent{GetMeshData(src, ppi)}
	{}


	auto ImageSprite::GetMeshData(const std::filesystem::path& src, const unsigned ppi) -> std::shared_ptr<const internal::MeshDataGroup>
	{
		if (auto p{internal::DataManager::Instance().FindMeshDataGroup(MeshDataGroup::GetMeshId(src, ppi))}; p)
		{
			return p;
		}

		return std::make_shared<MeshDataGroup>(src, ppi);
	}


	ImageSprite::MeshDataGroup::MeshDataGroup(const std::filesystem::path& src, const unsigned ppi) :
		internal::MeshDataGroup{GetMeshId(src, ppi)}
	{
		auto texture{internal::DataManager::Instance().FindTexture(src)};
		if (!texture)
		{
			texture = std::make_shared<Texture>(src);
		}

		// The width of the required rectangle.
		const auto rectW = static_cast<float>(texture->Width()) / static_cast<float>(ppi);
		// The height of the required rectangle.
		const auto rectH = static_cast<float>(texture->Height()) / static_cast<float>(ppi);

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

		auto material = std::make_shared<Material>(Color{255, 255, 255}, Color{0, 0, 0}, std::move(texture), nullptr, 0.f, false);

		m_MeshData.emplace_back(std::move(vertices), std::move(indices), std::move(material));
	}


	auto ImageSprite::MeshDataGroup::GetMeshId(const std::filesystem::path& src, const unsigned ppi) -> std::string
	{
		return src.generic_string() + "+ppi=" + std::to_string(ppi);
	}
}
