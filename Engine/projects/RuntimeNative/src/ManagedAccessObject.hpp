#pragma once

#include "Core.hpp"

namespace leopph
{
	class ManagedAccessObject
	{
	private:
		static u64 sNextId;

	public:
		u64 const id{sNextId++};
		u64 const managedObjectHandle;

		ManagedAccessObject(u64 managedObjectHandle);
		virtual ~ManagedAccessObject();
	};

	void store_mao(ManagedAccessObject* mao);
	void destroy_mao(u64 id);
	ManagedAccessObject* get_mao_by_id(u64 id);
	void destroy_all_maos();
}