#pragma once

#include <Leopph.hpp>
#include <span>
#include <vector>


namespace demo
{
	class Tiler final : public leopph::Behavior
	{
		public:
			struct Layer
			{
				leopph::Entity* Prototype;
				leopph::Entity* LeftEdge;
				leopph::Entity* RightEdge;
			};


			explicit Tiler(std::span<Layer const> layers);

			void on_frame_update() override;

		private:
			std::vector<Layer> m_Layers;
	};
}
