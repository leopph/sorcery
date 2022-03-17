#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "../src/components/Behavior.hpp"
#include "../src/components/Camera.hpp"
#include "../src/components/Component.hpp"
#include "../src/components/lighting/AmbientLight.hpp"
#include "../src/components/lighting/DirLight.hpp"
#include "../src/components/lighting/PointLight.hpp"
#include "../src/components/lighting/SpotLight.hpp"
#include "../src/components/rendering/Cube.hpp"
#include "../src/components/rendering/ImageSprite.hpp"
#include "../src/components/rendering/Model.hpp"
#include "../src/components/rendering/Model.hpp"
#include "../src/config/Settings.hpp"
#include "../src/data/Poelo.hpp"
#include "../src/entity/Entity.hpp"
#include "../src/events/Event.hpp"
#include "../src/events/handling/EventManager.hpp"
#include "../src/events/handling/EventReceiver.hpp"
#include "../src/events/handling/EventReceiverHandle.hpp"
#include "../src/input/Input.hpp"
#include "../src/math/LeopphMath.hpp"
#include "../src/math/Matrix.hpp"
#include "../src/math/Quaternion.hpp"
#include "../src/math/Vector.hpp"
#include "../src/misc/Color.hpp"
#include "../src/rendering/Skybox.hpp"
#include "../src/timing/Time.hpp"
#include "../src/util/Exit.hpp"
#include "../src/windowing/Window.hpp"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "../src/entry/Entry.hpp"
#endif
