#pragma once

#include "Core.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "SkyboxComponent.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>

#include <span>
#include <vector>


namespace leopph::renderer {
class View {
public:
	[[nodiscard]] virtual auto CalculateFrustum(float aspectRatio) const noexcept -> Frustum = 0;

	View() = default;
	View(View const& other) = default;
	View(View&& other) noexcept = default;

	auto operator=(View const& other) -> View& = default;
	auto operator=(View&& other) noexcept -> View& = default;

	virtual ~View() = default;
};


class Camera : public View {
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
	enum class Side : std::uint8_t {
		Vertical   = 0,
		Horizontal = 1
	};


	[[nodiscard]] virtual auto GetPosition() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetForwardAxis() const noexcept -> Vector3 = 0;

	LEOPPHAPI [[nodiscard]] auto GetNearClipPlane() const noexcept -> float;
	LEOPPHAPI auto SetNearClipPlane(float nearClipPlane) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetFarClipPlane() const noexcept -> float;
	LEOPPHAPI auto SetFarClipPlane(float farClipPlane) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetType() const noexcept -> Type;
	LEOPPHAPI auto SetType(Type type) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetHorizontalPerspectiveFov() const -> float;
	LEOPPHAPI auto SetHorizontalPerspectiveFov(float degrees) -> void;

	LEOPPHAPI [[nodiscard]] auto GetHorizontalOrthographicSize() const -> float;
	LEOPPHAPI auto SetHorizontalOrthographicSize(float size) -> void;

	LEOPPHAPI [[nodiscard]] auto CalculateFrustum(float aspectRatio) const noexcept -> Frustum override;
	LEOPPHAPI [[nodiscard]] auto CalculateViewMatrix() const noexcept -> Matrix4;
	LEOPPHAPI [[nodiscard]] auto CalculateProjectionMatrix(float aspectRatio) const noexcept -> Matrix4;

	LEOPPHAPI [[nodiscard]] static auto HorizontalPerspectiveFovToVertical(float fovDegrees, float aspectRatio) noexcept -> float;
	LEOPPHAPI [[nodiscard]] static auto VerticalPerspectiveFovToHorizontal(float fovDegrees, float aspectRatio) noexcept -> float;

	Camera() = default;
	Camera(Camera const& other) = default;
	Camera(Camera&& other) noexcept = default;

	auto operator=(Camera const& other) -> Camera& = default;
	auto operator=(Camera&& other) noexcept -> Camera& = default;

	~Camera() override = default;
};


struct Visibility {
	std::span<LightComponent const*> allLights;
	std::vector<int> visibleLightIndices;

	std::span<StaticMeshComponent const*> allStaticMeshes;
	std::vector<int> visibleStaticMeshIndices;
};


LEOPPHAPI auto StartUp() -> void;
LEOPPHAPI auto ShutDown() noexcept -> void;

LEOPPHAPI auto DrawGame() noexcept -> void;
LEOPPHAPI auto DrawSceneView(Camera const& cam) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSceneResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetSceneResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameFrame() noexcept -> ID3D11ShaderResourceView*;
LEOPPHAPI [[nodiscard]] auto GetSceneFrame() noexcept -> ID3D11ShaderResourceView*;

LEOPPHAPI [[nodiscard]] auto GetGameAspectRatio() noexcept -> f32;
LEOPPHAPI [[nodiscard]] auto GetSceneAspectRatio() noexcept -> f32;

LEOPPHAPI auto BindAndClearSwapChain() noexcept -> void;

LEOPPHAPI auto Present() noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSyncInterval() noexcept -> u32;
LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

LEOPPHAPI [[nodiscard]] auto GetDevice() noexcept -> ID3D11Device*;
LEOPPHAPI [[nodiscard]] auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

LEOPPHAPI [[nodiscard]] auto GetDefaultMaterial() noexcept -> Material*;
LEOPPHAPI [[nodiscard]] auto GetCubeMesh() noexcept -> Mesh*;
LEOPPHAPI [[nodiscard]] auto GetPlaneMesh() noexcept -> Mesh*;

LEOPPHAPI [[nodiscard]] auto GetGamma() noexcept -> f32;
LEOPPHAPI auto SetGamma(f32 gamma) noexcept -> void;

LEOPPHAPI auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
LEOPPHAPI auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

LEOPPHAPI auto RegisterGameCamera(Camera const& cam) -> void;
LEOPPHAPI auto UnregisterGameCamera(Camera const& cam) -> void;

LEOPPHAPI auto CullLights(Frustum const& frust, Matrix4 const& viewMtx, Visibility& visibility) -> void;
LEOPPHAPI auto CullStaticMeshComponents(Frustum const& frust, Matrix4 const& viewMtx, Visibility& visibility) -> void;
}
