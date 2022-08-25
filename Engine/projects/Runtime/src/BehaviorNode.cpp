#include "BehaviorNode.hpp"

#include <algorithm>
#include <utility>


namespace leopph
{
	i32 BehaviorNode::get_update_index() const
	{
		return mUpdateIndex;
	}



	void BehaviorNode::set_update_index(i32 const index)
	{
		mUpdateIndex = index;
	}



	BehaviorNode::BehaviorNode()
	{
		sInstances.emplace_back(this);
	}



	BehaviorNode::BehaviorNode(BehaviorNode const& other) :
		Node{other},
		mUpdateIndex{other.mUpdateIndex}
	{
		sInstances.emplace_back(this);
	}



	BehaviorNode::BehaviorNode(BehaviorNode&& other) noexcept :
		Node{std::move(other)},
		mUpdateIndex{other.mUpdateIndex}
	{
		sInstances.emplace_back(this);
	}



	BehaviorNode::~BehaviorNode()
	{
		std::erase(sInstances, this);
	}



	void BehaviorNode::update_all()
	{
		std::ranges::sort(sInstances, [](auto const* const left, auto const* const right)
		{
			return left->get_update_index() > right->get_update_index();
		});

		for (auto* const behavior : sInstances)
		{
			behavior->on_frame_update();
		}
	}



	std::vector<BehaviorNode*> BehaviorNode::sInstances;
}
