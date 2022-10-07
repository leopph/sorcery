#pragma once

#include <mono/metadata/object.h>

namespace leopph
{
	class ManagedNodeInstance
	{
	private:
		MonoObject* mInstance;
		unsigned mGcHandle;
		MonoMethod* mTickMethod;

	public:
		ManagedNodeInstance(MonoClass* monoClass);
		ManagedNodeInstance(ManagedNodeInstance&& other) noexcept;

		~ManagedNodeInstance();

	public:
		void tick() const;

	private:
		static void check_and_handle_exception(MonoObject* exception);
	};
}