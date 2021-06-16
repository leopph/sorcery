#include "component.h"
#include <stdexcept>
#include "../instances/instanceholder.h"

namespace leopph
{
	Component::Component() :
		m_Object{ nullptr }
	{
		impl::InstanceHolder::AddComponent(this);
	}

	Component::~Component()
	{
		impl::InstanceHolder::RemoveComponent(this);
	}

	
	leopph::Object& Component::Object()
	{
		return const_cast<leopph::Object&>(const_cast<const Component*>(this)->Object());
	}

	const leopph::Object& Component::Object() const
	{
		if (m_Object == nullptr)
			throw std::runtime_error
			{
				"Component has not been initialized properly!\n"
				"Most likely you are trying to refer to your component's owning object in constructor.\n"
				"If so, use the Component::Init function to complete your initialization.\n"
				"Refer to \"component.h\" for more information."
			};

		return *m_Object;
	}
}