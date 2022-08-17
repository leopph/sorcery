#define LEOPPH_ENTRY

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../WindowController.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>



void leopph::init()
{
	AmbientLight::Instance().Intensity(Vector3{1});

	get_window()->set_title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->attach_component<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->attach_component<Exiter>();
	utilEnt->attach_component<WindowController>();

	auto* const player = new Entity{};
	player->set_position(Vector3{0, 0, -5});

	auto* const camera = &player->attach_component<PerspectiveCamera>();
	//camera->set_background(std::make_shared<Skybox>("skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"));
	camera->set_near_clip_plane(0.3f);
	camera->set_far_clip_plane(1000);

	player->attach_component<demo::FirstPersonCameraController>(2.0f, 0.1f, 5.0f, 0.2f);

	/*auto* const sLight = &player->attach_component<leopph::SpotLight>();
	sLight->set_color(Vector3{1});
	sLight->set_inner_angle(25);
	sLight->set_outer_angle(35);
	sLight->set_range(5);*/

	/*auto* const dirLightEntity = new Entity{};
	dirLightEntity->rotate(Vector3::up(), 45);
	dirLightEntity->rotate(Vector3::right(), 30, Space::Local);

	auto* const dirLight = &dirLightEntity->attach_component<DirectionalLight>();
	dirLight->set_intensity(50);
	dirLight->set_color(Vector3{.06f});
	dirLight->set_casting_shadow(false);*/

	auto* const lamp = new Entity{};
	//lamp->translate(0, -1.25, 0);
	//lamp->rotate(Vector3::up(), -90);
	lamp->set_scale(Vector3{0.01f});

	for (auto const lampRenderData = generate_render_structures(import_static_model("models/lamp/scene.gltf"));
	     auto const& [mesh, material] : lampRenderData)
	{
		lamp->attach_component<StaticMeshComponent>(mesh, material);
	}

	/*auto* const pLightEntity = new Entity{};
	pLightEntity->set_parent(lamp);
	pLightEntity->translate(-0.7f, 3.7f, 0, Space::Local);

	auto* const pLight = &pLightEntity->attach_component<PointLight>();
	pLight->set_color(Vector3{1});
	pLight->set_range(7);
	pLight->set_casting_shadow(false);

	auto* const church = new Entity{};
	church->translate(0, -3, 0, Space::World);
	church->rotate(Vector3::right(), 90);
	church->rotate(Vector3::up(), -90);*/

	//auto* const churchModel = &church->attach_component<StaticMeshComponent>("models/church/church.leopph3d");
	//churchModel->set_casting_shadow(true);
}
