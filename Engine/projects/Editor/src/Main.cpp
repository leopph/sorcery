#include "Serialization.hpp"
#include "Widgets.hpp"
#include "Material.hpp"
#include "Time.hpp"
#include "BinarySerializer.hpp"
#include "ModelImport.hpp"

#include <TransformComponent.hpp>
#include <BehaviorComponent.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Entity.hpp>
#include <OnGui.hpp>
#include <Scene.hpp>
#include <SceneManager.hpp>
#include <Systems.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>

#include <YamlInclude.hpp>
#include <nfd.h>

#include <DirectXMath.h>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "Mesh.hpp"


extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


class ResourceStorage {
	std::unordered_map<std::filesystem::path, std::shared_ptr<leopph::Resource>> mData;

public:
	auto operator[](std::filesystem::path const& path) -> auto&& {
		return mData[absolute(path)];
	}

	auto find(std::filesystem::path const& path) const {
		return mData.find(path);
	}

	auto begin() const noexcept {
		return std::begin(mData);
	}

	auto end() const noexcept {
		return std::end(mData);
	}

	auto cbegin() const noexcept {
		return std::cbegin(mData);
	}

	auto cend() const noexcept {
		return std::cend(mData);
	}
};


namespace {
std::string_view constexpr RESOURCE_FILE_EXT{ ".leopphres" };

leopph::Object* gSelected{ nullptr };
std::optional<std::filesystem::path> gProjDir;
std::filesystem::path gRelativeResDir;

auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
	return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}


auto CloseCurrentScene() -> void {
	for (static std::vector<leopph::Entity*> entities; auto const& entity : leopph::SceneManager::GetActiveScene()->GetEntities(entities)) {
		entity->GetScene().DestroyEntity(entity);
	}
}


YAML::Node gSerializedSceneBackup;
std::atomic<bool> gLoading{ false };

auto LoadAndBlockEditor(ImGuiIO& io, std::function<void()> const& fun) -> void {
	auto const oldFlags{ io.ConfigFlags };
	io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
	io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	gLoading = true;
	fun();
	io.ConfigFlags = oldFlags;
	gLoading = false;
}

auto GenerateResourceFileHeader(leopph::Resource const& resource, std::vector<leopph::u8>& out) -> void {
	leopph::BinarySerializer<leopph::i32>::Serialize(static_cast<leopph::i32>(resource.GetSerializationType()), out, std::endian::little);
	leopph::BinarySerializer<leopph::u64>::Serialize(*reinterpret_cast<leopph::u64 const*>(&resource.GetGuid()), out, std::endian::little);
	leopph::BinarySerializer<leopph::u64>::Serialize(*(reinterpret_cast<leopph::u64 const*>(&resource.GetGuid()) + 1), out, std::endian::little);
	leopph::BinarySerializer<leopph::u64>::Serialize(resource.GetName().size(), out, std::endian::little);
	for (auto const c : resource.GetName()) {
		out.emplace_back(c);
	}
}

auto SerializeResource(leopph::Resource const& resource, std::vector<leopph::u8>& out) -> void {
	GenerateResourceFileHeader(resource, out);
	resource.SerializeBinary(out);
}

auto GetHeaderFromResourceBytes(std::span<leopph::u8 const> const allBytes) -> std::span<leopph::u8 const> {
	auto constexpr fixedContentByteCount{ sizeof(leopph::i32) + sizeof(leopph::Guid) + sizeof(leopph::u64) };

	std::string_view constexpr errMsg{ "Failed to extract resource header bytes: the buffer is too small." };

	if (allBytes.size() < fixedContentByteCount) {
		throw std::runtime_error{ errMsg.data() };
	}

	auto const nameLength{ *reinterpret_cast<leopph::u64 const*>(allBytes.subspan<sizeof(leopph::i32) + sizeof(leopph::Guid), sizeof(leopph::u64)>().data()) };

	if (allBytes.size() < fixedContentByteCount + nameLength) {
		throw std::runtime_error{ errMsg.data() };
	}

	return allBytes.first(fixedContentByteCount + nameLength);
}

