#define LEOPPH_ENTRY

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../WindowController.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>



void leopph::init()
{
	get_main_window().set_title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->attach_component<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->attach_component<Exiter>();
	utilEnt->attach_component<WindowController>();

	auto* const player = new Entity{};
	player->set_position(Vector3{0, 0, -5});

	auto& camera = player->attach_component<PerspectiveCamera>();
	//camera.set_background(std::make_shared<Skybox>("skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"));
	camera.set_near_clip_plane(0.3f);
	camera.set_far_clip_plane(1000);

	player->attach_component<demo::FirstPersonCameraController>(2.0f, 0.1f, 5.0f, 0.2f);

	auto* const lampEntity = new Entity{};
	lampEntity->rescale(Vector3{0.01});
	attach_static_mesh_component_from_model_file(lampEntity, "models/lamp/scene.gltf");

	auto* const churchEntity = new Entity{};
	churchEntity->rotate(Vector3::forward(), 90);
	churchEntity->rotate(Vector3::right(), 90);
	//attach_static_mesh_component_from_model_file(churchEntity, "models/church/ChristchurchGreyfriarsRuinGarden03.obj");

	AmbientLight::Instance().Intensity(Vector3{1});

	//auto* const dirLightEntity = new Entity{};
	//dirLightEntity->rotate(Vector3::up(), 45);
	//dirLightEntity->rotate(Vector3::right(), 30, Space::Local);
	//dirLightEntity->attach_component<DirectionalLight>();

	/*auto& spotLight = player->attach_component<SpotLight>();
	spotLight.set_inner_angle(25);
	spotLight.set_outer_angle(35);
	spotLight.set_range(5);

	auto* const pointLightEntity = new Entity{};
	pointLightEntity->set_parent(lampEntity);
	pointLightEntity->translate(-0.7f, 3.7f, 0, Space::Local);
	auto& pointLight = pointLightEntity->attach_component<PointLight>();
	pointLight.set_range(7);*/
}
