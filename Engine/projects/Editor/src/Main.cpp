#include <Components.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <Renderer.hpp>
#include <Time.hpp>
#include <Entity.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads.h>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include <format>
#include <optional>
#include <fstream>
#include <limits>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <string_view>
#include <cstring>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <variant>

using leopph::Vector3;
using leopph::Quaternion;
using leopph::f32;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	bool EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) {
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	}


	void CloseCurrentScene() {
		static std::vector<leopph::Entity*> entities;
		for (auto const& entity : leopph::GetEntities(entities)) {
			leopph::DeleteEntity(entity);
		}
	}


	YAML::Node gSerializedSceneBackup;


	YAML::Node SerializeScene() {
		YAML::Node scene;
		static std::vector<leopph::Entity*> entities;
		for (auto const& entity : leopph::GetEntities(entities)) {
			YAML::Node entityNode;
			entityNode["name"] = entity->name;

			static std::vector<leopph::Component*> components;

			for (auto const& component : entity->GetComponents(components)) {
				YAML::Node componentNode;
				auto const componentClass = mono_object_get_class(component->GetManagedObject());
				componentNode["classNameSpace"] = mono_class_get_namespace(componentClass);
				componentNode["className"] = mono_class_get_name(componentClass);

				std::function<void(MonoObject*, YAML::Node&)> serializeObject;

				serializeObject = [&serializeObject](MonoObject* const obj, YAML::Node& node) -> void {
					auto const objClass = mono_object_get_class(obj);

					void* iter{ nullptr };

					while (auto const field = mono_class_get_fields(objClass, &iter)) {
						if (leopph::ShouldSerialize(mono_field_get_object(leopph::GetManagedDomain(), objClass, field))) {
							auto const fieldType = mono_field_get_type(field);
							auto const fieldRefType = mono_type_get_object(leopph::GetManagedDomain(), fieldType);
							auto const fieldClass = mono_type_get_class(fieldType);
							auto const fieldName = mono_field_get_name(field);
							auto const fieldValueBoxed = mono_field_get_value_object(leopph::GetManagedDomain(), field, obj);

							if (leopph::IsTypePrimitive(fieldRefType)) {
								node[fieldName] = mono_string_to_utf8(mono_object_to_string(fieldValueBoxed, nullptr));
							}
							else if (mono_class_is_enum(fieldClass)) {
								node[fieldName] = mono_string_to_utf8(mono_object_to_string(leopph::EnumToUnderlyingType(fieldRefType, fieldValueBoxed), nullptr));
							}
							else if (mono_class_is_valuetype(fieldClass)) {
								auto dataNode = node[fieldName];
								serializeObject(fieldValueBoxed, dataNode);
							}
							else {
								std::cerr << "Serialization of reference type fields is not yet supported." << std::endl;
							}
						}
					}

					iter = nullptr;

					while (auto const prop = mono_class_get_properties(objClass, &iter)) {
						if (leopph::ShouldSerialize(mono_property_get_object(leopph::GetManagedDomain(), objClass, prop))) {
							auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
							auto const propRefType = mono_type_get_object(leopph::GetManagedDomain(), propType);
							auto const propClass = mono_type_get_class(propType);
							auto const propName = mono_property_get_name(prop);
							auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
							auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);

							if (leopph::IsTypePrimitive(propRefType)) {
								node[propName] = mono_string_to_utf8(mono_object_to_string(propValueBoxed, nullptr));
							}
							else if (mono_class_is_enum(propClass)) {
								node[propName] = mono_string_to_utf8(mono_object_to_string(leopph::EnumToUnderlyingType(propRefType, propValueBoxed), nullptr));
							}
							else if (mono_class_is_valuetype(propClass)) {
								auto dataNode = node[propName];
								serializeObject(propValueBoxed, dataNode);
							}
							else {
								std::cerr << "Serialization of reference type properties is not yet supported." << std::endl;
							}
						}
					}
				};

				auto dataNode = componentNode["data"];
				serializeObject(component->GetManagedObject(), dataNode);
				entityNode["components"].push_back(componentNode);
			}

			scene.push_back(entityNode);
		}

		return scene;
	}


	void DeserializeScene(YAML::Node const& scene) {
		for (auto const& entityNode : scene) {
			auto const entity = leopph::Entity::Create();
			entity->name = entityNode["name"].as<std::string>();
			entity->CreateManagedObject("leopph", "Entity");

			for (auto const& componentNode : entityNode["components"]) {
				auto const classNs = componentNode["classNameSpace"].as<std::string>();
				auto const className = componentNode["className"].as<std::string>();
				auto const componentClass = mono_class_from_name(leopph::GetManagedImage(), classNs.c_str(), className.c_str());
				auto const component = entity->CreateComponent(componentClass);
				auto const managedComponent = component->GetManagedObject();

				std::function<void(MonoObject*, YAML::Node const&)> parseAndSetMembers;
				std::function<MonoObject* (YAML::Node const&, MonoObject*, std::variant<MonoProperty*, MonoClassField*>)> setMember;

				parseAndSetMembers = [&parseAndSetMembers](MonoObject* const obj, YAML::Node const& dataNode) -> void {
					auto const objClass = mono_object_get_class(obj);

					for (auto it = dataNode.begin(); it != dataNode.end(); ++it) {
						auto const memberName = it->first.as<std::string>();

						if (auto const prop = mono_class_get_property_from_name(objClass, memberName.data())) {
							auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
							auto const propRefType = mono_type_get_object(leopph::GetManagedDomain(), propType);
							auto const propClass = mono_type_get_class(propType);
							auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
							auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);
							auto const refProp = mono_property_get_object(leopph::GetManagedDomain(), objClass, prop);

							if (leopph::IsTypePrimitive(propRefType)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseValue(refProp, memberValueStr.c_str());
								auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
							}
							else if (mono_class_is_enum(propClass)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseEnumValue(propRefType, memberValueStr.c_str());
								auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
							}
							else if (mono_class_is_valuetype(propClass)) {
								parseAndSetMembers(propValueBoxed, it->second);
								auto propValueUnboxed = mono_object_unbox(propValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &propValueUnboxed, nullptr);
							}
							else {
								std::cerr << "Deserialization of reference type properties is not yet supported." << std::endl;
							}
						}
						else if (auto const field = mono_class_get_field_from_name(objClass, memberName.data())) {
							auto const fieldType = mono_field_get_type(field);
							auto const fieldRefType = mono_type_get_object(leopph::GetManagedDomain(), fieldType);
							auto const fieldClass = mono_type_get_class(fieldType);
							auto const refField = mono_field_get_object(leopph::GetManagedDomain(), objClass, field);

							if (leopph::IsTypePrimitive(fieldRefType)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseValue(refField, memberValueStr.c_str());
								mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
							}
							else if (mono_class_is_enum(fieldClass)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseEnumValue(fieldRefType, memberValueStr.c_str());
								mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
							}
							else if (mono_class_is_valuetype(fieldClass)) {
								auto const fieldValueBoxed = mono_field_get_value_object(leopph::GetManagedDomain(), field, obj);
								parseAndSetMembers(fieldValueBoxed, it->second);
								mono_field_set_value(obj, field, mono_object_unbox(fieldValueBoxed));
							}
							else {
								std::cerr << "Deserialization of reference type fields is not yet supported." << std::endl;
							}
						}
						else {
							std::cerr << std::format("Member \"{}\" in file has no corresponding member in class.", memberName) << std::endl;
						}
					}
				};

				parseAndSetMembers(managedComponent, componentNode["data"]);
			}
		}
	}
}


int WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE, [[maybe_unused]] _In_ wchar_t*, [[maybe_unused]] _In_ int) {
	if (!leopph::platform::init_platform_support()) {
		return 1;
	}

	leopph::platform::set_window_borderless(false);
	leopph::platform::set_window_windowed_client_area_size({ 1280, 720 });
	leopph::platform::SetIgnoreManagedRequests(true);

	if (!leopph::rendering::InitRenderer()) {
		return 2;
	}

	leopph::rendering::SetGameResolution({ 960, 540 });
	leopph::rendering::SetSyncInterval(1);

	if (!leopph::initialize_managed_runtime()) {
		return 3;
	}

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	auto const iniFilePath{ std::filesystem::path{ leopph::platform::GetExecutablePath() }.remove_filename() /= "editorconfig.ini" };
	auto const iniFilePathStr{ leopph::platform::WideToUtf8(iniFilePath.c_str()) };
	io.IniFilename = iniFilePathStr.c_str();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(leopph::platform::get_hwnd());
	ImGui_ImplDX11_Init(leopph::rendering::GetDevice(), leopph::rendering::GetImmediateContext());

	leopph::platform::SetEventHook(EditorImGuiEventHook);

	bool runGame{ false };
	bool showDemoWindow{ false };

	leopph::init_time();

	while (!leopph::platform::should_window_close()) {
		if (!leopph::platform::process_platform_events()) {
			return 4;
		}

		if (runGame) {
			leopph::init_behaviors();
			leopph::tick_behaviors();
			leopph::tack_behaviors();

			if (leopph::platform::GetKeyDown(leopph::platform::Key::Escape)) {
				runGame = false;
				leopph::platform::SetEventHook(EditorImGuiEventHook);
				leopph::platform::confine_cursor(false);
				leopph::platform::hide_cursor(false);
				leopph::rendering::SetSyncInterval(1);
				CloseCurrentScene();
				DeserializeScene(gSerializedSceneBackup);
			}
		}
		else {
			if (leopph::platform::GetKeyDown(leopph::platform::Key::F5)) {
				runGame = true;
				leopph::platform::SetEventHook({});
				leopph::rendering::SetSyncInterval(0);
				gSerializedSceneBackup = SerializeScene();
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport();

		if (showDemoWindow) {
			ImGui::ShowDemoWindow();
		}

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
					MessageBoxW(leopph::platform::get_hwnd(), L"Placeholder", L"Placeholder", 0);
				}

				if (ImGui::MenuItem("Save")) {
					if (!runGame) {
						std::ofstream out{ "scene.yaml" };
						YAML::Emitter emitter{ out };
						auto const serializedScene = SerializeScene();
						emitter << serializedScene;
						gSerializedSceneBackup = serializedScene;
					}
				}

				if (ImGui::MenuItem("Load")) {
					CloseCurrentScene();
					auto const serializedScene = YAML::LoadFile("scene.yaml");
					DeserializeScene(serializedScene);
					gSerializedSceneBackup = serializedScene;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create")) {
				if (ImGui::MenuItem("Entity")) {
					leopph::Entity::Create()->CreateComponent<leopph::Transform>();
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

		static std::optional<std::size_t> selectedEntityIndex;
		static std::vector<leopph::Entity*> entities;
		leopph::GetEntities(entities);

		if (selectedEntityIndex && *selectedEntityIndex >= entities.size()) {
			selectedEntityIndex.reset();
		}

		if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
			for (std::size_t i = 0; i < entities.size(); i++) {
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::Selectable(entities[i]->name.data(), selectedEntityIndex && *selectedEntityIndex == i)) {
					selectedEntityIndex = i;
				}

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::Button("Delete")) {
						leopph::DeleteEntity(entities[i]);
					}
					ImGui::EndPopup();
				}

				ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);
				ImGui::PopID();
			}
		}
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Entity Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
			if (selectedEntityIndex) {
				auto const& entity = entities[*selectedEntityIndex];

				static std::string entityName;
				entityName = entity->name;

				if (ImGui::BeginTable("Property Widgets", 2)) {
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::PushItemWidth(FLT_MIN);
					ImGui::Text("Name");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (ImGui::InputText("##EntityName", &entityName)) {
						entity->name = entityName;
					}

					ImGui::EndTable();
				}

				static std::vector<leopph::Component*> components;

				for (auto const& component : entity->GetComponents(components)) {
					auto const obj = component->GetManagedObject();
					auto const klass = mono_object_get_class(obj);

					auto const componentNodeId = mono_class_get_name(klass);
					if (ImGui::TreeNodeEx(componentNodeId, ImGuiTreeNodeFlags_DefaultOpen)) {
						ImGui::Separator();
						if (ImGui::BeginTable(std::format("TableForMember{}", mono_class_get_name(klass)).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
							auto constexpr drawComponentMemberWidget = [](std::string_view const memberName, MonoType* const memberType, std::function<void* ()> const& getFunc, std::function<void(void**)> const& setFunc) {
								std::string_view const memberTypeName = mono_type_get_name(memberType);
								auto const memberClass = mono_type_get_class(memberType);

								ImGui::TableNextRow();
								if (ImGui::TableGetRowIndex() == 0) {
									ImGui::TableSetColumnIndex(0);
									ImGui::PushItemWidth(FLT_MIN);
									ImGui::TableSetColumnIndex(1);
									ImGui::PushItemWidth(-FLT_MIN);
								}

								ImGui::TableSetColumnIndex(0);
								ImGui::Text(memberName.data());
								ImGui::TableSetColumnIndex(1);

								auto const widgetLabel = std::format("##WidgetForMember{}", memberName);

								if (memberClass && mono_class_is_enum(memberClass)) {
									auto const enumValues = leopph::GetEnumValues(mono_type_get_object(leopph::GetManagedDomain(), memberType));
									auto const numEnumValues = mono_array_length(enumValues);
									int valueAlign;
									auto const valueSize = mono_type_size(mono_class_enum_basetype(memberClass), &valueAlign);

									auto const pCurrentValueUnboxed = getFunc();
									auto const currentValueBoxed = mono_value_box(leopph::GetManagedDomain(), memberClass, pCurrentValueUnboxed);
									auto const currentValueManagedStr = mono_object_to_string(currentValueBoxed, nullptr);
									auto const currentValueStr = mono_string_to_utf8(currentValueManagedStr);

									if (ImGui::BeginCombo(widgetLabel.c_str(), currentValueStr)) {
										for (std::size_t i{ 0 }; i < numEnumValues; i++) {
											auto pValue = mono_array_addr_with_size(enumValues, valueSize, i);
											auto const valueBoxed = mono_value_box(leopph::GetManagedDomain(), memberClass, reinterpret_cast<void*>(pValue));

											bool selected{ true };
											for (std::size_t j{ 0 }; j < valueSize; j++) {
												if (*reinterpret_cast<char*>(pCurrentValueUnboxed) != *pValue) {
													selected = false;
													break;
												}
											}

											if (ImGui::Selectable(mono_string_to_utf8(mono_object_to_string(valueBoxed, nullptr)), selected)) {
												setFunc(reinterpret_cast<void**>(&pValue));
											}
										}

										ImGui::EndCombo();
									}
								}
								else if (memberTypeName == "leopph.Vector3") {
									float data[3];
									std::memcpy(data, getFunc(), sizeof(data));
									if (ImGui::DragFloat3(widgetLabel.c_str(), data, 0.1f)) {
										auto pData = &data[0];
										setFunc(reinterpret_cast<void**>(&pData));
									}
								}
								else if (memberTypeName == "leopph.Quaternion") {
									auto euler = reinterpret_cast<leopph::Quaternion*>(getFunc())->ToEulerAngles();
									if (ImGui::DragFloat3(widgetLabel.c_str(), euler.get_data())) {
										auto quaternion = leopph::Quaternion::FromEulerAngles(euler[0], euler[1], euler[2]);
										auto pQuaternion = &quaternion;
										setFunc(reinterpret_cast<void**>(&pQuaternion));
									}
								}
								else if (memberTypeName == "System.Single") {
									float data;
									std::memcpy(&data, getFunc(), sizeof(data));
									if (ImGui::DragFloat(widgetLabel.c_str(), &data)) {
										auto pData = &data;
										setFunc(reinterpret_cast<void**>(&pData));
									}
								}
							};

							void* iter{ nullptr };
							while (auto const field = mono_class_get_fields(klass, &iter)) {
								auto const refField = mono_field_get_object(leopph::GetManagedDomain(), klass, field);

								if (leopph::ShouldSerialize(refField)) {
									drawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj] {
										return mono_object_unbox(mono_field_get_value_object(leopph::GetManagedDomain(), field, obj));
									}, [field, obj](void** data) {
										mono_field_set_value(obj, field, *data);
									});
								}
							}

							iter = nullptr;

							while (auto const prop = mono_class_get_properties(klass, &iter)) {
								auto const refProp = mono_property_get_object(leopph::GetManagedDomain(), klass, prop);

								if (leopph::ShouldSerialize(refProp)) {
									drawComponentMemberWidget(mono_property_get_name(prop), mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop))), [prop, obj] {
										return mono_object_unbox(mono_property_get_value(prop, obj, nullptr, nullptr));
									}, [prop, obj](void** data) {
										mono_property_set_value(prop, reinterpret_cast<void*>(obj), data, nullptr);
									});
								}
							}

							ImGui::EndTable();
						}

						ImGui::TreePop();
					}

					if (ImGui::BeginPopupContextItem(componentNodeId)) {
						if (ImGui::Button("Delete")) {
							entities[*selectedEntityIndex]->DeleteComponent(component);
						}
						ImGui::EndPopup();
					}
					ImGui::OpenPopupOnItemClick(componentNodeId, ImGuiPopupFlags_MouseButtonRight);
				}

				auto constexpr addNewComponentLabel = "Add New Component";
				ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(addNewComponentLabel).x) * 0.5f);
				ImGui::Button(addNewComponentLabel);

				if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
					for (auto const& componentClass : leopph::GetComponentClasses()) {
						auto const componentName = mono_class_get_name(componentClass);
						ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(componentName).x) * 0.5f);
						if (ImGui::Button(componentName)) {
							entity->CreateComponent(componentClass);
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();

		ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

		ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
			ImGui::PopStyleVar();

			leopph::Extent2D<leopph::u32> const resolutions[]{ {960, 540}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160} };
			char const* const resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
			static int selectedRes = 0;

			if (ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7)) {
				if (selectedRes != 0) {
					leopph::rendering::SetGameResolution(resolutions[selectedRes - 1]);
				}
			}

			auto const gameRes = leopph::rendering::GetGameResolution();
			auto const contentRegionSize = ImGui::GetContentRegionAvail();
			leopph::Extent2D<leopph::u32> const viewportRes{ static_cast<leopph::u32>(contentRegionSize.x), static_cast<leopph::u32>(contentRegionSize.y) };
			ImVec2 frameDisplaySize;

			if (selectedRes == 0) {
				if (viewportRes.width != gameRes.width || viewportRes.height != gameRes.height) {
					leopph::rendering::SetGameResolution(viewportRes);
				}

				frameDisplaySize = contentRegionSize;
			}
			else {
				leopph::f32 const scale = std::min(contentRegionSize.x / static_cast<leopph::f32>(gameRes.width), contentRegionSize.y / static_cast<leopph::f32>(gameRes.height));
				frameDisplaySize = ImVec2(gameRes.width * scale, gameRes.height * scale);
			}

			if (!leopph::rendering::DrawGame()) {
				return 5;
			}
			ImGui::Image(reinterpret_cast<void*>(leopph::rendering::GetGameFrame()), frameDisplaySize);
		}
		else {
			ImGui::PopStyleVar();
		}
		ImGui::End();

		ImGui::Render();

		leopph::rendering::BindAndClearSwapChain();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		leopph::rendering::Present();

		leopph::measure_time();
	}

	CloseCurrentScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	leopph::cleanup_managed_runtime();
	leopph::rendering::CleanupRenderer();
	leopph::platform::cleanup_platform_support();
	return 0;
}