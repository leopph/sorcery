#include "Component.hpp"


namespace leopph
{
	Entity* Component::get_owner() const
	{
		return mOwner;
	}



	void Component::init_all()
	{
		while (!sUninitialized.empty())
		{
			sUninitialized.back()->on_init();
			sUninitialized.pop_back();
		}
	}



	Component::Component()
	{
		sUninitialized.emplace_back(this);
	}



	Component::Component(Component const& other)
	{
		sUninitialized.emplace_back(this);
	}



	Component::Component(Component&&) noexcept
	{
		sUninitialized.emplace_back(this);
	}



	Component::~Component()
	{
		std::erase(sUninitialized, this);
	}



	std::vector<Component*> Component::sUninitialized;
}
