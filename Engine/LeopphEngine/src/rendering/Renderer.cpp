#include "Renderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "Context.hpp"
#include "GlCore.hpp"
#include "Math.hpp"
#include "RenderSettings.hpp"
#include "Window.hpp"
#include "../../../include/Entity.hpp"

#include <algorithm>
#include <array>
#include <cmath>


namespace leopph::internal
{
	void Renderer::render()
	{
		clean_unused_resources();

		if (mResUpdateFlags.renderRes)
		{
			update_framebuffers();
			mResUpdateFlags.renderRes = false;
		}

		mFrameData.uboPerFrameData.ambientLight.intensity = AmbientLight::Instance().Intensity();
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


		mFrameData.meshes.clear();
		if (mFrameData.viewCount < mCameras.size())
		{
			mFrameData.perViewData.resize(mCameras.size());
		}
		mFrameData.viewCount = mCameras.size();


		for (auto i = 0; i < mFrameData.viewCount; i++)
		{
			auto& [viewport, viewMatrix, projMatrix, viewProjMatrix, transformMatrices] = mFrameData.perViewData[i];
			auto const& cam = mCameras[i];

			viewport = cam->get_window_extents();
			viewMatrix = cam->build_view_matrix();
			projMatrix = cam->build_projection_matrix();
			viewProjMatrix = viewMatrix * projMatrix;

			transformMatrices.clear();

			for (auto const& meshWeak : mStaticMeshes)
			{
				for (auto const mesh = meshWeak.lock();
				     auto const* const entity : mesh->get_entities())
				{
					if (auto const mvp = entity->get_model_matrix() * viewProjMatrix;
						mesh->get_bounding_box().is_visible_in_frustum(mvp))
					{
						mFrameData.meshes.emplace(mesh);
						transformMatrices[mesh.get()].emplace_back(mvp.transposed(), entity->get_normal_matrix().transposed()); // Transposed, because OpenGL buffer
					}
				}
			}
		}


		*static_cast<UboPerFrameData*>(mPerFrameUbo.get_ptr()) = mFrameData.uboPerFrameData;
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, mPerFrameUbo.get_internal_handle(), 0, sizeof UboPerFrameData);

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

		for (auto i = 0; i < mFrameData.perViewData.size(); i++)
		{
			auto const& [viewport, uboPerCameraData, materialNodes, materialNodeCount] = mFrameData.perViewData[i];

			glViewport(viewport.offsetX, viewport.offsetY, viewport.width, viewport.height);

			*static_cast<UboPerCameraData*>(mPerCameraUbo.get_ptr()) = uboPerCameraData;
			glBindBufferRange(GL_UNIFORM_BUFFER, 1, mPerCameraUbo.get_internal_handle(), 0, sizeof UboPerCameraData);

			for (auto j = 0; j < materialNodeCount; j++)
			{
				auto const& [material, meshNodes, meshNodeCount] = materialNodes[j];

				material->bind_and_set_renderstate(2);

				for (auto k = 0; k < meshNodeCount; k++)
				{
					auto const& [mesh, uboPerMesh, matrices, subMeshIndices] = meshNodes[k];

					// Update mesh instance buffer with matrices

					for (auto const subMeshIndex : subMeshIndices)
					{
						mesh->draw_sub_mesh(subMeshIndex);
					}
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



	void Renderer::register_texture_2d(std::weak_ptr<Texture2D> tex)
	{
		mTexture2Ds.emplace_back(std::move(tex));
	}



	void Renderer::unregister_texture_2d(std::weak_ptr<Texture2D> const& tex)
	{
		std::erase(mTexture2Ds, tex);
	}



	void Renderer::register_static_material(std::weak_ptr<StaticMaterial> mat)
	{
		mStaticMaterials.emplace_back(std::move(mat));
	}



	void Renderer::unregister_static_material(std::weak_ptr<StaticMaterial> const& mat)
	{
		std::erase(mStaticMaterials, mat);
	}



	void Renderer::register_static_mesh(std::weak_ptr<StaticMesh> mesh)
	{
		mStaticMeshes.emplace_back(std::move(mesh));
	}



	void Renderer::unregister_static_mesh(std::weak_ptr<StaticMesh> const& mesh)
	{
		std::erase(mStaticMeshes, mesh);
	}



	void Renderer::update_framebuffers()
	{
		auto const* window = get_window();
		auto const width = window->get_width();
		auto const height = window->get_height();

		for (auto& buf : mPingPongBuffers)
		{
			if (buf.width != width || buf.height != height)
			{
				glDeleteFramebuffers(1, &buf.name);
				glDeleteBuffers(1, &buf.depthStencilAtt);
				glDeleteBuffers(1, &buf.clrAtts[0]);

				buf.width = width;
				buf.height = height;


				glCreateTextures(GL_TEXTURE_2D, 1, &buf.clrAtts[0]);
				glTextureStorage2D(buf.clrAtts[0], 1, GL_RGBA8, buf.width, buf.height);

				glCreateTextures(GL_TEXTURE_2D, 1, &buf.depthStencilAtt);
				glTextureStorage2D(buf.depthStencilAtt, 1, GL_DEPTH24_STENCIL8, buf.width, buf.height);

				glCreateFramebuffers(1, &buf.name);
				glNamedFramebufferTexture(buf.name, GL_COLOR_ATTACHMENT0, buf.clrAtts[0], 0);
				glNamedFramebufferTexture(buf.name, GL_DEPTH_STENCIL_ATTACHMENT, buf.depthStencilAtt, 0);
				glNamedFramebufferDrawBuffer(buf.name, GL_COLOR_ATTACHMENT0);
			}
		}
	}



	void Renderer::clean_unused_resources()
	{
		for (auto it = std::begin(mTexture2Ds); it != std::end(mTexture2Ds); ++it)
		{
			if (it->expired())
			{
				std::erase(mTexture2Ds, it);
				it = std::begin(mTexture2Ds);
			}
		}

		for (auto it = std::begin(mStaticMaterials); it != std::end(mStaticMaterials); ++it)
		{
			if (it->first.expired())
			{
				mStaticMaterials.erase(it);
				it = std::begin(mStaticMaterials);
			}
		}

		for (auto it = std::begin(mStaticMeshes); it != std::end(mStaticMeshes); ++it)
		{
			if (it->expired())
			{
				std::erase(mStaticMeshes, it);
				it = std::begin(mStaticMeshes);
			}
		}
	}



	void Renderer::prepare()
	{
		// Calculate additional camera matrices
		mCamData.viewMatInv = mCamData.viewMat.inverse();
		mCamData.projMatInv = mCamData.projMat.inverse();
		mCamData.viewProjMat = mCamData.viewMat * mCamData.projMat;
		mCamData.viewProjMatInv = mCamData.viewProjMat.inverse();

		// Sort punctual lights by distance to camera

		auto const distToCam = [this](auto const& lightData)
		{
			return Vector3::distance(lightData.position, this->mCamData.position);
		};

		std::ranges::sort(mSpotLightData, std::ranges::less{}, distToCam);
		std::ranges::sort(mPointLightData, std::ranges::less{}, distToCam);

		// Select first N of the lights
		mSpotLightData.resize(RenderSettings::get_max_spot_light_count());
		mPointLightData.resize(RenderSettings::get_max_point_light_count());
	}



	void Renderer::OnEventReceived(EventReceiver<WindowEvent>::EventParamType)
	{
		mResUpdateFlags.renderRes = true;
	}



	Renderer::Renderer()
	{
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}



	Renderer::~Renderer()
	{ }



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
