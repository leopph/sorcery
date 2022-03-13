#include "Cube.hpp"

#include "../../data/DataManager.hpp"

#include <memory>
#include <source_location>
#include <utility>

using leopph::internal::Vertex;


namespace leopph
{
	Cube::Cube() :
		RenderComponent{GetMeshData()}
	{}


	auto Cube::GetMeshData() -> std::shared_ptr<const internal::MeshDataGroup>
	{
		if (auto p{internal::DataManager::Instance().FindMeshDataGroup(s_MeshDataId)}; p != nullptr)
		{
			return p;
		}

		return std::make_shared<CubeMeshDataGroup>();
	}


	Cube::CubeMeshDataGroup::CubeMeshDataGroup() :
		MeshDataGroup{s_MeshDataId}
	{
		std::vector<Vertex> vertices{
			// Back face
			Vertex{Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0, 0, -1}, Vector2{}}, // back bottom left
			Vertex{Vector3{-0.5f, 0.5f, -0.5f}, Vector3{0, 0, -1}, Vector2{}}, // back top left
			Vertex{Vector3{0.5f, 0.5f, -0.5f}, Vector3{0, 0, -1}, Vector2{}}, // back top right
			Vertex{Vector3{0.5f, -0.5f, -0.5f}, Vector3{0, 0, -1}, Vector2{}}, // back bottom right

			// Front face
			Vertex{Vector3{-0.5f, -0.5f, 0.5f}, Vector3{0, 0, 1}, Vector2{}}, // front bottom left
			Vertex{Vector3{-0.5f, 0.5f, 0.5f}, Vector3{0, 0, 1}, Vector2{}}, // front top left
			Vertex{Vector3{0.5f, 0.5f, 0.5f}, Vector3{0, 0, 1}, Vector2{}}, // front top right
			Vertex{Vector3{0.5f, -0.5f, 0.5f}, Vector3{0, 0, 1}, Vector2{}}, // front bottom right

			// Top face
			Vertex{Vector3{-0.5f, 0.5f, 0.5f}, Vector3{0, 1, 0}, Vector2{}}, // top left front
			Vertex{Vector3{-0.5f, 0.5f, -0.5f}, Vector3{0, 1, 0}, Vector2{}}, // top left back
			Vertex{Vector3{0.5f, 0.5f, 0.5f}, Vector3{0, 1, 0}, Vector2{}}, // top right front
			Vertex{Vector3{0.5f, 0.5f, -0.5f}, Vector3{0, 1, 0}, Vector2{}}, // top right back

			// Bottom face
			Vertex{Vector3{-0.5f, -0.5f, 0.5f}, Vector3{0, -1, 0}, Vector2{}}, // bottom left front
			Vertex{Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0, -1, 0}, Vector2{}}, // bottom left back
			Vertex{Vector3{0.5f, -0.5f, 0.5f}, Vector3{0, -1, 0}, Vector2{}}, // bottom right front
			Vertex{Vector3{0.5f, -0.5f, -0.5f}, Vector3{0, -1, 0}, Vector2{}}, // bottom right back

			// Right face
			Vertex{Vector3{0.5f, 0.5f, 0.5f}, Vector3{1, 0, 0}, Vector2{}}, // right top front
			Vertex{Vector3{0.5f, 0.5f, -0.5f}, Vector3{1, 0, 0}, Vector2{}}, // right top back
			Vertex{Vector3{0.5f, -0.5f, 0.5f}, Vector3{1, 0, 0}, Vector2{}}, // right bottom front
			Vertex{Vector3{0.5f, -0.5f, -0.5f}, Vector3{1, 0, 0}, Vector2{}}, // right bottom back

			// Left face
			Vertex{Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-1, 0, 0}, Vector2{}}, // left top front
			Vertex{Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-1, 0, 0}, Vector2{}}, // left top back
			Vertex{Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-1, 0, 0}, Vector2{}}, // left bottom front
			Vertex{Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-1, 0, 0}, Vector2{}}, // left bottom back
		};

		std::vector<unsigned> indices{
			// Back face
			0, 3, 2, 0, 2, 1,
			// Front face
			7, 4, 6, 6, 4, 5,
			// Top face
			10, 8, 9, 9, 11, 10,
			// Bottom face
			12, 14, 13, 15, 13, 14,
			// Right face
			16, 17, 18, 19, 18, 17,
			// Left face
			21, 20, 22, 22, 23, 21
		};

		auto material = s_Material.lock();
		if (!material)
		{
			s_Material = material = std::make_shared<Material>(Color{255, 255, 255}, Color{0, 0, 0}, nullptr, nullptr, 0.f, true);
		}

		Data().emplace_back(std::move(vertices), std::move(indices), std::move(material));
	}


	const std::string Cube::s_MeshDataId{
		[]
		{
			const auto srcLoc{std::source_location::current()};
			return std::string{srcLoc.file_name()}.append(std::to_string(srcLoc.line())).append(std::to_string(srcLoc.column())).append("CubeMeshDataId");
		}()
	};

	std::weak_ptr<Material> Cube::CubeMeshDataGroup::s_Material;
}
