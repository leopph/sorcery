#define LEOPPH_ENTRY

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../WindowController.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>



void leopph::init()
{
	get_main_window().set_title("LeopphEngine Demo");
	get_main_window().set_cursor_state(CursorState::Disabled);
	
	new FrameRateAnalyzer{0.5f, 60u};
	new WindowController{};
	new Exiter{};

	auto* const camera = new PerspectiveCameraNode{};
	//camera->set_background(std::make_shared<Skybox>("skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"));
	camera->set_near_clip_plane(0.3f);
	camera->set_far_clip_plane(1000);
	camera->set_position(Vector3{0, 0, -5});

	new demo::FirstPersonCameraController{camera, 2.0f, 0.1f, 5.0f, 0.2f};
	
	auto* const lamp = std::get<StaticMeshNode*>(create_static_mesh_node_from_model_file("models/lamp/scene.gltf"));
	lamp->rescale(Vector3{0.01});

	/*auto* const church = std::get<StaticMeshNode*>(create_static_mesh_node_from_model_file("models/church/ChristchurchGreyfriarsRuinGarden03.obj"));
	church->rotate(Vector3::forward(), 90);
	church->rotate(Vector3::right(), 90);*/

	AmbientLight::get_instance().set_intensity(Vector3{1});

	//auto* const dirLightEntity = new Node{};
	//dirLightEntity->rotate(Vector3::up(), 45);
	//dirLightEntity->rotate(Vector3::right(), 30, Space::Local);
	//dirLightEntity->attach_component<DirectionalLight>();

	/*auto& spotLight = player->attach_component<SpotLight>();
	spotLight.set_inner_angle(25);
	spotLight.set_outer_angle(35);
	spotLight.set_range(5);

	auto* const pointLightEntity = new Node{};
	pointLightEntity->set_parent(lampEntity);
	pointLightEntity->translate(-0.7f, 3.7f, 0, Space::Local);
	auto& pointLight = pointLightEntity->attach_component<PointLight>();
	pointLight.set_range(7);*/
}
