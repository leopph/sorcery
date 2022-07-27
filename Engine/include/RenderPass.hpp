#pragma once

namespace leopph
{
	enum class RenderPass
	{
		Shadow, // In this pass objects are rendered into shadow maps
		Forward, // In this pass objects are rendered in the forward path's opaque pass
		Gbuffer, // In this pass objects are rendered in the deferred path's gbuffer pass
		ForceForward, // In this pass objects are rendered during forward path's opaque pass as well as the deferred path's forward opaque pass
	};
}