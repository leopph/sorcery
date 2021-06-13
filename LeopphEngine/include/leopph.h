#pragma once

/* LEOPPHENGINE API */
#include "../src/api/leopphapi.h"
#include "../src/components/component.h"
#include "../src/components/behavior.h"
#include "../src/components/camera.h"
#include "../src/components/lighting/dirlight.h"
#include "../src/components/lighting/pointlight.h"
#include "../src/input/input.h"
#include "../src/math/leopphmath.h"
#include "../src/math/vector.h"
#include "../src/math/matrix.h"
#include "../src/math/quaternion.h"
#include "../src/rendering/model.h"
#include "../src/hierarchy/object.h"
#include "../src/timing/timekeeping.h"

/* INCLUDE ENTRY POINT IF CLIENT REQUIRES */
#ifdef LEOPPH_ENTRY
#include "../src/entry/entry.h"
#endif