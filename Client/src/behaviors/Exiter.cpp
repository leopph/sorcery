#include "Exiter.hpp"


Exiter::Exiter(leopph::Entity* const entity) :
	Behavior{entity}
{}


auto Exiter::OnFrameUpdate() -> void
{
	if (leopph::Input::GetKey(leopph::KeyCode::Escape))
	{
		leopph::Exit();
	}
}
