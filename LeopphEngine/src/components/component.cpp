#include "component.h"

#include "../instances/instanceholder.h"
#include "../util/logger.h"

#include <stdexcept>

namespace leopph
{
	Component::~Component() = default;

	
	leopph::Object& Component::Object()
	{
		return const_cast<leopph::Object&>(const_cast<const Component*>(this)->Object());
	}

	const leopph::Object& Component::Object() const
	{
		if (m_Object == nullptr)
		{
			const auto errorMsg{ "Component has not been initialized properly!\n"
				"Most likely you are trying to refer to your component's owning object in constructor.\n"
				"If so, use the Component::Init function to complete your initialization.\n"
				"Refer to \"component.h\" for more information." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::runtime_error{ errorMsg };
		}

		return *m_Object;
	}


	void Component::SetOwnership(leopph::Object* object)
	{
		m_Object = object;
		impl::InstanceHolder::RegisterComponent(this);
		
	}
}