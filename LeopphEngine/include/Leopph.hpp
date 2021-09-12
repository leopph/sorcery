#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "../src/components/Component.hpp"
#include "../src/components/Behavior.hpp"
#include "../src/components/Camera.hpp"
#include "../src/components/lighting/AmbientLight.hpp"
#include "../src/components/lighting/DirLight.hpp"
#include "../src/components/lighting/PointLight.hpp"
#include "../src/components/lighting/SpotLight.hpp"
#include "../src/components/Model.hpp"
#include "../src/config/Settings.hpp"
#include "../src/events/Event.hpp"
#include "../src/events/EventManager.hpp"
#include "../src/events/EventReceiver.hpp"
#include "../src/events/EventReceiverHandle.hpp"
#include "../src/entity/Entity.hpp"
#include "../src/input/Input.hpp"
#include "../src/math/LeopphMath.hpp"
#include "../src/math/Matrix.hpp"
#include "../src/math/Quaternion.hpp"
#include "../src/math/Vector.hpp"
#include "../src/misc/camerabackground.h"
#include "../src/misc/color.h"
#include "../src/misc/Skybox.hpp"
#include "../src/timing/leopphtime.h"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "../src/entry/entry.h"
#endif