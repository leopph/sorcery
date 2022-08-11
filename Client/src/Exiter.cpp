#include "Exiter.hpp"


void Exiter::OnFrameUpdate()
{
	if (leopph::Input::GetKey(leopph::KeyCode::Escape))
	{
		leopph::exit();
	}
}
