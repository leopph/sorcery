#include "StaticMeshComponent.hpp"

#include "Context.hpp"
#include "../../../include/Import.hpp"
#include "../rendering/Renderer.hpp"
#include "../rendering/StaticMesh.hpp"


namespace leopph
{
	std::shared_ptr<StaticMesh const> StaticMeshComponent::get_mesh() const
	{
		return mMesh;
	}



	void StaticMeshComponent::set_mesh(std::shared_ptr<StaticMesh> mesh)
	{
		try_unregister();
		mMesh = std::move(mesh);
		try_register();
	}



	std::shared_ptr<StaticMaterial const> StaticMeshComponent::get_material() const
	{
		return mMaterial;
	}



	void StaticMeshComponent::set_material(std::shared_ptr<StaticMaterial> material)
	{
		try_unregister();
		mMaterial = std::move(material);
		try_unregister();
	}



	bool StaticMeshComponent::is_casting_shadow() const
	{
		return mIsCastingShadow;
	}



	void StaticMeshComponent::set_casting_shadow(bool const value)
	{
		mIsCastingShadow = value;
	}



	void StaticMeshComponent::on_init()
	{
		try_register();
	}



	StaticMeshComponent::StaticMeshComponent(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<StaticMaterial> material) :
		mMesh{std::move(mesh)},
		mMaterial{std::move(material)}
	{}



	StaticMeshComponent& StaticMeshComponent::operator=(StaticMeshComponent const& other)
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



	StaticMeshComponent::~StaticMeshComponent()
	{
		try_unregister();
	}



	void StaticMeshComponent::try_register() const
	{
		if (mMesh)
		{
			mMesh->register_entity(get_owner());
		}

		if (mMesh && mMaterial)
		{
			internal::get_renderer().register_mesh_for_material(mMaterial, mMesh);
		}
	}



	void StaticMeshComponent::try_unregister() const
	{
		if (mMesh)
		{
			mMesh->unregister_entity(get_owner());
		}

		if (mMesh && mMaterial)
		{
			internal::get_renderer().unregister_mesh_for_material(mMaterial, mMesh);
		}
	}



	void attach_static_mesh_component_from_model_file(Entity* const entity, std::filesystem::path const& path)
	{
		for (auto const renderData = generate_render_structures(import_static_model(path));
		     auto const& [mesh, material] : renderData)
		{
			entity->attach_component<StaticMeshComponent>(mesh, material);
		}
	}
}
