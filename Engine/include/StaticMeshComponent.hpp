#pragma once

#include "Component.hpp"

#include <memory>


namespace leopph
{
	class StaticMesh;
	class StaticMaterial;


	class StaticMeshComponent : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI std::shared_ptr<StaticMesh const> get_mesh() const;
			LEOPPHAPI void set_mesh(std::shared_ptr<StaticMesh> mesh);

			[[nodiscard]] LEOPPHAPI std::shared_ptr<StaticMaterial const> get_material() const;
			LEOPPHAPI void set_material(std::shared_ptr<StaticMaterial> material);

			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);

			void on_init() override;

			LEOPPHAPI explicit StaticMeshComponent(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<StaticMaterial> material);
			LEOPPHAPI StaticMeshComponent(StaticMeshComponent const& other) = default;
			StaticMeshComponent(StaticMeshComponent&&) = delete;

			LEOPPHAPI StaticMeshComponent& operator=(StaticMeshComponent const& other);
			void operator=(StaticMeshComponent&&) = delete;

			LEOPPHAPI ~StaticMeshComponent() override;

		private:
			void try_register() const;
			void try_unregister() const;

			std::shared_ptr<StaticMesh> mMesh;
			std::shared_ptr<StaticMaterial> mMaterial;
			bool mIsCastingShadow{true};
	};
}
