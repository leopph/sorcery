#include "Behavior.hpp"

#include <algorithm>


namespace leopph
{
	i32 Behavior::get_update_index() const
	{
		return mUpdateIndex;
	}



	void Behavior::set_update_index(i32 const index)
	{
		mUpdateIndex = index;
	}



	Behavior::Behavior()
	{
		sAllBehaviors.emplace_back(this);
	}



	Behavior::~Behavior()
	{
		std::erase(sAllBehaviors, this);
	}



	void Behavior::update_all_behaviors()
	{
		std::ranges::sort(sAllBehaviors, [](auto const* const left, auto const* const right)
		{
			return left->get_update_index() > right->get_update_index();
		});

		for (auto* const behavior : sAllBehaviors)
		{
			behavior->on_frame_update();
		}
	}
}
