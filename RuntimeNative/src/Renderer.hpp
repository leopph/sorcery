#pragma once

#include "Core.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "SkyboxComponent.hpp"
#include "shaders\ShadowFilteringModes.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>

#include <vector>


namespace leopph::renderer {
class Camera {
public:
	enum class Type : std::uint8_t {
		Perspective  = 0,
		Orthographic = 1
	};

private:
	constexpr static float MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE{ 0.03f };
	constexpr static float MINIMUM_PERSPECTIVE_FAR_CLIP_PLANE_OFFSET{ 0.1f };
	constexpr static float MINIMUM_PERSPECTIVE_HORIZONTAL_FOV{ 5.0f };
	constexpr static float MINIMUM_ORTHOGRAPHIC_HORIZONTAL_SIZE{ 0.1f };

	float mNear{ MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE };
	float mFar{ 100.f };
	float mOrthoSizeHoriz{ 10 };
	float mPerspFovHorizDeg{ 90 };
	Type mType{ Type::Perspective };

public:
	[[nodiscard]] virtual auto GetPosition() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetRightAxis() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetUpAxis() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetForwardAxis() const noexcept -> Vector3 = 0;

	[[nodiscard]] LEOPPHAPI auto GetNearClipPlane() const noexcept -> float;
	LEOPPHAPI auto SetNearClipPlane(float nearClipPlane) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetFarClipPlane() const noexcept -> float;
	LEOPPHAPI auto SetFarClipPlane(float farClipPlane) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetType() const noexcept -> Type;
	LEOPPHAPI auto SetType(Type type) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetHorizontalPerspectiveFov() const -> float;
	LEOPPHAPI auto SetHorizontalPerspectiveFov(float degrees) -> void;

	[[nodiscard]] LEOPPHAPI auto GetHorizontalOrthographicSize() const -> float;
	LEOPPHAPI auto SetHorizontalOrthographicSize(float size) -> void;

	[[nodiscard]] LEOPPHAPI auto CalculateViewMatrix() const noexcept -> Matrix4;
	[[nodiscard]] LEOPPHAPI auto CalculateProjectionMatrix(float aspectRatio) const noexcept -> Matrix4;

	[[nodiscard]] LEOPPHAPI static auto HorizontalPerspectiveFovToVertical(float fovDegrees, float aspectRatio) noexcept -> float;
	[[nodiscard]] LEOPPHAPI static auto VerticalPerspectiveFovToHorizontal(float fovDegrees, float aspectRatio) noexcept -> float;

	Camera() = default;
	Camera(Camera const& other) = default;
	Camera(Camera&& other) noexcept = default;

	auto operator=(Camera const& other) -> Camera& = default;
	auto operator=(Camera&& other) noexcept -> Camera& = default;

	virtual ~Camera() = default;
};


struct Visibility {
	std::vector<int> lightIndices;
	std::vector<int> staticMeshIndices;
};


enum class ShadowFilteringMode {
	None        = SHADOW_FILTERING_NONE,
	HardwarePCF = SHADOW_FILTERING_HARDWARE_PCF,
	PCF3x3      = SHADOW_FILTERING_PCF_3x3,
	PCFTent3x3  = SHADOW_FILTERING_PCF_TENT_3x3,
	PCFTent5x5  = SHADOW_FILTERING_PCF_TENT_5x5
};


LEOPPHAPI auto StartUp() -> void;
LEOPPHAPI auto ShutDown() noexcept -> void;

LEOPPHAPI auto DrawGame() -> void;
LEOPPHAPI auto DrawSceneView(Camera const& cam) -> void;

[[nodiscard]] LEOPPHAPI auto GetGameResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;

[[nodiscard]] LEOPPHAPI auto GetSceneResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetSceneResolution(Extent2D<u32> resolution) noexcept -> void;

[[nodiscard]] LEOPPHAPI auto GetGameFrame() noexcept -> ID3D11ShaderResourceView*;
[[nodiscard]] LEOPPHAPI auto GetSceneFrame() noexcept -> ID3D11ShaderResourceView*;

[[nodiscard]] LEOPPHAPI auto GetGameAspectRatio() noexcept -> f32;
[[nodiscard]] LEOPPHAPI auto GetSceneAspectRatio() noexcept -> f32;

LEOPPHAPI auto BindAndClearSwapChain() noexcept -> void;

LEOPPHAPI auto Present() noexcept -> void;

[[nodiscard]] LEOPPHAPI auto GetSyncInterval() noexcept -> u32;
LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

[[nodiscard]] LEOPPHAPI auto GetDevice() noexcept -> ID3D11Device*;
[[nodiscard]] LEOPPHAPI auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

[[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() noexcept -> Material*;
[[nodiscard]] LEOPPHAPI auto GetCubeMesh() noexcept -> Mesh*;
[[nodiscard]] LEOPPHAPI auto GetPlaneMesh() noexcept -> Mesh*;

[[nodiscard]] LEOPPHAPI auto GetGamma() noexcept -> f32;
LEOPPHAPI auto SetGamma(f32 gamma) noexcept -> void;

LEOPPHAPI auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
LEOPPHAPI auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

LEOPPHAPI auto RegisterGameCamera(Camera const& cam) -> void;
LEOPPHAPI auto UnregisterGameCamera(Camera const& cam) -> void;

LEOPPHAPI auto CullLights(Frustum const& frustumWS, Visibility& visibility) -> void;
LEOPPHAPI auto CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) -> void;

LEOPPHAPI auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;

[[nodiscard]] LEOPPHAPI auto GetShadowCascadeCount() noexcept -> int;
LEOPPHAPI auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto GetMaxShadowCascadeCount() noexcept -> int;
[[nodiscard]] LEOPPHAPI auto GetNormalizedShadowCascadeSplits() noexcept -> std::span<float const>;
LEOPPHAPI auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto GetShadowDistance() noexcept -> float;
LEOPPHAPI auto SetShadowDistance(float shadowDistance) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto IsVisualizingShadowCascades() noexcept -> bool;
LEOPPHAPI auto VisualizeShadowCascades(bool visualize) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto IsUsingStableShadowCascadeProjection() noexcept -> bool;
LEOPPHAPI auto UseStableShadowCascadeProjection(bool useStableProj) noexcept -> void;

[[nodiscard]] LEOPPHAPI auto GetShadowFilteringMode() noexcept -> ShadowFilteringMode;
LEOPPHAPI auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;
}
