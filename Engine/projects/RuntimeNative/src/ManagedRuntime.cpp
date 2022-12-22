#include "ManagedRuntime.hpp"

#include "Entity.hpp"
#include "Time.hpp"
#include "Platform.hpp"
#include "ManagedAccessObject.hpp"
#include "Components.hpp"
#include "Systems.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <cassert>
#include <filesystem>
#include <format>
#include <string>
#include <vector>


namespace leopph {
	auto ManagedRuntime::StartUp() -> void {
		HKEY monoRemKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Mono)", 0, KEY_READ, &monoRemKey) != ERROR_SUCCESS) {
			throw std::runtime_error{ "Failed to locate Mono installation. Couldn't find Mono registry key." };
		}

		auto const fwAssemblyDirKey = L"FrameworkAssemblyDirectory";
		DWORD bufSz{};
		if (RegGetValueW(monoRemKey, nullptr, fwAssemblyDirKey, RRF_RT_REG_SZ, nullptr, nullptr, &bufSz) != ERROR_SUCCESS) {
			throw std::runtime_error{ "Failed to locate Mono installation. Couldn't find Mono assembly directory key." };
		}

		std::wstring buf(static_cast<std::size_t>(bufSz), L'\0');
		if (RegGetValueW(monoRemKey, nullptr, fwAssemblyDirKey, RRF_RT_REG_SZ, nullptr, buf.data(), &bufSz) != ERROR_SUCCESS) {
			throw std::runtime_error{ "Failed to locate Mono installation. Couldn't read Mono assembly directory key." };
		}

		auto const fwAssemblyDir{ WideToUtf8(buf) };

		auto const confimDirKey = L"MonoConfigDir";
		if (RegGetValueW(monoRemKey, nullptr, confimDirKey, RRF_RT_REG_SZ, nullptr, nullptr, &bufSz) != ERROR_SUCCESS) {
			throw std::runtime_error{ "Failed to locate Mono installation. Couldn't find Mono config directory key." };
		}

		buf.resize(bufSz);

		if (RegGetValueW(monoRemKey, nullptr, confimDirKey, RRF_RT_REG_SZ, nullptr, buf.data(), &bufSz) != ERROR_SUCCESS) {
			throw std::runtime_error{ "Failed to locate Mono installation. Couldn't read Mono config directory key." };
		}

		auto const confimDir{ WideToUtf8(buf) };

		mono_set_dirs(fwAssemblyDir.c_str(), confimDir.c_str());

		mDomain = mono_jit_init("leopph");

		if (!mDomain) {
			throw std::runtime_error{ "Failed to create managed domain." };
		}

		mono_add_internal_call("leopph.Input::GetKeyDown", reinterpret_cast<void*>(&GetKeyDown));
		mono_add_internal_call("leopph.Input::GetKey", reinterpret_cast<void*>(&GetKeyUp));
		mono_add_internal_call("leopph.Input::get_MousePosition", reinterpret_cast<void*>(&Call<gWindow, &Window::GetMousePosition>));
		mono_add_internal_call("leopph.Input::get_MouseDelta", reinterpret_cast<void*>(&Call<gWindow, &Window::GetMouseDelta>));
		mono_add_internal_call("leopph.Input::get_IsCursorConfined", reinterpret_cast<void*>(&Call<gWindow, &Window::IsCursorConfined>));
		mono_add_internal_call("leopph.Input::set_IsCursorConfined", reinterpret_cast<void*>(&Call<gWindow, &Window::SetCursorConfinement, bool>));
		mono_add_internal_call("leopph.Input::get_IsCursorHidden", reinterpret_cast<void*>(&Call<gWindow, &Window::IsCursorHidden>));
		mono_add_internal_call("leopph.Input::set_IsCursorHidden", reinterpret_cast<void*>(&Call<gWindow, &Window::SetCursorHiding, bool>));

		mono_add_internal_call("leopph.Time::get_FullTime", reinterpret_cast<void*>(&managedbindings::get_full_time));
		mono_add_internal_call("leopph.Time::get_FrameTime", reinterpret_cast<void*>(&managedbindings::get_frame_time));

		mono_add_internal_call("leopph.Window::get_CurrentResolution", reinterpret_cast<void*>(&Call<gWindow, &Window::GetCurrentClientAreaSize>));
		mono_add_internal_call("leopph.Window::get_WindowedResolution", reinterpret_cast<void*>(&Call<gWindow, &Window::GetWindowedClientAreaSize>));
		mono_add_internal_call("leopph.Window::set_WindowedResolution", reinterpret_cast<void*>(&Call<gWindow, &Window::SetWindowedClientAreaSize, Extent2D<u32>>));
		mono_add_internal_call("leopph.Window::get_IsBorderless", reinterpret_cast<void*>(&Call<gWindow, &Window::IsBorderless>));
		mono_add_internal_call("leopph.Window::set_IsBorderless", reinterpret_cast<void*>(&Call<gWindow, &Window::SetBorderless, bool>));
		mono_add_internal_call("leopph.Window::get_IsMinimizinmOnBorderlessFocusLoss", reinterpret_cast<void*>(&Call<gWindow, &Window::IsMinimizingOnBorderlessFocusLoss>));
		mono_add_internal_call("leopph.Window::set_IsMinimizinmOnBorderlessFocusLoss", reinterpret_cast<void*>(&Call<gWindow, &Window::SetWindowMinimizeOnBorderlessFocusLoss, bool>));
		mono_add_internal_call("leopph.Window::InternalSetShouldClose", reinterpret_cast<void*>(&Call<gWindow, &Window::SetQuitSignal, bool>));

		mono_add_internal_call("leopph.NativeWrapper.get_Guid", reinterpret_cast<void*>(&managedbindings::GetManagedAccessObjectGuid));

		mono_add_internal_call("leopph.Entity::NewNativeEntity", reinterpret_cast<void*>(&managedbindings::CreateNativeEntity));
		mono_add_internal_call("leopph.Entity::get_Name", reinterpret_cast<void*>(&managedbindings::GetEntityName));
		mono_add_internal_call("leopph.Entity::set_Name", reinterpret_cast<void*>(&managedbindings::SetEntityName));
		mono_add_internal_call("leopph.Entity::get_Transform", reinterpret_cast<void*>(&managedbindings::GetEntityTransform));
		mono_add_internal_call("leopph.Entity::InternalCreateComponent", reinterpret_cast<void*>(&managedbindings::EntityCreateComponent));

		mono_add_internal_call("leopph.Component::get_Entity", reinterpret_cast<void*>(&managedbindings::GetComponentEntity));
		mono_add_internal_call("leopph.Component::get_Transform", reinterpret_cast<void*>(&managedbindings::GetComponentEntityTransform));

		mono_add_internal_call("leopph.Transform::get_WorldPosition", reinterpret_cast<void*>(&managedbindings::GetTransformWorldPosition));
		mono_add_internal_call("leopph.Transform::set_WorldPosition", reinterpret_cast<void*>(&managedbindings::SetTransformWorldPosition));
		mono_add_internal_call("leopph.Transform::get_LocalPosition", reinterpret_cast<void*>(&managedbindings::GetTransformLocalPosition));
		mono_add_internal_call("leopph.Transform::set_LocalPosition", reinterpret_cast<void*>(&managedbindings::SetTransformLocalPosition));
		mono_add_internal_call("leopph.Transform::get_WorldRotation", reinterpret_cast<void*>(&managedbindings::GetTransformWorldRotation));
		mono_add_internal_call("leopph.Transform::set_WorldRotation", reinterpret_cast<void*>(&managedbindings::SetTransformWorldRotation));
		mono_add_internal_call("leopph.Transform::get_LocalRotation", reinterpret_cast<void*>(&managedbindings::GetTransformLocalRotation));
		mono_add_internal_call("leopph.Transform::set_LocalRotation", reinterpret_cast<void*>(&managedbindings::SetTransformLocalRotation));
		mono_add_internal_call("leopph.Transform::get_WorldScale", reinterpret_cast<void*>(&managedbindings::GetTransformWorldScale));
		mono_add_internal_call("leopph.Transform::set_WorldScale", reinterpret_cast<void*>(&managedbindings::SetTransformWorldScale));
		mono_add_internal_call("leopph.Transform::get_LocalScale", reinterpret_cast<void*>(&managedbindings::GetTransformLocalScale));
		mono_add_internal_call("leopph.Transform::set_LocalScale", reinterpret_cast<void*>(&managedbindings::SetTransformLocalScale));
		mono_add_internal_call("leopph.Transform::get_Right", reinterpret_cast<void*>(&managedbindings::GetTransformRightAxis));
		mono_add_internal_call("leopph.Transform::get_Up", reinterpret_cast<void*>(&managedbindings::GetTransformUpAxis));
		mono_add_internal_call("leopph.Transform::get_Forward", reinterpret_cast<void*>(&managedbindings::GetTransformForwardAxis));
		mono_add_internal_call("leopph.Transform::get_Parent", reinterpret_cast<void*>(&managedbindings::GetTransformParent));
		mono_add_internal_call("leopph.Transform::set_Parent", reinterpret_cast<void*>(&managedbindings::SetTransformParent));
		mono_add_internal_call("leopph.Transform::get_ModelMatrix", reinterpret_cast<void*>(&managedbindings::GetTransformModelMatrix));
		mono_add_internal_call("leopph.Transform::get_NormalMatrix", reinterpret_cast<void*>(&managedbindings::GetTransformNormalMatrix));
		mono_add_internal_call("leopph.Transform::Translate(leopph.Vector3,leopph.Space)", reinterpret_cast<void*>(&managedbindings::TranslateTransformVector));
		mono_add_internal_call("leopph.Transform::Translate(single,single,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::TranslateTransform));
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Quaternion,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RotateTransform));
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Vector3,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RotateTransformAngleAxis));
		mono_add_internal_call("leopph.Transform::Rescale(leopph.Vector3,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RescaleTransformVector));
		mono_add_internal_call("leopph.Transform::Rescale(single,single,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RescaleTransform));

		mono_add_internal_call("leopph.Camera::get_Type", reinterpret_cast<void*>(&managedbindings::GetCameraType));
		mono_add_internal_call("leopph.Camera::set_Type", reinterpret_cast<void*>(&managedbindings::SetCameraType));
		mono_add_internal_call("leopph.Camera::get_PerspectiveFieldOfView", reinterpret_cast<void*>(&managedbindings::GetCameraPerspectiveFov));
		mono_add_internal_call("leopph.Camera::set_PerspectiveFieldOfView", reinterpret_cast<void*>(&managedbindings::SetCameraPerspectiveFov));
		mono_add_internal_call("leopph.Camera::get_OrthographicSize", reinterpret_cast<void*>(&managedbindings::GetCameraOrthographicSize));
		mono_add_internal_call("leopph.Camera::set_OrthographicSize", reinterpret_cast<void*>(&managedbindings::SetCameraOrthographicSize));
		mono_add_internal_call("leopph.Camera::get_NearClipPlane", reinterpret_cast<void*>(&managedbindings::GetCameraNearClipPlane));
		mono_add_internal_call("leopph.Camera::set_NearClipPlane", reinterpret_cast<void*>(&managedbindings::SetCameraNearClipPlane));
		mono_add_internal_call("leopph.Camera::get_FarClipPlane", reinterpret_cast<void*>(&managedbindings::GetCameraFarClipPlane));
		mono_add_internal_call("leopph.Camera::set_FarClipPlane", reinterpret_cast<void*>(&managedbindings::SetCameraFarClipPlane));

		mono_add_internal_call("leopph.Light::get_Color", reinterpret_cast<void*>(&managedbindings::GetLightColor));
		mono_add_internal_call("leopph.Light::set_Color", reinterpret_cast<void*>(&managedbindings::SetLightColor));
		mono_add_internal_call("leopph.Light::get_Intensity", reinterpret_cast<void*>(&managedbindings::GetLightIntensity));
		mono_add_internal_call("leopph.Light::set_Intensity", reinterpret_cast<void*>(&managedbindings::SetLightIntensity));

		auto const managedLibPath{ std::filesystem::path{ GetExecutablePath() }.remove_filename() /= "LeopphRuntimeManaged.dll" };

		auto const assembly{ mono_domain_assembly_open(mDomain, WideToUtf8(managedLibPath.c_str()).c_str()) };

		if (!assembly) {
			throw std::runtime_error{ WideToUtf8(std::format(L"Failed to open managed assembly at {}.", managedLibPath.c_str())) };
		}

		mImage = mono_assembly_get_image(assembly);

		if (!mImage) {
			throw std::runtime_error{ "Failed to image from managed assembly." };
		}

		auto const helperClass = mono_class_from_name(mImage, "leopph", "NativeHelpers");
		assert(helperClass);
		mFieldSerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeField", 1);
		assert(mFieldSerializeTestMethod);
		mPropertySerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeProperty", 1);
		assert(mPropertySerializeTestMethod);
		mGetEnumValuesMethod = mono_class_get_method_from_name(helperClass, "GetEnumValues", 1);
		assert(mGetEnumValuesMethod);
		mPrimitiveTestMethod = mono_class_get_method_from_name(helperClass, "IsPrimitive", 1);
		assert(mPrimitiveTestMethod);
		mParsePropertyValueMethod = mono_class_get_method_from_name(helperClass, "ParsePropertyValue", 2);
		assert(mParsePropertyValueMethod);
		mParseFieldValueMethod = mono_class_get_method_from_name(helperClass, "ParseFieldValue", 2);
		assert(mParseFieldValueMethod);
		mEnumToUnderlyingValueMethod = mono_class_get_method_from_name(helperClass, "EnumToUnderlyingType", 2);
		assert(mEnumToUnderlyingValueMethod);
		mParseEnumValueMethod = mono_class_get_method_from_name(helperClass, "ParseEnumValue", 2);
		assert(mParseEnumValueMethod);

		auto const componentClass = mono_class_from_name(mImage, "leopph", "Component");
		auto const behaviorClass = mono_class_from_name(mImage, "leopph", "Behavior");

		auto const table = mono_image_get_table_info(mImage, MONO_TABLE_TYPEDEF);
		auto const numRows = mono_table_info_get_rows(table);

		for (int i = 0; i < numRows; i++) {
			u32 cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table, i, cols, MONO_TYPEDEF_SIZE);
			auto const mName = mono_metadata_string_heap(mImage, cols[MONO_TYPEDEF_NAME]);
			auto const ns = mono_metadata_string_heap(mImage, cols[MONO_TYPEDEF_NAMESPACE]);
			auto const klass = mono_class_from_name(mImage, ns, mName);

			if (mono_class_is_subclass_of(klass, componentClass, false) && klass != componentClass && klass != behaviorClass) {
				mComponentClasses.emplace_back(klass);
			}
		}
	}


	auto ManagedRuntime::ShutDown() noexcept -> void {
		if (mDomain) {
			mono_jit_cleanup(mDomain);
			mDomain = nullptr;
		}
	}


	MonoImage* ManagedRuntime::GetManagedImage() const {
		return mImage;
	}


	MonoDomain* ManagedRuntime::GetManagedDomain() const {
		return mDomain;
	}


	bool ManagedRuntime::ShouldSerialize(MonoReflectionField* field) const {
		auto const res = mono_runtime_invoke(mFieldSerializeTestMethod, nullptr, reinterpret_cast<void**>(&field), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	bool ManagedRuntime::ShouldSerialize(MonoReflectionProperty* prop) const {
		auto const res = mono_runtime_invoke(mPropertySerializeTestMethod, nullptr, reinterpret_cast<void**>(&prop), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	MonoArray* ManagedRuntime::GetEnumValues(MonoReflectionType* enumType) const {
		return reinterpret_cast<MonoArray*>(mono_runtime_invoke(mGetEnumValuesMethod, nullptr, reinterpret_cast<void**>(&enumType), nullptr));
	}


	std::span<MonoClass* const> ManagedRuntime::GetComponentClasses() {
		return mComponentClasses;
	}


	bool ManagedRuntime::IsTypePrimitive(MonoReflectionType* refType) const {
		return *static_cast<int*>(mono_object_unbox(mono_runtime_invoke(mPrimitiveTestMethod, nullptr, reinterpret_cast<void**>(&refType), nullptr)));
	}


	MonoObject* ManagedRuntime::ParseValue(MonoReflectionField* field, std::string_view const str) const {
		auto const managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(field), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(mParseFieldValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}


	MonoObject* ManagedRuntime::ParseValue(MonoReflectionProperty* property, std::string_view const str) const {
		auto const managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(property), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(mParsePropertyValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}


	MonoObject* ManagedRuntime::EnumToUnderlyingType(MonoReflectionType* enumType, MonoObject* enumValue) const {
		void* params[]{ reinterpret_cast<void*>(enumType), reinterpret_cast<void*>(enumValue) };
		auto const underlyinmValue = mono_runtime_invoke(mEnumToUnderlyingValueMethod, nullptr, params, nullptr);
		return underlyinmValue;
	}


	MonoObject* ManagedRuntime::ParseEnumValue(MonoReflectionType* enumType, std::string_view const str) const {
		auto const managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(enumType), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(mParseEnumValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}
}