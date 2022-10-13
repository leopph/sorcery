#pragma once

#include "Core.hpp"

struct _MonoObject;
typedef _MonoObject MonoObject;

namespace leopph
{
	class ManagedAccessObject
	{
	private:
		static u64 sNextId;

	public:
		u64 const id{sNextId++};
		u32 const managedObjectHandle;

		explicit ManagedAccessObject(MonoObject* managedObject);
		ManagedAccessObject(ManagedAccessObject const&) = delete;

		virtual ~ManagedAccessObject();
	};

	void store_mao(ManagedAccessObject* mao);
	void destroy_mao(u64 id);
	ManagedAccessObject* get_mao_by_id(u64 id);
	void destroy_all_maos();
}