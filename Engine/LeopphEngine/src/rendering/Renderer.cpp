#include "Renderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "Context.hpp"
#include "GlContext.hpp"
#include "Math.hpp"
#include "Window.hpp"
#include "../../../include/Entity.hpp"

#include <algorithm>
#include <array>
#include <cmath>


namespace leopph::internal
{
	void Renderer::render()
	{
		// Clean unused resources

		std::erase_if(mMaterialsToMeshes, [](auto const& elem)
		{
			return elem.second.size() == 0;
		});


		// Update framebuffers if needed

		if (mResUpdateFlags.renderRes)
		{
			delete_ping_pong_buffers();
			create_ping_pong_buffers();
			mResUpdateFlags.renderRes = false;
		}

		// Extract per frame data

		mFrameData.uboPerFrameData.ambient = AmbientLight::Instance().Intensity();
		mFrameData.uboPerFrameData.lightCount = 0;

		for (auto i = 0; i < mDirLights.size() && mFrameData.uboPerFrameData.lightCount < NUM_MAX_LIGHTS; i++, mFrameData.uboPerFrameData.lightCount++)
		{
			auto const& light = *mDirLights[i];
			auto& uboLight = mFrameData.uboPerFrameData.lights[mFrameData.uboPerFrameData.lightCount];

			uboLight.color = light.get_color();
			uboLight.intensity = light.get_intensity();
			uboLight.direction = light.get_direction();
			uboLight.type = static_cast<i32>(UboLightType::Directional);
		}


		for (auto i = 0; i < mSpotLights.size() && mFrameData.uboPerFrameData.lightCount < NUM_MAX_LIGHTS; i++, mFrameData.uboPerFrameData.lightCount++)
		{
			auto const& light = *mSpotLights[i];
			auto& uboLight = mFrameData.uboPerFrameData.lights[mFrameData.uboPerFrameData.lightCount];

			uboLight.color = light.get_color();
			uboLight.intensity = light.get_intensity();
			uboLight.direction = light.get_owner()->get_forward_axis();
			uboLight.position = light.get_owner()->get_position();
			uboLight.range = light.get_range();
			uboLight.innerCos = std::cos(to_radians(light.get_inner_angle()));
			uboLight.outerCos = std::cos(to_radians(light.get_outer_angle()));
			uboLight.type = static_cast<i32>(UboLightType::Spot);
		}


		for (auto i = 0; i < mPointLights.size() && mFrameData.uboPerFrameData.lightCount < NUM_MAX_LIGHTS; i++, mFrameData.uboPerFrameData.lightCount++)
		{
			auto const& light = *mPointLights[i];
			auto& uboLight = mFrameData.uboPerFrameData.lights[mFrameData.uboPerFrameData.lightCount];

			uboLight.color = light.get_color();
			uboLight.intensity = light.get_intensity();
			uboLight.position = light.get_owner()->get_position();
			uboLight.range = light.get_range();
			uboLight.type = static_cast<i32>(UboLightType::Point);
		}


		// Extract per view data

		mFrameData.perViewData.clear();


		for (auto const* const cam : mCameras)
		{
			mFrameData.perViewData.emplace_back();
			auto& [viewport, uboPerCameraData, materialNodes] = mFrameData.perViewData.back();

			viewport = cam->get_window_extents();
			uboPerCameraData.viewMat = cam->build_view_matrix();
			uboPerCameraData.viewMatInv = uboPerCameraData.viewMat.inverse();
			uboPerCameraData.projMat = cam->build_projection_matrix();
			uboPerCameraData.projMatInv = uboPerCameraData.projMat.inverse();
			uboPerCameraData.viewProjMat = uboPerCameraData.viewMat * uboPerCameraData.projMat;
			uboPerCameraData.viewProjMatInv = uboPerCameraData.viewProjMat.inverse();
			uboPerCameraData.position = cam->get_owner()->get_position();

			for (auto const& [material, meshes] : mMaterialsToMeshes)
			{
				MaterialNode matNode{};

				for (auto const& mesh : meshes)
				{
					MeshNode meshNode{};

					for (auto const* const entity : mesh->get_entities())
					{
						if (mesh->get_bounding_box().is_visible_in_frustum(entity->get_model_matrix() * uboPerCameraData.viewProjMat))
						{
							meshNode.matrices.emplace_back(entity->get_model_matrix().transpose(), entity->get_normal_matrix().transpose()); // transpose because of OpenGL
						}
					}

					if (!meshNode.matrices.empty())
					{
						meshNode.mesh = mesh;
						matNode.meshNodes.emplace_back(std::move(meshNode));
					}
				}

				if (!matNode.meshNodes.empty())
				{
					matNode.material = material;
					materialNodes.emplace_back(std::move(matNode));
				}
			}
		}

		*static_cast<UboPerFrameData*>(mPerFrameUbo.get_ptr()) = mFrameData.uboPerFrameData;


		// Opaque pass

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);

