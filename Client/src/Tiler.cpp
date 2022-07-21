#include "Tiler.hpp"

#include <algorithm>

using leopph::Camera;
using leopph::Vector3;
using leopph::ImageSprite;
using leopph::Entity;


namespace demo
{
	Tiler::Tiler(std::span<Layer const> layers) :
		m_Layers{layers.begin(), layers.end()}
	{ }


	auto Tiler::OnFrameUpdate() -> void
	{
		auto const cam = Camera::Current();
		std::ranges::for_each(m_Layers, [cam](auto& layer)
		{
			auto const extentX = layer.Prototype->get_component<ImageSprite>()->Extents()[0];
			auto leftSpawn = cam->TransformToViewport(layer.LeftEdge->get_transform().get_position())[0] > 0;
			auto rightSpawn = cam->TransformToViewport(layer.RightEdge->get_transform().get_position())[0] < 1;

			while (leftSpawn || rightSpawn)
			{
				if (leftSpawn)
				{
					auto spawnPos = layer.LeftEdge->get_transform().get_position();
					spawnPos[0] -= 2 * extentX;

					auto const newLeftEdge = new Entity{*layer.LeftEdge};
					newLeftEdge->get_transform().set_position(spawnPos);

					layer.LeftEdge = newLeftEdge;
					leftSpawn = cam->TransformToViewport(newLeftEdge->get_transform().get_position())[0] > 0;
				}

				if (rightSpawn)
				{
					auto spawnPos = layer.RightEdge->get_transform().get_position();
					spawnPos[0] += 2 * extentX;

					auto const newRightEdge = new Entity{*layer.RightEdge};
					newRightEdge->get_transform().set_position(spawnPos);

					layer.RightEdge = newRightEdge;
					rightSpawn = cam->TransformToViewport(newRightEdge->get_transform().get_position())[0] < 1;
				}
			}
		});
	}
}
