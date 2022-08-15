#include "Exiter.hpp"


void Exiter::on_frame_update()
{
	if (leopph::Input::GetKey(leopph::KeyCode::Escape))
	{
		leopph::exit();
	}
}
