#include "GPUScene.h"

#include <stdio.h>

namespace Columbus
{

	static void CreateGPUSceneBuffers(SPtr<DeviceVulkan> Device, u64 Size, const char* Name, Buffer*& Buf, Buffer** UploadBufs)
	{
		BufferDesc Desc;
		Desc.Size = Size;
		Desc.BindFlags = BufferType::UAV;
		Buf = Device->CreateBuffer(Desc, nullptr);
		Device->SetDebugName(Buf, Name);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			Buffer*& UploadBuf = UploadBufs[i];

			BufferDesc UploadBufferDesc(Size, BufferType::UAV);
			UploadBufferDesc.HostVisible = true;
			UploadBuf = Device->CreateBuffer(UploadBufferDesc, nullptr);

			char UploadName[256]{ 0 };
			snprintf(UploadName, 256, "%s (upload buffer %i)", Name, i+1);

			Device->SetDebugName(UploadBuf, UploadName);
		}
	}

	static void DestroyGPUSceneBuffers(SPtr<DeviceVulkan> Device, Buffer* Buf, Buffer** UploadBufs)
	{
		Device->DestroyBuffer(Buf);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			Device->DestroyBuffer(UploadBufs[i]);
		}
	}

	static GPUViewCamera CreateCameraCompact(const Camera& Camera)
	{
		return GPUViewCamera
		{
			.ViewMatrix           = Camera.GetViewMatrix(),
			.ProjectionMatrix     = Camera.GetProjectionMatrix(),
			.ViewProjectionMatrix = Camera.GetViewProjection(),

			.InverseViewMatrix           = Camera.GetViewMatrix().GetInverted(),
			.InverseProjectionMatrix     = Camera.GetProjectionMatrix().GetInverted(),
			.InverseViewProjectionMatrix = Camera.GetViewProjection().GetInverted(),

			.CameraPosition = Vector4(Camera.Pos, 0),
			.CameraForward  = Vector4(Camera.Direction(), 0),
			.CameraUp       = Vector4(Camera.Up(), 0),
			.CameraRight    = Vector4(Camera.Right(), 0),
			.Jittering      = Camera.GetJittering(),
		};
	}

	void GPUScene::Update()
	{
		for (int i = 0; i < (int)Lights.Size(); i++)
		{
			const GPULight& SceneLight = Lights.Data()[i];

			// select first directional light as sun direction
			if (SceneLight.Type == LightType::Directional)
			{
				SunDirection = Vector4(SceneLight.Direction.XYZ().Normalized(), 0);
			}
		}
	}

	GPUSceneCompact GPUScene::CreateCompact(const RenderView& View) const
	{
		Vector4 SHR(1, 0, 0, 0);
		Vector4 SHG(1, 0, 0, 0);
		Vector4 SHB(1, 0, 0, 0);

		SkySettings SkyConverted = Sky;
		// convert units
		{
			SkyConverted.PlanetRadiusKm     *= 1000;
			SkyConverted.AtmosphereHeightKm *= 1000;
			SkyConverted.RayleightHeightKm  *= 1000;
			SkyConverted.MieHeight          *= 1000;
			SkyConverted.OzonePeak          *= 1000;
			SkyConverted.OzoneFalloff       *= 1000;
		}

		return GPUSceneCompact
		{
			.CameraCur  = CreateCameraCompact(View.CameraCur),
			.CameraPrev = CreateCameraCompact(View.CameraPrev),

			.RenderSize = View.RenderSize,
			.OutputSize = View.OutputSize,

			.SunDirection = SunDirection,
			.SkySHR       = SHR,
			.SkySHG       = SHG,
			.SkySHB       = SHB,

			.MeshesCount    = (u32)Meshes.Size(),
			.MaterialsCount = (u32)Materials.size(),
			.TexturesCount  = (u32)Textures.size(),
			.LightsCount    = (u32)Lights.Size(),
			.DecalsCount    = (u32)Decals.size(),

			.Sky = SkyConverted,
		};
	}

	GPUScene* GPUScene::CreateGPUScene(SPtr<DeviceVulkan> Device)
	{
		GPUScene* Scene = new GPUScene();

		CreateGPUSceneBuffers(Device, sizeof(GPUSceneCompact), "GPUScene.Scene", Scene->SceneBuffer, Scene->SceneUploadBuffers);
		CreateGPUSceneBuffers(Device, GPUScene::MaxGPULights * sizeof(GPULight), "GPUScene.Lights", Scene->LightsBuffer, Scene->LightUploadBuffers);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMeshes * sizeof(GPUSceneMeshCompact), "GPUScene.Meshes", Scene->MeshesBuffer, Scene->MeshesUploadBuffers);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMaterials * sizeof(GPUMaterialCompact), "GPUScene.Materials", Scene->MaterialsBuffer, Scene->MaterialsUploadBuffers);
		CreateGPUSceneBuffers(Device, GPUScene::MaxDecals * sizeof(GPUDecal), "GPUScene.Decals", Scene->DecalsBuffers, Scene->DecalsUploadBuffers);

		
		// allocate empty TLAS
		{
			AccelerationStructureDesc TlasDesc;
			TlasDesc.Type = AccelerationStructureType::TLAS;
			TlasDesc.Instances = {};

			for (int i = 0; i < MaxMeshes; i++)
			{
				TlasDesc.Instances.push_back({ Matrix(1), nullptr });
			}

			Scene->TLAS = Device->CreateAccelerationStructure(TlasDesc);
		}

		return Scene;
	}

	void GPUScene::DestroyGPUScene(GPUScene* Scene, SPtr<DeviceVulkan> Device)
	{
		DestroyGPUSceneBuffers(Device, Scene->SceneBuffer, Scene->SceneUploadBuffers);
		DestroyGPUSceneBuffers(Device, Scene->LightsBuffer, Scene->LightUploadBuffers);
		DestroyGPUSceneBuffers(Device, Scene->MeshesBuffer, Scene->MeshesUploadBuffers);
		DestroyGPUSceneBuffers(Device, Scene->MaterialsBuffer, Scene->MaterialsUploadBuffers);
		DestroyGPUSceneBuffers(Device, Scene->DecalsBuffers, Scene->DecalsUploadBuffers);

		Device->DestroyAccelerationStructure(Scene->TLAS);
	}
}
