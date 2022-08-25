#pragma once

#include "Node.hpp"

#include <filesystem>
#include <memory>
#include <variant>


namespace leopph
{
	class StaticMesh;
	class StaticMaterial;


	class StaticMeshNode : public Node
	{
		public:
			[[nodiscard]] LEOPPHAPI std::shared_ptr<StaticMesh const> get_mesh() const;
			LEOPPHAPI void set_mesh(std::shared_ptr<StaticMesh> mesh);

			[[nodiscard]] LEOPPHAPI std::shared_ptr<StaticMaterial const> get_material() const;
			LEOPPHAPI void set_material(std::shared_ptr<StaticMaterial> material);

			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);

			LEOPPHAPI explicit StaticMeshNode(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<StaticMaterial> material);
			LEOPPHAPI StaticMeshNode(StaticMeshNode const& other);
			LEOPPHAPI StaticMeshNode(StaticMeshNode&& other) noexcept;

			LEOPPHAPI StaticMeshNode& operator=(StaticMeshNode const& other);
			LEOPPHAPI StaticMeshNode& operator=(StaticMeshNode&& other) noexcept;

			LEOPPHAPI ~StaticMeshNode() override;

		private:
			void try_register() const;
			void try_unregister() const;

			std::shared_ptr<StaticMesh> mMesh;
			std::shared_ptr<StaticMaterial> mMaterial;
			bool mIsCastingShadow{true};
	};


	LEOPPHAPI std::variant<Node*, StaticMeshNode*> create_static_mesh_node_from_model_file(std::filesystem::path const& path);
	LEOPPHAPI StaticMeshNode* create_static_cube_model();
}
