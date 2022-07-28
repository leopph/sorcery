#include "StaticMeshGroupComponent.hpp"

#include "../InternalContext.hpp"
#include "../rendering/Renderer.hpp"

#include <utility>


namespace leopph::internal
{
	bool StaticMeshGroupComponent::is_casting_shadow() const
	{
		return mIsCastingShadow;
	}



	void StaticMeshGroupComponent::set_casting_shadow(bool const value)
	{
		mIsCastingShadow = value;
	}



	StaticMeshGroupComponent::StaticMeshGroupComponent(std::shared_ptr<StaticMeshGroup> staticMeshGroup) :
		mMeshGroup{std::move(staticMeshGroup)}
	{
		GetRenderer()->register_static_mesh_group(this, mMeshGroup.get());
	}



	StaticMeshGroupComponent::StaticMeshGroupComponent(StaticMeshGroupComponent const& other) :
		mIsCastingShadow{other.mIsCastingShadow},
		mMeshGroup{other.mMeshGroup}
	{
		GetRenderer()->register_static_mesh_group(this, mMeshGroup.get());
	}



	StaticMeshGroupComponent& StaticMeshGroupComponent::operator=(StaticMeshGroupComponent const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		GetRenderer()->unregister_static_mesh_group(this);
		mMeshGroup = other.mMeshGroup;
		GetRenderer()->register_static_mesh_group(this, mMeshGroup.get());

		return *this;
	}



	StaticMeshGroupComponent::StaticMeshGroupComponent(StaticMeshGroupComponent&& other) noexcept :
		StaticMeshGroupComponent{other}
	{}



	StaticMeshGroupComponent& StaticMeshGroupComponent::operator=(StaticMeshGroupComponent&& other) noexcept
	{
		return *this = other;
	}



	StaticMeshGroupComponent::~StaticMeshGroupComponent()
	{
		GetRenderer()->unregister_static_mesh_group(this);
	}
}
