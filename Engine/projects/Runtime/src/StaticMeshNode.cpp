#include "StaticMeshNode.hpp"

#include "Context.hpp"
#include "Math.hpp"
#include "ModelImport.hpp"
#include "Renderer.hpp"
#include "StaticMesh.hpp"
#include "Types.hpp"

#include <utility>


namespace leopph
{
	std::shared_ptr<StaticMesh const> StaticMeshNode::get_mesh() const
	{
		return mMesh;
	}



	void StaticMeshNode::set_mesh(std::shared_ptr<StaticMesh> mesh)
	{
		try_unregister();
		mMesh = std::move(mesh);
		try_register();
	}



	std::shared_ptr<StaticMaterial const> StaticMeshNode::get_material() const
	{
		return mMaterial;
	}



	void StaticMeshNode::set_material(std::shared_ptr<StaticMaterial> material)
	{
		try_unregister();
		mMaterial = std::move(material);
		try_unregister();
	}



	bool StaticMeshNode::is_casting_shadow() const
	{
		return mIsCastingShadow;
	}



	void StaticMeshNode::set_casting_shadow(bool const value)
	{
		mIsCastingShadow = value;
	}



	StaticMeshNode::StaticMeshNode(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<StaticMaterial> material) :
		mMesh{std::move(mesh)},
		mMaterial{std::move(material)}
	{
		try_register();
	}



	StaticMeshNode::StaticMeshNode(StaticMeshNode const& other) :
		Node{other},
		mMesh{other.mMesh},
		mMaterial{other.mMaterial}
	{
		try_register();
	}



	StaticMeshNode::StaticMeshNode(StaticMeshNode&& other) noexcept :
		Node{std::move(other)},
		mMesh{other.mMesh},
		mMaterial{other.mMaterial}
	{
		try_register();
	}



	StaticMeshNode& StaticMeshNode::operator=(StaticMeshNode const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		try_unregister();
		mMesh = other.mMesh;
		mMaterial = other.mMaterial;
		try_register();

		return *this;
	}



	StaticMeshNode& StaticMeshNode::operator=(StaticMeshNode&& other) noexcept
	{
		return *this = other;
	}



	StaticMeshNode::~StaticMeshNode()
	{
		try_unregister();
	}



	void StaticMeshNode::try_register() const
	{
		if (mMesh)
		{
			mMesh->register_entity(this);
		}

		if (mMesh && mMaterial)
		{
			internal::get_renderer().register_mesh_for_material(mMaterial, mMesh);
		}
	}



	void StaticMeshNode::try_unregister() const
	{
		if (mMesh)
		{
			mMesh->unregister_entity(this);
		}

		if (mMesh && mMaterial)
		{
			internal::get_renderer().unregister_mesh_for_material(mMaterial, mMesh);
		}
	}



	std::variant<Node*, StaticMeshNode*> create_static_mesh_node_from_model_file(std::filesystem::path const& path)
	{
		auto const renderData = generate_render_structures(import_static_model(path));

		if (renderData.size() == 1)
		{
			return new StaticMeshNode{renderData[0].first, renderData[0].second};
		}

		auto* const group = new Node{};

		for (auto const& [mesh, material] : renderData)
		{
			auto* const child = new StaticMeshNode{mesh, material};
			child->set_parent(group);
		}

		return group;
	}



	StaticMeshNode* create_static_cube_model()
	{
		std::vector vertices
		{
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

		std::vector<u32> indices
		{
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

		StaticMeshData meshData
		{
			.vertices = std::move(vertices),
			.indices = std::move(indices),
			.boundingBox = AABB{Vector3{-0.5}, Vector3{0.5}}
		};

		MaterialData materialData
		{
			.diffuseColor = Color{255, 255, 255, 255},
			.diffuseMapIndex = std::nullopt,
			.specularColor = Color{255, 255, 255},
			.specularMapIndex = std::nullopt,
			.gloss = 32,
			.alphaThreshold = 0,
			.cullBackFace = true,
			.isTransparent = false
		};

		StaticModelData modelData;
		modelData.meshes.emplace_back(std::move(meshData));
		modelData.materials.emplace_back(materialData);

		auto const renderData = generate_render_structures(modelData);
		return new StaticMeshNode{renderData[0].first, renderData[0].second};
	}
}
