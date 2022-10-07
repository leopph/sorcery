#include "Managed.hpp"

#include <mono/metadata/appdomain.h>

#include <cassert>

namespace leopph
{
	ManagedNodeInstance::ManagedNodeInstance(MonoClass* const monoClass) :
		mInstance{mono_object_new(mono_domain_get(), monoClass)},
		mGcHandle{mono_gchandle_new(mInstance, true)},
		mTickMethod{mono_class_get_method_from_name(monoClass, "Tick", 0)}
	{
		assert(mInstance);
		assert(mGcHandle);

		if (MonoMethod* defaultCtor = mono_class_get_method_from_name(monoClass, ".ctor", 0))
		{
			MonoObject* exception;
			mono_runtime_invoke(defaultCtor, mInstance, nullptr, &exception);
			check_and_handle_exception(exception);
		}
	}


	ManagedNodeInstance::ManagedNodeInstance(ManagedNodeInstance&& other) noexcept :
		mInstance{other.mInstance},
		mGcHandle{other.mGcHandle},
		mTickMethod{other.mTickMethod}
	{
		other.mInstance = nullptr;
		other.mGcHandle = 0;
		other.mTickMethod = nullptr;
	}


	ManagedNodeInstance::~ManagedNodeInstance()
	{
		if (mGcHandle)
		{
			mono_gchandle_free(mGcHandle);
		}
	}


	void ManagedNodeInstance::tick() const
	{
		if (!mTickMethod)
		{
			return;
		}

		MonoObject* exception;
		mono_runtime_invoke(mTickMethod, mInstance, nullptr, &exception);
		check_and_handle_exception(exception);
	}


	void ManagedNodeInstance::check_and_handle_exception(MonoObject* const exception)
	{
		if (exception)
		{
			mono_print_unhandled_exception(exception);
		}
	}
}