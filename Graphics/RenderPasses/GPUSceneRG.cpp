#include "RenderPasses.h"

namespace Columbus
{

	void UploadGPUSceneRG(RenderGraph& Graph, const RenderView& View)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("UploadGPUScene + Build TLAS", RenderGraphPassType::Compute, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			// TODO: create UploadBuffer wrapper? have CurrentFrame as a constant in Context?
			static int CurrentFrame = 0;
			CurrentFrame = ++CurrentFrame % MaxFramesInFlight;

			Context.Scene->Update();

			// scene
			{
				Buffer*& UploadBuffer = Context.Scene->SceneUploadBuffers[CurrentFrame];

				GPUSceneCompact Compact = Context.Scene->CreateCompact(View);

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				memcpy(Ptr, &Compact, sizeof(Compact));
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->SceneBuffer, 0, 0, sizeof(Compact));
			}

			// lights
			if (Context.Scene->Lights.Size() > 0)
			{
				Buffer*& UploadBuffer = Context.Scene->LightUploadBuffers[CurrentFrame];

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				memcpy(Ptr, Context.Scene->Lights.Data(), Context.Scene->Lights.Size() * sizeof(GPULight));
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->LightsBuffer, 0, 0, sizeof(GPULight) * Context.Scene->Lights.Size());
			}

			// meshes
			if (Context.Scene->Meshes.Size() > 0)
			{
				Buffer*& UploadBuffer = Context.Scene->MeshesUploadBuffers[CurrentFrame];

				void* Ptr = Context.Device->MapBuffer(UploadBuffer);
				for (int i = 0; i < Context.Scene->Meshes.Size(); i++)
				{
					const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];
					GPUSceneMeshCompact Compact {
						.Transform = Mesh.Transform,
						.VertexBufferAddress = Mesh.MeshResource->Vertices->GetDeviceAddress(),
						.IndexBufferAddress = Mesh.MeshResource->Indices->GetDeviceAddress(),
						.Uv1BufferAddress = Mesh.MeshResource->UV1->GetDeviceAddress(),
						.Uv2BufferAddress = Mesh.MeshResource->UV2 ? Mesh.MeshResource->UV2->GetDeviceAddress() : 0,
						.NormalsBufferAddress = Mesh.MeshResource->Normals->GetDeviceAddress(),
						.TangentsBufferAddress = Mesh.MeshResource->Tangents->GetDeviceAddress(),
						.VertexCount = Mesh.MeshResource->VertexCount,
						.IndexCount = Mesh.MeshResource->IndicesCount,
						.MaterialId = Mesh.MaterialId,
						.LightmapId = Mesh.LightmapId,
					};

					((GPUSceneMeshCompact*)Ptr)[i] = Compact;
				}
				Context.Device->UnmapBuffer(UploadBuffer);

				Context.CommandBuffer->CopyBuffer(UploadBuffer, Context.Scene->MeshesBuffer, 0, 0, sizeof(GPUSceneMeshCompact) * Context.Scene->Meshes.Size());
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

			// synchronisation
			{
				Buffer* BuffersToSync[] = { Context.Scene->SceneBuffer, Context.Scene->MaterialsBuffer, Context.Scene->MeshesBuffer };

				for (Buffer* Buf : BuffersToSync)
				{
					BufferVulkan* VkBuffer = static_cast<BufferVulkan*>(Buf);

					// TODO: RHI sync
					VkBufferMemoryBarrier VkBarrier;
					VkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
					VkBarrier.pNext = nullptr;
					VkBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					VkBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					VkBarrier.srcQueueFamilyIndex = 0;
					VkBarrier.dstQueueFamilyIndex = 0;
					VkBarrier.buffer = VkBuffer->_Buffer;
					VkBarrier.offset = 0;
					VkBarrier.size = VkBuffer->GetDesc().Size;

					vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
						0, 0, // memory barrier
						1, &VkBarrier, // buffer memory barrier
						0, 0  // image memory barrier
					);
				}
			}

			// TLAS
			{
				AccelerationStructureDesc& Desc = Context.Scene->TLAS->GetDescMut();
				Desc.Instances.clear();

				for (int i = 0; i < (int)Context.Scene->Meshes.Size(); i++)
				{
					AccelerationStructureInstance& Instance = Desc.Instances.emplace_back();
					Instance.Blas = Context.Scene->Meshes.Data()[i].MeshResource->BLAS;
					Instance.Transform = Context.Scene->Meshes.Data()[i].Transform;
				}

				Context.Device->UpdateAccelerationStructureBuffer(Context.Scene->TLAS, Context.CommandBuffer, (u32)Context.Scene->Meshes.Size());
				Context.CommandBuffer->BuildAccelerationStructure(Context.Scene->TLAS, (u32)Context.Scene->Meshes.Size());

				// TLAS sync
				{
					AccelerationStructureVulkan* TLAS = static_cast<AccelerationStructureVulkan*>(Context.Scene->TLAS);
					BufferVulkan* Buffer = static_cast<BufferVulkan*>(TLAS->_Buffer);

					// TODO: RHI sync
					VkBufferMemoryBarrier VkBarrier;
					VkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
					VkBarrier.pNext = nullptr;
					VkBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
					VkBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
					VkBarrier.srcQueueFamilyIndex = 0;
					VkBarrier.dstQueueFamilyIndex = 0;
					VkBarrier.buffer = Buffer->_Buffer;
					VkBarrier.offset = 0;
					VkBarrier.size = Buffer->GetSize();

					vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0,
						0, 0, // memory barrier
						1, &VkBarrier, // buffer memory barrier
						0, 0  // image memory barrier
					);
				}
			}
		});
	}

}
