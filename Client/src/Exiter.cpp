#include "Exiter.hpp"


auto Exiter::OnFrameUpdate() -> void
{
	if (leopph::Input::GetKey(leopph::KeyCode::Escape))
	{
		leopph::Exit();
	}
}
