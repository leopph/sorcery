#include "Parallaxer.hpp"

#include <iostream>

using leopph::Vector3;


namespace demo
{
	Parallaxer::Parallaxer(leopph::ComponentPtr<leopph::Camera> camera, std::span<Layer> layers) :
		m_Layers{layers.begin(), layers.end()},
		m_TargetCamera{std::move(camera)},
		m_PrevCamPosX{m_TargetCamera->Entity()->Transform()->Position()[0]}
	{
		std::cout << m_TargetCamera.get() << '\n';
	}


	auto Parallaxer::OnFrameUpdate() -> void
	{
		std::cout << m_TargetCamera.get() << '\n';

		std::ranges::for_each(m_Layers, [this](auto const& layer)
		{
			layer.Transform->Position(layer.Transform->Position() + Vector3{(m_TargetCamera->Entity()->Transform()->Position()[0] - m_PrevCamPosX) * layer.Depth, 0, 0});
		});

		m_PrevCamPosX = m_TargetCamera->Entity()->Transform()->Position()[0];
	}
}
