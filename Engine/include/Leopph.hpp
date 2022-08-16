#pragma once

/*------------------------------------------
Standard LeopphEngine API headers.
See individual headers for more information.
------------------------------------------*/

#include "AABB.hpp"
#include "AmbientLight.hpp"
#include "AttenuatedLight.hpp"
#include "Behavior.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Cube.hpp"
#include "CursorState.hpp"
#include "DirectionalLight.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "EventManager.hpp"
#include "EventReceiver.hpp"
#include "EventReceiverBase.hpp"
#include "EventReceiverHandle.hpp"
#include "Exit.hpp"
#include "Extent.hpp"
#include "FrameCompleteEvent.hpp"
#include "Frustum.hpp"
#include "Image.hpp"
#include "ImageSprite.hpp"
#include "Input.hpp"
#include "KeyCode.hpp"
#include "KeyEvent.hpp"
#include "KeyState.hpp"
#include "Light.hpp"
#include "Math.hpp"
#include "Matrix.hpp"
#include "MouseEvent.hpp"
#include "OrthographicCamera.hpp"
#include "PerspectiveCamera.hpp"
#include "Poelo.hpp"
#include "PointLight.hpp"
#include "Quaternion.hpp"
#include "Settings.hpp"
#include "Skybox.hpp"
#include "Sphere.hpp"
#include "SpotLight.hpp"
#include "StaticMeshComponent.hpp"
#include "StaticModelData.hpp"
#include "SubMeshDescriptor.hpp"
#include "Time.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "Vector.hpp"
#include "Vertex.hpp"
#include "Window.hpp"
#include "WindowEvent.hpp"

/*-----------------------------------------------------
Define LEOPPH_ENTRY in exactly one of your source files
to create an entry point in your application.
-----------------------------------------------------*/

#ifdef LEOPPH_ENTRY
#include "Entry.hpp"
#endif
