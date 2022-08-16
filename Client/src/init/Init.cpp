#define LEOPPH_ENTRY

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../WindowController.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>



void leopph::Init()
{
	get_window()->set_title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->attach_component<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->attach_component<Exiter>();
	utilEnt->attach_component<WindowController>();

	auto* const group = new Entity{};
	group->rotate(Vector3::up(), 90);

	auto* const player = new Entity{};
	player->set_parent(group);

	auto* const camera = &player->attach_component<leopph::PerspectiveCamera>();
	camera->set_background(std::make_shared<Skybox>("skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"));
	camera->set_near_clip_plane(0.1f);
	camera->set_far_clip_plane(100);

	player->attach_component<demo::FirstPersonCameraController>(2.0f, 0.1f, 5.0f, 0.2f);
	auto* const sLight = &player->attach_component<leopph::SpotLight>();
	sLight->set_color(Vector3{1});
	sLight->set_inner_angle(25);
	sLight->set_outer_angle(35);
	sLight->set_range(5);

	auto* const dirLightEntity = new Entity{};
	dirLightEntity->set_parent(group);
	dirLightEntity->rotate(Vector3::up(), 45);
	dirLightEntity->rotate(Vector3::right(), 30, Space::Local);

	auto* const dirLight = &dirLightEntity->attach_component<DirectionalLight>();
	dirLight->set_color(Vector3{.06f});
	dirLight->set_casting_shadow(false);

	auto* const lamp = new Entity{};
	lamp->set_parent(group);
	lamp->translate(0, -1.25, 0);
	lamp->rotate(Vector3::up(), -90);
	lamp->rescale(0.01f, 0.01f, 0.01f);

	//auto* const lampModel = &lamp->attach_component<StaticMeshComponent>("models/lamp/lamp.leopph3d");
	//lampModel->set_casting_shadow(true);

	auto* const pLightEntity = new Entity{};
	pLightEntity->set_parent(lamp);
	pLightEntity->translate(-0.7f, 3.7f, 0, Space::Local);

	auto* const pLight = &pLightEntity->attach_component<PointLight>();
	pLight->set_color(Vector3{1});
	pLight->set_range(7);
	pLight->set_casting_shadow(false);

	auto* const church = new Entity{};
	church->set_parent(group);
	church->translate(0, -3, 0, Space::World);
	church->rotate(Vector3::right(), 90);
	church->rotate(Vector3::up(), -90);

	//auto* const churchModel = &church->attach_component<StaticMeshComponent>("models/church/church.leopph3d");
	//churchModel->set_casting_shadow(true);
}
