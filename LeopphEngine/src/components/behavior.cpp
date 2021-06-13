#include "behavior.h"

namespace leopph
{
	// constructor
	Behavior::Behavior(leopph::Object& object) :
		Component{ object }
	{
		s_Behaviors.emplace(this);
	}

	// destructor
	Behavior::~Behavior()
	{
		s_Behaviors.erase(this);
	}


	std::set<Behavior*> Behavior::s_Behaviors{};


	void Behavior::UpdateAll()
	{
		for (Behavior* behavior : s_Behaviors)
			behavior->OnFrameUpdate();
	}
}