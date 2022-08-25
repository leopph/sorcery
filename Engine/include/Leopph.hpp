#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "AABB.hpp"
#include "BehaviorNode.hpp"
#include "CameraNodes.hpp"
#include "Color.hpp"
#include "Context.hpp"
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
#include "Node.hpp"
#include "Settings.hpp"
#include "Skybox.hpp"
#include "StaticMeshNode.hpp"
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
