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
