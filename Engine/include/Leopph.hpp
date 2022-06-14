#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "AmbientLight.hpp"
#include "Behavior.hpp"
#include "Color.hpp"
#include "Component.hpp"
#include "Cube.hpp"
#include "DirLight.hpp"
#include "Entity.hpp"
#include "Exit.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "EventReceiver.hpp"
#include "EventReceiverHandle.hpp"
#include "ImageSprite.hpp"
#include "Input.hpp"
#include "Math.hpp"
#include "Matrix.hpp"
#include "Model.hpp"
#include "OrthographicCamera.hpp"
#include "PerspectiveCamera.hpp"
#include "Poelo.hpp"
#include "PointLight.hpp"
#include "Quaternion.hpp"
#include "Settings.hpp"
#include "Skybox.hpp"
#include "SpotLight.hpp"
#include "Time.hpp"
#include "Vector.hpp"
#include "Window.hpp"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "Entry.hpp"
#endif