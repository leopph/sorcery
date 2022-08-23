#include "Exiter.hpp"


void Exiter::on_frame_update()
{
	if (leopph::Input::get_key(leopph::KeyCode::Escape))
	{
		leopph::exit();
	}
}
