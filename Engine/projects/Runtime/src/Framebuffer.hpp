#pragma once

#include "Types.hpp"

#include <vector>


namespace leopph
{
	struct Framebuffer
	{
		u32 name; // framebuffer object handle
		u32 depthStencilAtt; // depth stencil attachment
		u32 width;
		u32 height;
		std::vector<u32> clrAtts; // color attachments
	};
}