#include "RenderPasses.h"

namespace Columbus
{

	void UploadGPUSceneRG(RenderGraph& Graph)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("UploadGPUScene", RenderGraphPassType::Compute, Parameters, Dependencies, [](RenderGraphContext& Context)
		{
			// TODO: create UploadBuffer wrapper? have CurrentFrame as a constant in Context?
			static int CurrentFrame = 0;
			CurrentFrame = ++CurrentFrame % MaxFramesInFlight;

			// scene
			{
				Buffer*& UploadBuffer = Context.Scene->SceneUploadBuffers[CurrentFrame];

				GPUSceneCompact Compact = Context.Scene->CreateCompact();

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				memcpy(Ptr, &Compact, sizeof(Compact));
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->SceneBuffer, 0, 0, sizeof(Compact));
			}

			// lights
			if (Context.Scene->Lights.size() > 0)
			{
				Buffer*& UploadBuffer = Context.Scene->LightUploadBuffers[CurrentFrame];

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				memcpy(Ptr, Context.Scene->Lights.data(), sizeof(GPULight) * Context.Scene->Lights.size());
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->LightsBuffer, 0, 0, sizeof(GPULight) * Context.Scene->Lights.size());
			}

			// meshes
			if (Context.Scene->Meshes.size() > 0)
			{
				Buffer*& UploadBuffer = Context.Scene->MeshesUploadBuffers[CurrentFrame];

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				for (int i = 0; i < Context.Scene->Meshes.size(); i++)
				{
					GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
					GPUSceneMeshCompact Compact {
						.Transform = Mesh.Transform,
						.VertexBufferAddress = Mesh.Vertices->GetDeviceAddress(),
						.IndexBufferAddress = Mesh.Indices->GetDeviceAddress(),
						.Uv1BufferAddress = Mesh.UV1->GetDeviceAddress(),
						.Uv2BufferAddress = Mesh.UV2 ? Mesh.UV2->GetDeviceAddress() : 0,
						.NormalsBufferAddress = Mesh.Normals->GetDeviceAddress(),
						.TangentsBufferAddress = Mesh.Tangents->GetDeviceAddress(),
						.VertexCount = Mesh.VertexCount,
						.IndexCount = Mesh.IndicesCount,
						.MaterialId = Mesh.MaterialId,
						.LightmapId = Mesh.LightmapId,
					};

					((GPUSceneMeshCompact*)Ptr)[i] = Compact;
				}
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->MeshesBuffer, 0, 0, sizeof(GPUSceneMeshCompact) * Context.Scene->Meshes.size());
			}

			// materials
			if (Context.Scene->Materials.size() > 0)
			{
				Buffer*& UploadBuffer = Context.Scene->MaterialsUploadBuffers[CurrentFrame];

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				for (int i = 0; i < Context.Scene->Materials.size(); i++)
				{
					Material& Mat = Context.Scene->Materials[i];
					GPUMaterialCompact Compact{
						.AlbedoFactor = Mat.AlbedoFactor,
						.EmissiveFactor = Mat.EmissiveFactor,
						.AlbedoId = Mat.AlbedoId,
						.NormalId = Mat.NormalId,
						.OrmID = Mat.OrmId,
						.EmissiveId = Mat.EmissiveId,
						.Roughness = Mat.Roughness,
						.Metallic = Mat.Metallic,
					};
					((GPUMaterialCompact*)Ptr)[i] = Compact;
				}
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->MaterialsBuffer, 0, 0, sizeof(GPUMaterialCompact) * Context.Scene->Materials.size());
			}
		});
	}

}