// Does not validate the passed header bytes in any way.
auto CreateResourceFromHeader(leopph::ObjectFactory const& factory, std::span<leopph::u8 const> const headerBytes) -> std::shared_ptr<leopph::Resource> {
	auto const obj{ factory.New(static_cast<leopph::Object::Type>(leopph::BinarySerializer<leopph::i32>::Deserialize(headerBytes.first<sizeof(leopph::i32)>(), std::endian::little))) };

	if (auto const res{ dynamic_cast<leopph::Resource*>(obj) }) {
		res->SetGuid(*reinterpret_cast<leopph::Guid const*>(headerBytes.subspan<sizeof(leopph::i32), sizeof(leopph::Guid)>().data()));

		if (auto const nameSize{ *reinterpret_cast<leopph::u64 const*>(headerBytes.subspan<sizeof(leopph::i32) + sizeof(leopph::Guid), sizeof(leopph::u64)>().data()) }; nameSize > 0) {
			res->SetName(std::string{ reinterpret_cast<char const*>(headerBytes.data()) + sizeof(leopph::i32) + sizeof(leopph::Guid) + sizeof(leopph::u64), nameSize });
		}

		return std::shared_ptr<leopph::Resource>{ res };
	}

	if (obj) {
		delete obj;
	}

	throw std::runtime_error{ "Failed create resource header bytes: the object is not a resource type." };
}

auto LoadResourcesFromProjectDir(leopph::ObjectFactory const& objectFactory, ResourceStorage& out) noexcept -> void {
	struct ResourceAndBodyBytes {
		std::shared_ptr<leopph::Resource> resource{};
		std::vector<leopph::u8> bodyBytes{};
	};

	std::unordered_map<std::filesystem::path, ResourceAndBodyBytes> loadedResources;

	for (auto const& childEntry : std::filesystem::recursive_directory_iterator{ *gProjDir }) {
		if (!childEntry.is_directory() && childEntry.path().extension() == RESOURCE_FILE_EXT) {
			try {
				std::ifstream in{ childEntry.path(), std::ios::binary };
				std::vector<leopph::u8> bytes;
				bytes.insert(std::begin(bytes), std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{});

				auto const headerBytes{ GetHeaderFromResourceBytes(bytes) };
				std::shared_ptr res{ CreateResourceFromHeader(objectFactory, headerBytes) };
				bytes.erase(std::begin(bytes), std::begin(bytes) + headerBytes.size());

				loadedResources[childEntry.path()] = ResourceAndBodyBytes{ std::move(res), std::move(bytes) };
			}
			catch (std::exception const& ex) {
				MessageBoxA(nullptr, ex.what(), "Error", MB_ICONERROR);
			}
		}
	}

	for (auto& [path, data] : loadedResources) {
		try {
			std::ignore = data.resource->DeserializeBinary(data.bodyBytes);
			out[path] = data.resource;
		}
		catch (std::exception const& ex) {
			MessageBoxA(nullptr, ex.what(), "Error", MB_ICONERROR);
		}
	}
}

auto ImportResource(std::filesystem::path const& srcPath) -> std::shared_ptr<leopph::Resource> {
	auto [positions, normals, uvs, indices]{ leopph::ImportMeshResourceData(srcPath) };
	auto mesh{ std::make_shared<leopph::Mesh>() };
	mesh->SetPositions(std::move(positions));
	mesh->SetNormals(std::move(normals));
	mesh->SetUVs(std::move(uvs));
	mesh->SetIndices(std::move(indices));
	mesh->ValidateAndUpdate();
	return mesh;
}

