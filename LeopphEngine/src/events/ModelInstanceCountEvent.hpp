#pragma once

#include "Event.hpp"

#include <cstddef>


namespace leopph::impl
{
	class ModelResource;


	class ModelInstanceCountEvent : public Event
	{
	public:
		ModelInstanceCountEvent(const ModelResource* modelResource);

		const std::size_t Count;
		const ModelResource* const Model;
	};
}