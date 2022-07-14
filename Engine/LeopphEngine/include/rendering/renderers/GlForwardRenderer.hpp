#pragma once

#include "GlRenderer.hpp"


namespace leopph::internal
{
	class GlForwardRenderer final : public GlRenderer
	{
		public:
			auto Render() -> void override;

			GlForwardRenderer();
	};
}
