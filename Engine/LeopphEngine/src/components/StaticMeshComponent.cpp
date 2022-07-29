#include "StaticMeshComponent.hpp"

#include "../InternalContext.hpp"
#include "../rendering/Renderer.hpp"

#include <utility>


namespace leopph::internal
{
	bool StaticMeshComponent::is_casting_shadow() const
	{
		return mIsCastingShadow;
	}



	void StaticMeshComponent::set_casting_shadow(bool const value)
	{
		mIsCastingShadow = value;
	}



	void StaticMeshComponent::init(std::shared_ptr<StaticMesh> staticMeshGroup)
	{
		mMesh = std::move(staticMeshGroup);
		GetRenderer()->register_static_mesh(this, mMesh.get());
	}



	StaticMeshComponent::StaticMeshComponent(StaticMeshComponent const& other) :
		mIsCastingShadow{other.mIsCastingShadow},
		mMesh{other.mMesh}
	{
		GetRenderer()->register_static_mesh(this, mMesh.get());
	}



	StaticMeshComponent& StaticMeshComponent::operator=(StaticMeshComponent const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		GetRenderer()->unregister_static_mesh(this);
		mMesh = other.mMesh;
		GetRenderer()->register_static_mesh(this, mMesh.get());

		return *this;
	}



	StaticMeshComponent::StaticMeshComponent(StaticMeshComponent&& other) noexcept :
		StaticMeshComponent{other}
	{}



	StaticMeshComponent& StaticMeshComponent::operator=(StaticMeshComponent&& other) noexcept
	{
		return *this = other;
	}



	StaticMeshComponent::~StaticMeshComponent()
	{
		GetRenderer()->unregister_static_mesh(this);
	}
}
