#pragma once

#include "Event.hpp"

#include <cstddef>


namespace leopph::impl
{
	class ModelResource;


	class ModelCountChangedEvent : public Event
	{
	public:
		ModelCountChangedEvent(const ModelResource* modelResource);

		const std::size_t Count;
		const ModelResource* const Model;
	};
}