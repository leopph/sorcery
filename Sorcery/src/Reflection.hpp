#pragma once

#define RTTR_DLL // This is necessary because rttr defines RTTR_API as __declspec(dllimport) only if RTTR_DLL is defined, which it is when building RTTR, but of course not when consuming it
#include <rttr/registration>
#include <rttr/registration_friend>

#define REFLECT_REGISTER_SCENE_OBJECT_CTOR constructor<>()(::rttr::policy::ctor::as_raw_ptr)
#define REFLECT_REGISTER_COMPONENT_CTOR REFLECT_REGISTER_SCENE_OBJECT_CTOR
#define REFLECT_REGISTER_ENTITY_CTOR REFLECT_REGISTER_SCENE_OBJECT_CTOR
