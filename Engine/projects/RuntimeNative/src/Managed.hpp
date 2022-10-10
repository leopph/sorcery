#pragma once

#include "Core.hpp"

#include <vector>


struct _MonoMethod;
struct _MonoObject;
struct _MonoClass;

typedef _MonoMethod MonoMethod;
typedef _MonoObject MonoObject;
typedef _MonoClass MonoClass;


namespace leopph
{
	LEOPPHAPI void initialize_managed_runtime();


	class ManagedEntityInstance
	{
	private:
		u32 mGcHandle;
		MonoMethod* mTickMethod;

	public:
		ManagedEntityInstance(MonoClass* monoClass);
		ManagedEntityInstance(ManagedEntityInstance&& other) noexcept;

		~ManagedEntityInstance();

	public:
		LEOPPHAPI void tick() const;

	private:
		static void check_and_handle_exception(MonoObject* exception);
	};


	LEOPPHAPI extern std::vector<ManagedEntityInstance> managedEntityInstances;
}