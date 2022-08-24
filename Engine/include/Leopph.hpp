#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "AABB.hpp"
#include "Behavior.hpp"
#include "Cameras.hpp"
#include "Color.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "Exit.hpp"
#include "Extent.hpp"
#include "FrameCompleteEvent.hpp"
#include "Frustum.hpp"
#include "Image.hpp"
#include "ImageSprite.hpp"
#include "Input.hpp"
#include "Lights.hpp"
#include "Math.hpp"
#include "ModelImport.hpp"
#include "Settings.hpp"
#include "Skybox.hpp"
#include "StaticMeshComponent.hpp"
#include "StaticModelData.hpp"
#include "Timer.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "Window.hpp"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "Entry.hpp"
#endif
