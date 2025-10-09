#include "RenderPasses.h"

namespace Columbus
{

	void UploadGPUSceneRG(RenderGraph& Graph, const RenderView& View)
	{
		Graph.Scene->Update();

		// Scene
		{
			GPUSceneCompact Compact = Graph.Scene->CreateCompact(View);
			Graph.Device->UploadRing.UploadBuffer(&Compact, sizeof(Compact), 0, Graph.Scene->SceneBuffer);
		}

		// Lights
		if (Graph.Scene->Lights.Size() > 0)
		{
			Graph.Device->UploadRing.UploadBuffer(Graph.Scene->Lights.Data(), Graph.Scene->Lights.Size() * sizeof(GPULight), 0, Graph.Scene->LightsBuffer);
		}

		// Meshes
		if (Graph.Scene->Meshes.Size() > 0)
		{
			u32 NumMeshes = Graph.Scene->Meshes.Size();

			void* Ptr = Graph.Device->UploadRing.UploadBufferMap(NumMeshes * sizeof(GPUSceneMeshCompact), 0, Graph.Scene->MeshesBuffer);

			for (u32 i = 0; i < NumMeshes; i++)
			{
				const GPUSceneMesh& Mesh = Graph.Scene->Meshes[i];
				GPUSceneMeshCompact Compact{
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
		}

		// Materials
		if (Graph.Scene->Materials.Size() > 0)
		{
			u32 NumMaterials = Graph.Scene->Materials.Size();

			void* Ptr = Graph.Device->UploadRing.UploadBufferMap(NumMaterials * sizeof(GPUMaterialCompact), 0, Graph.Scene->MaterialsBuffer);

			for (u32 i = 0; i < NumMaterials; i++)
			{
				Material& Mat = Graph.Scene->Materials[i];
				GPUMaterialCompact Compact{
					.AlbedoFactor = Mat.AlbedoFactor,
					.EmissiveFactor = Mat.EmissiveFactor,
					.AlbedoId = (int)Mat.AlbedoId.index,
					.NormalId = (int)Mat.NormalId.index,
					.OrmID = (int)Mat.OrmId.index,
					.EmissiveId = (int)Mat.EmissiveId.index,
					.Roughness = Mat.Roughness,
					.Metallic = Mat.Metallic,
				};
				((GPUMaterialCompact*)Ptr)[i] = Compact;
			}
		}

		// Particles
		{
			u32 NumEmitters = Graph.Scene->Particles.Size();
			for (u32 i = 0; i < NumEmitters; i++)
			{
				const GPUSceneParticles& Particles = Graph.Scene->Particles[i];

				u32 NumParticles = Particles.ParticleInstance->Particles.Count;

				if (NumParticles == 0)
					continue;

				GPUParticleCompact* Ptr = (GPUParticleCompact*)Graph.Device->UploadRing.UploadBufferMap(NumParticles * sizeof(GPUParticleCompact), 0, Particles.DataBuffer);

				// pack particles
				for (u32 j = 0; j < NumParticles; j++)
				{
					auto& Container = Particles.ParticleInstance->Particles;

					GPUParticleCompact Particle{
						.Position_Rotation = Vector4(Container.Positions[j], Container.Rotations[j]),
						.Size_Frame = Vector4(Container.Sizes[j], Container.Frames[j]),
						.Colour = Container.Colors[j]
					};

					Ptr[j] = Particle;
				}
			}
		}

		Graph.Device->UploadRing.FlushUploads();

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("Build TLAS", RenderGraphPassType::Compute, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
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