		GLfloat constexpr clearColor[]{0, 0, 0, 1};
		GLfloat constexpr clearDepth{1};
		glClearNamedFramebufferfv(mPingPongBuffers[0].name, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(mPingPongBuffers[0].name, GL_DEPTH, 0, &clearDepth);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mPingPongBuffers[0].name);

		mForwardShader.use();

		for (auto const& [viewport, uboPerCameraData, materialNodes] : mFrameData.perViewData)
		{
			glViewport(viewport.offsetX, viewport.offsetY, viewport.width, viewport.height);

			*static_cast<UboPerCameraData*>(mPerCameraUbo.get_ptr()) = uboPerCameraData;
			glBindBufferRange(GL_UNIFORM_BUFFER, 1, mPerCameraUbo.get_internal_handle(), 0, sizeof UboPerCameraData);

			for (auto const& [material, meshNodes] : materialNodes)
			{
				material->bind_and_set_renderstate(2);

				for (auto const& [mesh, matrices] : meshNodes)
				{
					mesh->set_instance_data(matrices);
					mesh->draw();
				}
			}
		}

		glBlitNamedFramebuffer(mPingPongBuffers[0].name, 0, 0, 0, mPingPongBuffers[0].width, mPingPongBuffers[0].height, 0, 0, mPingPongBuffers[0].width, mPingPongBuffers[0].height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		++mFrameCount;
	}



	void Renderer::register_dir_light(DirectionalLight const* dirLight)
	{
		mDirLights.push_back(dirLight);
	}



	void Renderer::unregister_dir_light(DirectionalLight const* dirLight)
	{
		std::erase(mDirLights, dirLight);
	}



	void Renderer::register_spot_light(SpotLight const* spotLight)
	{
		mSpotLights.push_back(spotLight);
	}



	void Renderer::unregister_spot_light(SpotLight const* spotLight)
	{
		std::erase(mSpotLights, spotLight);
	}



	void Renderer::register_point_light(PointLight const* pointLight)
	{
		mPointLights.push_back(pointLight);
	}



	void Renderer::unregister_point_light(PointLight const* pointLight)
	{
		std::erase(mPointLights, pointLight);
	}



	void Renderer::register_camera(Camera const* camera)
	{
		mCameras.push_back(camera);
	}



	void Renderer::unregister_camera(Camera const* camera)
	{
		std::erase(mCameras, camera);
	}



	void Renderer::register_mesh_for_material(std::shared_ptr<StaticMaterial const> const& material, std::shared_ptr<StaticMesh> mesh)
	{
		mMaterialsToMeshes[material].emplace_back(std::move(mesh));
	}



	void Renderer::unregister_mesh_for_material(std::shared_ptr<StaticMaterial const> const& material, std::shared_ptr<StaticMesh> const& mesh)
	{
		std::erase(mMaterialsToMeshes[material], mesh);
	}



	void Renderer::create_ping_pong_buffers()
	{
		for (auto& [name, depthStencilAtt, width, height, clrAtts] : mPingPongBuffers)
		{
			glCreateFramebuffers(1, &name);
			width = get_window()->get_width();
			height = get_window()->get_height();

			clrAtts.resize(1);
			glCreateTextures(GL_TEXTURE_2D, 1, clrAtts.data());
			glTextureStorage2D(clrAtts[0], 1, GL_RGBA8, width, height);

			glCreateTextures(GL_TEXTURE_2D, 1, &depthStencilAtt);
			glTextureStorage2D(depthStencilAtt, 1, GL_DEPTH24_STENCIL8, width, height);

			glNamedFramebufferTexture(name, GL_COLOR_ATTACHMENT0, clrAtts[0], 0);
			glNamedFramebufferTexture(name, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilAtt, 0);

			glNamedFramebufferDrawBuffer(name, GL_COLOR_ATTACHMENT0);
		}
	}



	void Renderer::delete_ping_pong_buffers()
	{
		for (auto const& [name, depthStencilAtt, width, height, clrAtts] : mPingPongBuffers)
		{
			glDeleteTextures(1, clrAtts.data());
			glDeleteTextures(1, &depthStencilAtt);
			glDeleteFramebuffers(1, &name);
		}
	}



