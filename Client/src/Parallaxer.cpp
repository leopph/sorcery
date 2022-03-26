#include "Parallaxer.hpp"

using leopph::Vector3;


namespace demo
{
	Parallaxer::Parallaxer(leopph::ComponentPtr<leopph::Camera> camera, std::span<Layer> layers) :
		m_Layers{layers.begin(), layers.end()},
		m_TargetCamera{std::move(camera)},
		m_PrevCamPosX{m_TargetCamera->Entity()->Transform()->Position()[0]}
	{ }


	auto Parallaxer::OnFrameUpdate() -> void
	{
		std::ranges::for_each(m_Layers, [this](auto const& layer)
		{
			layer.Transform->Position(layer.Transform->Position() + Vector3{(m_TargetCamera->Entity()->Transform()->Position()[0] - m_PrevCamPosX) * layer.SpeedMult, 0, 0});
		});

		m_PrevCamPosX = m_TargetCamera->Entity()->Transform()->Position()[0];
	}
}
