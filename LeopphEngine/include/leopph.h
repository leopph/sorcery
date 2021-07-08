#pragma once

/*------------------------------------------
Standard LeopphEngine API
See individual headers for more information.
------------------------------------------*/

#include "../src/components/component.h"
#include "../src/components/behavior.h"
#include "../src/components/camera.h"
#include "../src/components/lighting/dirlight.h"
#include "../src/components/lighting/pointlight.h"
#include "../src/components/models/model.h"
#include "../src/config/settings.h"
#include "../src/input/input.h"
#include "../src/math/leopphmath.h"
#include "../src/math/vector.h"
#include "../src/math/matrix.h"
#include "../src/math/quaternion.h"
#include "../src/misc/camerabackground.h"
#include "../src/misc/skybox.h"
#include "../src/hierarchy/object.h"
#include "../src/timing/leopphtime.h"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "../src/entry/entry.h"
#endif