	Renderer::Renderer()
	{
		create_ping_pong_buffers();

		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, mPerFrameUbo.get_internal_handle(), 0, sizeof UboPerFrameData);
	}



	Renderer::~Renderer()
	{
		delete_ping_pong_buffers();
	}



	/*void Renderer::calculate_shadow_cascades(std::vector<ShadowCascade>& out)
	{
		auto const frust = (*mMainCam)->Frustum();
		auto const& frustNearZ = frust.NearBottomLeft[2];
		auto const& frustFarZ = frust.FarBottomLeft[2];
		auto const frustDepth = frustFarZ - frustNearZ;
		auto const numCascades = static_cast<u8>(mDirShadowRes.size());
		auto const lightViewMat = Matrix4::LookAt(Vector3{0}, (*mDirLight)->Direction(), Vector3::Up());
		auto const camViewToLightViewMat = (*mMainCam)->ViewMatrix().Inverse() * lightViewMat;

		out.resize(numCascades);

		// Calculate the cascade near and far planes on the Z axis in main camera space

		out[0].near = frustNearZ;
		for (auto i = 1; i < numCascades; i++)
		{
			out[i].near = mDirCorr * frustNearZ * math::Pow(frustFarZ / frustNearZ, static_cast<f32>(i) / static_cast<f32>(numCascades)) + (1 - mDirCorr) * (frustNearZ + static_cast<f32>(i) / static_cast<f32>(numCascades) * (frustFarZ - frustNearZ));
			// On bound borders the far plane is multiplied by this value to avoid precision problems.
			auto constexpr borderCorrection = 1.005f;
			out[i - 1].far = out[i].near * borderCorrection;
		}
		out[numCascades - 1].far = frustFarZ;

		for (auto& cascade : out)
		{
			// The normalized distance of the cascades faces from the near frustum face.

			auto const cascadeNearDist = (cascade.near - frustNearZ) / frustDepth;
			auto const cascadeFarDist = (cascade.far - frustNearZ) / frustDepth;

			// The cascade vertices in camera view space.
			std::array const cascadeVertsCam
			{
				Vector4{math::lerp(frust.NearTopLeft, frust.FarTopLeft, cascadeNearDist), 1.f},
				Vector4{math::lerp(frust.NearBottomLeft, frust.FarBottomLeft, cascadeNearDist), 1.f},
				Vector4{math::lerp(frust.NearBottomRight, frust.FarBottomRight, cascadeNearDist), 1.f},
				Vector4{math::lerp(frust.NearTopRight, frust.FarTopRight, cascadeNearDist), 1.f},
				Vector4{math::lerp(frust.NearTopLeft, frust.FarTopLeft, cascadeFarDist), 1.f},
				Vector4{math::lerp(frust.NearBottomLeft, frust.FarBottomLeft, cascadeFarDist), 1.f},
				Vector4{math::lerp(frust.NearBottomRight, frust.FarBottomRight, cascadeFarDist), 1.f},
				Vector4{math::lerp(frust.NearTopRight, frust.FarTopRight, cascadeFarDist), 1.f},
			};

			// The light view space mininum point of the bounding box of the cascade
			Vector3 bBoxMinLight{std::numeric_limits<float>::max()};

			// The light view space maximum point of the bounding box of the cascade
			Vector3 bBoxMaxLight{std::numeric_limits<float>::min()};

			// Calculate the bounding box min and max points by transforming the vertices to light space.
			std::ranges::for_each(cascadeVertsCam, [&](auto const& vertex)
			{
				auto const vertLight = vertex * camViewToLightViewMat;
				bBoxMinLight = Vector3{std::min(bBoxMinLight[0], vertLight[0]), std::min(bBoxMinLight[1], vertLight[1]), std::min(bBoxMinLight[2], vertLight[2])};
				bBoxMaxLight = Vector3{std::max(bBoxMaxLight[0], vertLight[0]), std::max(bBoxMaxLight[1], vertLight[1]), std::max(bBoxMaxLight[2], vertLight[2])};
			});

			// The projection matrix that uses the calculated min/max values of the bounding box. Essentially THE bounding box + the near clip offset of the DirectionalLight.
			auto const lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2] - (*mDirLight)->ShadowExtension(), bBoxMaxLight[2])};

			cascade.wordToClip = lightViewMat * lightProjMat;
		}
	}*/
}
