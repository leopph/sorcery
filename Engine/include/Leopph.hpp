#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "../LeopphEngine/src/components/Behavior.hpp"
#include "../LeopphEngine/src/components/Component.hpp"
#include "../LeopphEngine/src/components/OrthographicCamera.hpp"
#include "../LeopphEngine/src/components/PerspectiveCamera.hpp"
#include "../LeopphEngine/src/components/lighting/AmbientLight.hpp"
#include "../LeopphEngine/src/components/lighting/DirLight.hpp"
#include "../LeopphEngine/src/components/lighting/PointLight.hpp"
#include "../LeopphEngine/src/components/lighting/SpotLight.hpp"
#include "../LeopphEngine/src/components/rendering/Cube.hpp"
#include "../LeopphEngine/src/components/rendering/ImageSprite.hpp"
#include "../LeopphEngine/src/components/rendering/Model.hpp"
#include "../LeopphEngine/src/components/rendering/Model.hpp"
#include "../LeopphEngine/src/config/Settings.hpp"
#include "../LeopphEngine/src/data/Poelo.hpp"
#include "../LeopphEngine/src/entity/Entity.hpp"
#include "../LeopphEngine/src/events/Event.hpp"
#include "../LeopphEngine/src/events/handling/EventManager.hpp"
#include "../LeopphEngine/src/events/handling/EventReceiver.hpp"
#include "../LeopphEngine/src/events/handling/EventReceiverHandle.hpp"
#include "../LeopphEngine/src/input/Input.hpp"
#include "../LeopphUtils/include/Math.hpp"
#include "../LeopphUtils/include/Matrix.hpp"
#include "../LeopphUtils/include/Quaternion.hpp"
#include "../LeopphUtils/include/Vector.hpp"
#include "../LeopphUtils/include/LeopphApi.hpp"
#include "../LeopphUtils/include/Color.hpp"
#include "../LeopphEngine/src/rendering/Skybox.hpp"
#include "../LeopphEngine/src/timing/Time.hpp"
#include "../LeopphEngine/src/util/Exit.hpp"
#include "../LeopphEngine/src/windowing/Window.hpp"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "../src/entry/Entry.hpp"
#endif