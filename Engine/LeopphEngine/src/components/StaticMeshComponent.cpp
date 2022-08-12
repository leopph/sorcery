#include "StaticModelComponent.hpp"

#include "../InternalContext.hpp"
#include "../rendering/Renderer.hpp"

#include <utility>


namespace leopph::internal
{
	bool StaticModelComponent::is_casting_shadow() const
	{
		return mIsCastingShadow;
	}



	void StaticModelComponent::set_casting_shadow(bool const value)
	{
		mIsCastingShadow = value;
	}



	void StaticModelComponent::init(std::shared_ptr<StaticMesh> staticMeshGroup)
	{
		mMesh = std::move(staticMeshGroup);
		GetRenderer()->register_static_mesh(this, mMesh.get());
	}



	StaticModelComponent::StaticModelComponent(StaticModelComponent const& other) :
		mIsCastingShadow{other.mIsCastingShadow},
		mMesh{other.mMesh}
	{
		GetRenderer()->register_static_mesh(this, mMesh.get());
	}



	StaticModelComponent& StaticModelComponent::operator=(StaticModelComponent const& other)
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



	StaticModelComponent::StaticModelComponent(StaticModelComponent&& other) noexcept :
		StaticModelComponent{other}
	{}



	StaticModelComponent& StaticModelComponent::operator=(StaticModelComponent&& other) noexcept
	{
		return *this = other;
	}



	StaticModelComponent::~StaticModelComponent()
	{
		GetRenderer()->unregister_static_mesh(this);
	}
}
