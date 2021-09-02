#pragma once


namespace leopph::impl
{
	class Resource
	{
	public:
		Resource();
		Resource(const Resource& other) = delete;
		Resource(Resource&& other) = delete;
		Resource& operator=(const Resource& other) = delete;
		Resource& operator=(Resource&& other) = delete;
		virtual ~Resource() = 0;
	};
}