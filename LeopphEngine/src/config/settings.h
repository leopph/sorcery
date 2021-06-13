#pragma once

namespace leopph
{
	// TODO
	class Settings
	{
	public:
		enum class GraphicsAPI
		{
			OpenGL
		};


		static inline GraphicsAPI RenderAPI = GraphicsAPI::OpenGL;
	};
}