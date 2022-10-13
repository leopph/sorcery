#include "CubeModel.hpp"

#include "RenderCore.hpp"
#include "Managed.hpp"

#include <mono/metadata/class.h>


namespace leopph
{
	CubeModel::CubeModel(MonoObject* const managedObject, Entity* const entity) :
		Component{ managedObject, entity }
	{
		RenderCore::get_last_instance()->register_cube_model(this);
	}


	CubeModel::~CubeModel()
	{
		RenderCore::get_last_instance()->unregister_cube_model(this);
	}
}