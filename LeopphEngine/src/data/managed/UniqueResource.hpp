#pragma once

#include <filesystem>


namespace leopph::impl
{
	class UniqueResource
	{
	public:
		UniqueResource(const UniqueResource& other) = delete;
		UniqueResource(UniqueResource&& other) = delete;
		UniqueResource& operator=(const UniqueResource& other) = delete;
		UniqueResource& operator=(UniqueResource&& other) = delete;
		virtual ~UniqueResource() = 0;

		const std::filesystem::path Path;


	protected:
		explicit UniqueResource(std::filesystem::path path);
	};
}