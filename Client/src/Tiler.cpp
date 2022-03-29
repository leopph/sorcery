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
			auto leftSpawn = cam->TransformToViewport(layer.LeftEdge->Transform()->Position())[0] > 0;
			auto rightSpawn = cam->TransformToViewport(layer.RightEdge->Transform()->Position())[0] < 1;

			while (leftSpawn || rightSpawn)
			{
				if (leftSpawn)
				{
					auto const leftExtentX = layer.LeftEdge->GetComponent<ImageSprite>()->Extents()[0];
					auto spawnPos = layer.LeftEdge->Transform()->Position();
					spawnPos[0] -= layer.LeftEdge->Transform()->Position()[0] - 2 * leftExtentX, 0, 0;

					auto const newLeftEdge = new Entity{*layer.LeftEdge};
					newLeftEdge->Transform()->Position(spawnPos);

					layer.LeftEdge = newLeftEdge;
					leftSpawn = cam->TransformToViewport(newLeftEdge->Transform()->Position())[0] > 0;
				}

				if (rightSpawn)
				{
					auto const rightExtentX = layer.RightEdge->GetComponent<ImageSprite>()->Extents()[0];
					auto spawnPos = layer.RightEdge->Transform()->Position();
					spawnPos[0] -= layer.RightEdge->Transform()->Position()[0] + 2 * rightExtentX, 0, 0;

					auto const newRightEdge = new Entity{*layer.RightEdge};
					newRightEdge->Transform()->Position(spawnPos);

					layer.RightEdge = newRightEdge;
					rightSpawn = cam->TransformToViewport(newRightEdge->Transform()->Position())[0] < 1;
				}
			}
		});
	}
}