auto IndexFileNameIfNeeded(std::filesystem::path const& filePathAbsolute) -> std::filesystem::path {
	std::string const originalStem{ filePathAbsolute.stem().string() };
	std::filesystem::path const ext{ filePathAbsolute.extension() };
	std::filesystem::path const parentDir{ filePathAbsolute.parent_path() };

	std::string currentStem{ originalStem };
	std::size_t fileNameIndex{ 1 };

	while (true) {
		bool isUsed{ false };
		for (auto const& entry : std::filesystem::directory_iterator{ parentDir }) {
			if (entry.path().stem() == currentStem) {
				currentStem = originalStem;
				currentStem += " ";
				currentStem += std::to_string(fileNameIndex);
				++fileNameIndex;
				isUsed = true;
				break;
			}
		}

		if (!isUsed) {
			break;
		}
	}

	return (parentDir / currentStem).replace_extension(ext);
}
}


auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) -> int {
	try {
		leopph::ObjectFactory objectFactory;
		objectFactory.Register<leopph::Entity>();
		objectFactory.Register<leopph::TransformComponent>();
		objectFactory.Register<leopph::CameraComponent>();
		objectFactory.Register<leopph::BehaviorComponent>();
		objectFactory.Register<leopph::CubeModelComponent>();
		objectFactory.Register<leopph::DirectionalLightComponent>();
		objectFactory.Register<leopph::Material>();
		objectFactory.Register<leopph::Mesh>();

		leopph::gWindow.StartUp();
		leopph::gRenderer.StartUp();
		leopph::gManagedRuntime.StartUp();

		leopph::gWindow.SetBorderless(false);
		leopph::gWindow.SetWindowedClientAreaSize({ 1280, 720 });
		leopph::gWindow.SetIgnoreManagedRequests(true);

		leopph::gRenderer.SetGameResolution({ 960, 540 });
		leopph::gRenderer.SetSyncInterval(1);

		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		auto const iniFilePath{ std::filesystem::path{ leopph::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
		auto const iniFilePathStr{ leopph::WideToUtf8(iniFilePath.c_str()) };
		io.IniFilename = iniFilePathStr.c_str();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		leopph::SetImGuiContext(ImGui::GetCurrentContext());

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(leopph::gWindow.GetHandle());
		ImGui_ImplDX11_Init(leopph::gRenderer.GetDevice(), leopph::gRenderer.GetImmediateContext());

		leopph::gWindow.SetEventHook(EditorImGuiEventHook);

		bool runGame{ false };
		bool showDemoWindow{ false };

		ResourceStorage resources;

		leopph::init_time();

		while (!leopph::gWindow.IsQuitSignaled()) {
			leopph::gWindow.ProcessEvents();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			if (!gProjDir) {
				auto flags{ ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings };
				auto const viewport{ ImGui::GetMainViewport() };
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);

				if (ImGui::Begin("Open Project", nullptr, flags)) {
					auto const openProjectButtonLabel{ "Open Project" };
					auto const windowSize{ ImGui::GetWindowSize() };
					auto const textSize{ ImGui::CalcTextSize(openProjectButtonLabel) };

					ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
					ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

					if (ImGui::Button("Open Project")) {
						if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
							gProjDir = std::filesystem::path{ selectedPath };
							LoadResourcesFromProjectDir(objectFactory, resources);
						}
					}
				}
				ImGui::End();
			}
			else {
				if (runGame) {
					leopph::init_behaviors();
					leopph::tick_behaviors();
					leopph::tack_behaviors();

					if (leopph::GetKeyDown(leopph::Key::Escape)) {
						runGame = false;
						leopph::gWindow.SetEventHook(EditorImGuiEventHook);
						leopph::gWindow.SetCursorConfinement(false);
						leopph::gWindow.SetCursorHiding(false);
						leopph::gRenderer.SetSyncInterval(1);
						CloseCurrentScene();
						leopph::editor::DeserializeScene(objectFactory, gSerializedSceneBackup);
						gSelected = nullptr;
					}
				}
				else {
					if (leopph::GetKeyDown(leopph::Key::F5)) {
						runGame = true;
						leopph::gWindow.SetEventHook({});
						leopph::gRenderer.SetSyncInterval(0);
						gSerializedSceneBackup = leopph::editor::SerializeScene();
					}
				}

				ImGui::DockSpaceOverViewport();

				if (gLoading) {
					ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
					if (ImGui::Begin("LoadingIndicator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
						leopph::editor::DrawSpinner("##spinner", 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
					}
					ImGui::End();
				}

				if (showDemoWindow) {
					ImGui::ShowDemoWindow();
				}

				if (ImGui::BeginMainMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						/*if (ImGui::MenuItem("Open Project")) {
							if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
								auto const LoadAndAssignProject = [selectedPath] {
									gProject = leopph::editor::LoadProject(selectedPath);
									std::free(selectedPath);
								};

								std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), LoadAndAssignProject };
								loaderThread.detach();
							}
						}

						if (ImGui::MenuItem("Import Asset")) {
							ImportResource(io);
						}

						if (ImGui::MenuItem("Close Project")) {
							gProject = nullptr;
						}*/

						if (ImGui::MenuItem("Save Test Scene")) {
							if (!runGame) {
								std::ofstream out{ "scene.yaml" };
								YAML::Emitter emitter{ out };
								auto const serializedScene = leopph::editor::SerializeScene();
								emitter << serializedScene;
								gSerializedSceneBackup = serializedScene;
							}
						}

						if (ImGui::MenuItem("Load Test Scene")) {
							CloseCurrentScene();
							auto const serializedScene = YAML::LoadFile("scene.yaml");
							leopph::editor::DeserializeScene(objectFactory, serializedScene);
							gSerializedSceneBackup = serializedScene;
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Create")) {
						if (ImGui::MenuItem("Entity")) {
							auto const entity{ leopph::SceneManager::GetActiveScene()->CreateEntity() };
							entity->CreateManagedObject();

							auto transform = std::make_unique<leopph::TransformComponent>();
							transform->CreateManagedObject();

							entity->AddComponent(std::move(transform));
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Debug")) {
						if (showDemoWindow) {
							if (ImGui::MenuItem("Hide Demo Window")) {
								showDemoWindow = false;
							}
						}
						else {
							if (ImGui::MenuItem("Show Demo Window")) {
								showDemoWindow = true;
							}
						}

						ImGui::EndMenu();
					}

					ImGui::EndMainMenuBar();
				}

				static std::vector<leopph::Entity*> entities;
				leopph::SceneManager::GetActiveScene()->GetEntities(entities);


				if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
					auto constexpr baseFlags{ ImGuiTreeNodeFlags_OpenOnArrow };
					auto constexpr entityPayloadType{ "ENTITY" };

					if (ImGui::BeginDragDropTarget()) {
						if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
							static_cast<leopph::Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
							ImGui::EndDragDropTarget();
						}
					}

					for (std::size_t i = 0; i < entities.size(); i++) {
						std::function<void(leopph::Entity&)> displayEntityRecursive;
						displayEntityRecursive = [&displayEntityRecursive](leopph::Entity& entity) -> void {
							ImGuiTreeNodeFlags nodeFlags{ baseFlags };

							if (entity.GetTransform().GetChildren().empty()) {
								nodeFlags |= ImGuiTreeNodeFlags_Leaf;
							}
							else {
								nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
							}

							if (gSelected && gSelected->GetGuid() == entity.GetGuid()) {
								nodeFlags |= ImGuiTreeNodeFlags_Selected;
							}

							bool const nodeOpen{ ImGui::TreeNodeEx(entity.GetName().data(), nodeFlags) };

							if (ImGui::BeginDragDropSource()) {
								ImGui::SetDragDropPayload(entityPayloadType, &entity, sizeof entity);
								ImGui::Text(entity.GetName().data());
								ImGui::EndDragDropSource();
							}

							if (ImGui::BeginDragDropTarget()) {
								if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
									static_cast<leopph::Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
								}
								ImGui::EndDragDropTarget();
							}

							if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
								gSelected = &entity;
							}

							bool deleted{ false };

							if (ImGui::BeginPopupContextItem()) {
								if (ImGui::MenuItem("Delete")) {
									entity.GetScene().DestroyEntity(&entity);
									leopph::SceneManager::GetActiveScene()->GetEntities(entities);
									gSelected = nullptr;
									deleted = true;
									ImGui::CloseCurrentPopup();
								}
								ImGui::EndPopup();
							}

							ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);

							if (nodeOpen) {
								if (!deleted) {
									for (std::size_t childIndex{ 0 }; childIndex < entity.GetTransform().GetChildren().size(); childIndex++) {
										ImGui::PushID(static_cast<int>(childIndex));
										displayEntityRecursive(*entity.GetTransform().GetChildren()[childIndex]->GetEntity());
										ImGui::PopID();
									}
								}
								ImGui::TreePop();
							}
						};

						if (!entities[i]->GetTransform().GetParent()) {
							ImGui::PushID(static_cast<int>(i));
							displayEntityRecursive(*entities[i]);
							ImGui::PopID();
						}
					}
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

				if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
					if (gSelected) {
						gSelected->OnGui();
					}
				}
				ImGui::End();

				ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

				ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				if (runGame) {
					ImGui::SetNextWindowCollapsed(false);
					ImGui::SetNextWindowFocus();
				}

				if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
					ImGui::PopStyleVar();

					leopph::Extent2D<leopph::u32> constexpr resolutions[]{ { 960, 540 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } };
					constexpr char const* resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
					static int selectedRes = 0;

					if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7)) {
						if (selectedRes != 0) {
							leopph::gRenderer.SetGameResolution(resolutions[selectedRes - 1]);
						}
					}

					auto const gameRes = leopph::gRenderer.GetGameResolution();
					auto const contentRegionSize = ImGui::GetContentRegionAvail();
					leopph::Extent2D<leopph::u32> const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
					ImVec2 frameDisplaySize;

					if (selectedRes == 0) {
						if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height) {
							leopph::gRenderer.SetGameResolution(viewportRes);
						}

						frameDisplaySize = contentRegionSize;
					}
					else {
						leopph::f32 const scale = std::min(contentRegionSize.x / static_cast<leopph::f32>(gameRes.width), contentRegionSize.y / static_cast<leopph::f32>(gameRes.height));
						frameDisplaySize = ImVec2(static_cast<leopph::f32>(gameRes.width) * scale, static_cast<leopph::f32>(gameRes.height) * scale);
					}

					leopph::gRenderer.DrawGame();
					ImGui::Image(leopph::gRenderer.GetGameFrame(), frameDisplaySize);
				}
				else {
					ImGui::PopStyleVar();
				}
				ImGui::End();

				ImVec2 static constexpr sceneViewportMinSize{ 480, 270 };

				ImGui::SetNextWindowSize(sceneViewportMinSize, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSizeConstraints(sceneViewportMinSize, ImGui::GetMainViewport()->WorkSize);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse)) {
					ImGui::PopStyleVar();
					auto const [sceneWidth, sceneHeight]{ leopph::gRenderer.GetSceneResolution() };
					auto const contentRegionSize{ ImGui::GetContentRegionAvail() };

					if (leopph::Extent2D const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
						viewportRes.width != sceneWidth || viewportRes.height != sceneHeight) {
						leopph::gRenderer.SetSceneResolution(viewportRes);
					}

					leopph::EditorCamera static editorCam{
						.position = leopph::Vector3{},
						.orientation = leopph::Quaternion{},
						.nearClip = 0.03f,
						.farClip = 10000.f,
						.fovVertRad = leopph::ToRadians(60),
					};

					static bool isMovingSceneCamera{ false };
					isMovingSceneCamera = isMovingSceneCamera ?
						                      ImGui::IsMouseDown(ImGuiMouseButton_Right) :
						                      ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

					if (isMovingSceneCamera) {
						ImGui::SetWindowFocus();

						leopph::gWindow.SetCursorHiding(true);

						leopph::Vector3 posDelta{ 0, 0, 0 };
						if (GetKey(leopph::Key::W) || GetKey(leopph::Key::UpArrow)) {
							posDelta += leopph::Vector3::Forward();
						}
						if (GetKey(leopph::Key::A) || GetKey(leopph::Key::LeftArrow)) {
							posDelta += leopph::Vector3::Left();
						}
						if (GetKey(leopph::Key::D) || GetKey(leopph::Key::RightArrow)) {
							posDelta += leopph::Vector3::Right();
						}
						if (GetKey(leopph::Key::S) || GetKey(leopph::Key::DownArrow)) {
							posDelta += leopph::Vector3::Backward();
						}

						posDelta.Normalize();

						if (GetKey(leopph::Key::Shift)) {
							posDelta *= 2;
						}

						editorCam.position += editorCam.orientation.Rotate(posDelta) * leopph::get_frame_time() * 2;

						auto const [mouseX, mouseY]{ leopph::gWindow.GetMouseDelta() };
						auto constexpr sens{ 0.05f };

						editorCam.orientation = leopph::Quaternion{ leopph::Vector3::Up(), static_cast<leopph::f32>(mouseX) * sens } * editorCam.orientation;
						editorCam.orientation *= leopph::Quaternion{ leopph::Vector3::Right(), static_cast<leopph::f32>(mouseY) * sens };
					}
					else {
						leopph::gWindow.SetCursorConfinement(false);
						leopph::gWindow.SetCursorHiding(false);
					}

					leopph::gRenderer.DrawSceneView(editorCam);
					ImGui::Image(leopph::gRenderer.GetSceneFrame(), contentRegionSize);

					if (auto const selectedEntity{ dynamic_cast<leopph::Entity*>(gSelected) }; selectedEntity) {
						static auto op{ ImGuizmo::OPERATION::TRANSLATE };
						static bool showGrid{ true };

						if (!io.WantTextInput && !isMovingSceneCamera) {
							if (GetKeyDown(leopph::Key::T)) {
								op = ImGuizmo::TRANSLATE;
							}
							if (GetKeyDown(leopph::Key::R)) {
								op = ImGuizmo::ROTATE;
							}
							if (GetKeyDown(leopph::Key::S)) {
								op = ImGuizmo::SCALE;
							}
							if (GetKeyDown(leopph::Key::G)) {
								showGrid = !showGrid;
							}
							if (GetKeyDown(leopph::Key::F)) {
								editorCam.position = selectedEntity->GetTransform().GetWorldPosition() - leopph::Vector3::Forward() * 2;
								editorCam.orientation = selectedEntity->GetTransform().GetLocalRotation();
							}
						}

						leopph::Matrix4 modelMat{ selectedEntity->GetTransform().GetModelMatrix() };
						auto const viewMat{ leopph::Matrix4::LookAt(editorCam.position, editorCam.position + editorCam.orientation.Rotate(leopph::Vector3::Forward()), leopph::Vector3::Up()) };
						auto const projMat{ leopph::Matrix4::PerspectiveAsymZLH(editorCam.fovVertRad, ImGui::GetWindowWidth() / ImGui::GetWindowHeight(), editorCam.nearClip, editorCam.farClip) };

						ImGuizmo::AllowAxisFlip(false);
						ImGuizmo::SetDrawlist();
						ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

						if (showGrid) {
							ImGuizmo::DrawGrid(viewMat.GetData(), projMat.GetData(), leopph::Matrix4::Identity().GetData(), editorCam.farClip);
						}

						if (Manipulate(viewMat.GetData(), projMat.GetData(), op, ImGuizmo::MODE::LOCAL, modelMat.GetData())) {
							leopph::Vector3 pos, euler, scale;
							ImGuizmo::DecomposeMatrixToComponents(modelMat.GetData(), pos.GetData(), euler.GetData(), scale.GetData());
							selectedEntity->GetTransform().SetWorldPosition(pos);
							selectedEntity->GetTransform().SetWorldRotation(leopph::Quaternion::FromEulerAngles(euler));
							selectedEntity->GetTransform().SetWorldScale(scale);
						}
					}
				}
				else {
					ImGui::PopStyleVar();
				}
				ImGui::End();

				if (ImGui::Begin("Resources", nullptr, ImGuiWindowFlags_NoCollapse)) {
					if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("ResourcesContextMenu");
					}

					if (ImGui::BeginPopup("ResourcesContextMenu")) {
						static std::vector<leopph::u8> outBytes;

						if (ImGui::BeginMenu("Create##CreateResource")) {
							if (ImGui::MenuItem("Material##CreateMaterial")) {
								auto mat{ std::make_shared<leopph::Material>() };
								auto const outPath{ IndexFileNameIfNeeded(*gProjDir / gRelativeResDir / "New Material" += RESOURCE_FILE_EXT) };
								mat->SetName(outPath.stem().string());
								gSelected = mat.get();

								SerializeResource(*mat, outBytes);
								std::ofstream out{ outPath, std::ios::binary };
								std::ranges::copy(std::as_const(outBytes), std::ostream_iterator<leopph::u8>{ out });
								outBytes.clear();

								resources[outPath] = std::move(mat);
							}
							ImGui::EndMenu();
						}

						if (ImGui::MenuItem("Import Resource")) {
							if (nfdchar_t* selectedPath{ nullptr }; NFD_OpenDialog(nullptr, nullptr, &selectedPath) == NFD_OKAY) {
								auto const importAsset = [selectedPath, &resources] {
									std::filesystem::path srcPath{ selectedPath };
									srcPath = absolute(srcPath);
									std::free(selectedPath);

									if (auto const res{ ImportResource(srcPath) }; res) {
										auto const outPath{ IndexFileNameIfNeeded(*gProjDir / gRelativeResDir / srcPath.stem() += RESOURCE_FILE_EXT) };
										res->SetName(outPath.stem().string());
										gSelected = res.get();

										SerializeResource(*res, outBytes);
										std::ofstream out{ outPath, std::ios::binary };
										std::ranges::copy(std::as_const(outBytes), std::ostream_iterator<leopph::u8>{ out });
										outBytes.clear();

										resources[std::move(outPath)] = std::move(res);
									}
								};

								std::thread loaderThread{ LoadAndBlockEditor, std::ref(io), importAsset };
								loaderThread.detach();
							}
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					for (auto const& entry : std::filesystem::directory_iterator{ *gProjDir / gRelativeResDir }) {
						auto const& entryPath{ entry.path() };
						if (is_directory(entryPath)) {
							ImGui::Selectable(std::format("{}##AssetFolderPath", entryPath.filename().string().c_str()).c_str());
							if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
								gRelativeResDir /= entryPath.filename();
							}
						}
						else if (auto const it{ resources.find(entryPath) }; it != std::end(resources)) {
							auto const& asset{ it->second };

							if (ImGui::Selectable(std::format("{}##{}", asset->GetName().data(), entryPath.string().c_str()).c_str(), gSelected == asset.get())) {
								gSelected = asset.get();
							}
						}
					}
				}
				ImGui::End();
			}

			ImGui::Render();

			leopph::gRenderer.BindAndClearSwapChain();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			leopph::gRenderer.Present();

			leopph::measure_time();
		}

		CloseCurrentScene();

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
	catch (std::exception const& ex) {
		leopph::DisplayError(ex.what());
	}

	leopph::gManagedRuntime.ShutDown();
	leopph::gRenderer.ShutDown();
	leopph::gWindow.ShutDown();
	return 0;
}
