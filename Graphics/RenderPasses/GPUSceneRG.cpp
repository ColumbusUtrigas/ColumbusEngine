#include "Core/CVar.h"
#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/BufferVulkan.h"

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterBuildTLAS);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterDeformMeshes);
	DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_GPUScene_DeformableMeshes);
	DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_GPUScene_DeformBLASFullBuilds);
	DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_GPUScene_Morphs);
	DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_GPUScene_MorphVertices);
	DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_GPUScene_Bones);
	IMPLEMENT_GPU_PROFILING_COUNTER("Build TLAS", "RenderGraphGPU", GpuCounterBuildTLAS);
	IMPLEMENT_GPU_PROFILING_COUNTER("Deform Meshes", "RenderGraphGPU", GpuCounterDeformMeshes);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Deformable Meshes", "GPUSceneRG", CountingCounter_GPUScene_DeformableMeshes, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Full BLAS Builds", "GPUSceneRG", CountingCounter_GPUScene_DeformBLASFullBuilds, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Morphs", "GPUSceneRG", CountingCounter_GPUScene_Morphs, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Morph Vertices", "GPUSceneRG", CountingCounter_GPUScene_MorphVertices, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Bones", "GPUSceneRG", CountingCounter_GPUScene_Bones, true);

	ConsoleVariable<int> CVar_DeformBLASFullBuildInterval(
		"r.Animation.DeformBLASFullBuildInterval",
		"Update animated BLAS every frame, then force a full build after this many update-only frames. <= 0 disables scheduled full builds.",
		120);
	ConsoleVariable<int> CVar_DeformBLASFullBuildBudget(
		"r.Animation.DeformBLASFullBuildBudget",
		"Maximum animated BLAS full builds to schedule in one frame.",
		1);

	struct DeformMeshParams
	{
		u64 BaseVertices;
		u64 BaseNormals;
		u64 BaseTangents;
		u64 BoneIndices;
		u64 BoneWeights;
		u64 SkinMatrices;
		u64 OutVertices;
		u64 OutPrevVertices;
		u64 OutNormals;
		u64 OutTangents;
		u64 MorphDeltas;
		u64 MorphWeights;
		u32 VertexCount;
		u32 SkinMatrixCount;
		u32 MorphWeightCount;
		u32 _pad;
	};

	struct DeformMeshShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/DeformMesh.csd";

		struct Permutation {};
		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder) {}

		struct Parameters
		{
			ShaderPushConstants<DeformMeshParams> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Constants);
		}
	};

	static bool MeshNeedsDeform(GPUScene& Scene, GPUSceneMesh& Mesh)
	{
		if (Mesh.MeshResource == nullptr || Mesh.Deform == nullptr)
			return false;

		GPUSceneMeshInstance* Instance = Scene.MeshInstances.Get(Mesh.Instance);
		assert(Instance != nullptr);

		const bool bHasSkinning =
			Mesh.MeshResource->BoneIndices != nullptr &&
			Mesh.MeshResource->BoneWeights != nullptr &&
			Instance->SkinMatricesAddress != 0 &&
			Instance->SkinMatrixCount > 0;
		const bool bHasMorphing =
			Mesh.MeshResource->MorphDeltas != nullptr &&
			Instance->MorphWeightsAddress != 0 &&
			Instance->MorphWeightCount > 0;

		return
			Mesh.Deform->Vertices != nullptr &&
			Mesh.Deform->PrevVertices != nullptr &&
			Mesh.Deform->Normals != nullptr &&
			Mesh.Deform->Tangents != nullptr &&
			Mesh.Deform->VertexCount > 0 &&
			(bHasSkinning || bHasMorphing);
	}

	static void AddBufferBarrier(std::vector<VkBufferMemoryBarrier>& Barriers, Buffer* Buffer, VkAccessFlags SrcAccess, VkAccessFlags DstAccess)
	{
		if (Buffer == nullptr)
			return;

		BufferVulkan* VkBuffer = static_cast<BufferVulkan*>(Buffer);

		VkBufferMemoryBarrier Barrier;
		Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		Barrier.pNext = nullptr;
		Barrier.srcAccessMask = SrcAccess;
		Barrier.dstAccessMask = DstAccess;
		Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.buffer = VkBuffer->_Buffer;
		Barrier.offset = 0;
		Barrier.size = Buffer->GetSize();
		Barriers.push_back(Barrier);
	}

	static void DeformMeshesRG(RenderGraph& Graph)
	{
		bool bNeedsPass = false;
		for (u32 i = 0; i < Graph.Scene->Meshes.Size(); i++)
		{
			if (MeshNeedsDeform(*Graph.Scene, Graph.Scene->Meshes[i]))
			{
				bNeedsPass = true;
				break;
			}
		}

		if (!bNeedsPass)
			return;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("Deform Meshes", RenderGraphPassType::Compute, Parameters, Dependencies, [](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterDeformMeshes, Context);

			ComputePipeline* Pipeline = GetComputePipeline<DeformMeshShader>(Context, {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);

			const bool bSupportsRayTracing = Context.Device->SupportsRayTracing();
			std::vector<VkBufferMemoryBarrier> DeformBarriers;
			std::vector<AccelerationStructure*> RebuiltBLASes;
			u32 FullBuildCount = 0;

			for (u32 i = 0; i < Context.Scene->Meshes.Size(); i++)
			{
				GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
				if (!MeshNeedsDeform(*Context.Scene, Mesh))
					continue;

				GPUSceneMeshInstance* Instance = Context.Scene->MeshInstances.Get(Mesh.Instance);
				assert(Instance != nullptr);

				GPUMeshDeform* Deform = Mesh.Deform;
				const bool bHasSkinning =
					Mesh.MeshResource->BoneIndices != nullptr &&
					Mesh.MeshResource->BoneWeights != nullptr &&
					Instance->SkinMatricesAddress != 0 &&
					Instance->SkinMatrixCount > 0;
				const bool bHasMorphing =
					Mesh.MeshResource->MorphDeltas != nullptr &&
					Instance->MorphWeightsAddress != 0 &&
					Instance->MorphWeightCount > 0;

				DeformMeshShader::Parameters ShaderParameters;
				ShaderParameters.Constants.Value = DeformMeshParams{
					.BaseVertices = Mesh.MeshResource->Vertices->GetDeviceAddress(),
					.BaseNormals = Mesh.MeshResource->Normals->GetDeviceAddress(),
					.BaseTangents = Mesh.MeshResource->Tangents->GetDeviceAddress(),
					.BoneIndices = bHasSkinning ? Mesh.MeshResource->BoneIndices->GetDeviceAddress() : 0,
					.BoneWeights = bHasSkinning ? Mesh.MeshResource->BoneWeights->GetDeviceAddress() : 0,
					.SkinMatrices = bHasSkinning ? Instance->SkinMatricesAddress : 0,
					.OutVertices = Deform->Vertices->GetDeviceAddress(),
					.OutPrevVertices = Deform->PrevVertices->GetDeviceAddress(),
					.OutNormals = Deform->Normals->GetDeviceAddress(),
					.OutTangents = Deform->Tangents->GetDeviceAddress(),
					.MorphDeltas = bHasMorphing ? Mesh.MeshResource->MorphDeltas->GetDeviceAddress() : 0,
					.MorphWeights = bHasMorphing ? Instance->MorphWeightsAddress : 0,
					.VertexCount = Deform->VertexCount,
					.SkinMatrixCount = bHasSkinning ? Instance->SkinMatrixCount : 0,
					.MorphWeightCount = bHasMorphing ? Instance->MorphWeightCount : 0,
				};
				Context.BindComputeParameters<DeformMeshShader>(Pipeline, ShaderParameters);

				const u32 GroupCount = (Deform->VertexCount + 63) / 64;
				Context.CommandBuffer->Dispatch(GroupCount, 1, 1);

				const VkAccessFlags DeformDstAccess = bSupportsRayTracing
					? (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR)
					: VK_ACCESS_SHADER_READ_BIT;

				AddBufferBarrier(DeformBarriers, Deform->Vertices, VK_ACCESS_SHADER_WRITE_BIT, DeformDstAccess);
				AddBufferBarrier(DeformBarriers, Deform->PrevVertices, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
				AddBufferBarrier(DeformBarriers, Deform->Normals, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
				AddBufferBarrier(DeformBarriers, Deform->Tangents, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			}

			if (!DeformBarriers.empty())
			{
				const VkPipelineStageFlags DeformDstStages = bSupportsRayTracing
					? (VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)
					: VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

				vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					DeformDstStages,
					0,
					0, nullptr,
					(u32)DeformBarriers.size(), DeformBarriers.data(),
					0, nullptr);
			}

			const int FullBuildInterval = CVar_DeformBLASFullBuildInterval.GetValue();
			const int FullBuildBudget = Math::Max(CVar_DeformBLASFullBuildBudget.GetValue(), 0);
			int FullBuildsRemaining = FullBuildInterval > 0 ? FullBuildBudget : 0;

			for (u32 i = 0; bSupportsRayTracing && i < Context.Scene->Meshes.Size(); i++)
			{
				GPUSceneMesh& Mesh = Context.Scene->Meshes[i];
				if (!MeshNeedsDeform(*Context.Scene, Mesh) || Mesh.Deform->BLAS == nullptr)
					continue;

				GPUMeshDeform* Deform = Mesh.Deform;
				Deform->FramesSinceFullBLASBuild++;

				const bool bDoFullBuild =
					FullBuildsRemaining > 0 &&
					Deform->FramesSinceFullBLASBuild >= (u32)FullBuildInterval;
				const AccelerationStructureBuildMode BuildMode = bDoFullBuild
					? AccelerationStructureBuildMode::Build
					: AccelerationStructureBuildMode::Update;

				Context.CommandBuffer->BuildAccelerationStructure(Deform->BLAS, Mesh.MeshResource->IndicesCount / 3, BuildMode);
				if (bDoFullBuild)
				{
					Deform->FramesSinceFullBLASBuild = 0;
					FullBuildsRemaining--;
					FullBuildCount++;
				}

				RebuiltBLASes.push_back(Mesh.Deform->BLAS);
			}
			AddProfilingCount(CountingCounter_GPUScene_DeformBLASFullBuilds, FullBuildCount);

			std::vector<VkBufferMemoryBarrier> BLASBarriers;
			for (AccelerationStructure* BLAS : RebuiltBLASes)
			{
				AccelerationStructureVulkan* BLASVk = static_cast<AccelerationStructureVulkan*>(BLAS);
				AddBufferBarrier(BLASBarriers, BLASVk->_Buffer, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
			}

			if (!BLASBarriers.empty())
			{
				vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf,
					VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
					VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
					0,
					0, nullptr,
					(u32)BLASBarriers.size(), BLASBarriers.data(),
					0, nullptr);
			}
		});
	}

	void UploadGPUSceneRG(RenderGraph& Graph, const RenderView& View, DeferredRenderContext* DeferredContext)
	{
		Graph.Scene->Update();
		if (DeferredContext)
		{
			PrepareShadows(Graph, View, DeferredContext->Shadows);
		}

		const auto GetTextureDenseIndex = [&Graph](TStableSparseArray<Texture2*>::Handle Handle) -> int
		{
			return Graph.Scene->Textures.GetDenseIndex(Handle);
		};

		const auto GetMaterialDenseIndex = [&Graph](TStableSparseArray<Material>::Handle Handle) -> int
		{
			return Graph.Scene->Materials.GetDenseIndex(Handle);
		};

		const auto GetMaterialFlags = [&Graph](const Material& Mat) -> int
		{
			Texture2** ppNormalTexture = Graph.Scene->Textures.Get(Mat.NormalId);
			if (ppNormalTexture != nullptr && *ppNormalTexture != nullptr)
			{
				if ((*ppNormalTexture)->GetDesc().Format == TextureFormat::BC5)
				{
					return GPUMATERIAL_FLAG_NORMAL_RG;
				}
			}

			return 0;
		};

		// Scene
		{
			GPUSceneCompact Compact = Graph.Scene->CreateCompact(View);
			Graph.Device->UploadRing.UploadBuffer(&Compact, sizeof(Compact), 0, Graph.Scene->SceneBuffer);
		}

		// Lights
		if (Graph.Scene->Lights.Size() > 0)
		{
			u32 NumLights = (u32)Graph.Scene->Lights.Size();
			void* Ptr = Graph.Device->UploadRing.UploadBufferMap(NumLights * sizeof(GPULightCompact), 0, Graph.Scene->LightsBuffer);

			for (u32 i = 0; i < NumLights; i++)
			{
				((GPULightCompact*)Ptr)[i] = DeferredContext ? DeferredContext->Shadows.Frame.Lights[i] : CreateCompact(Graph.Scene->Lights[i]);
			}
		}

		// Meshes
		if (Graph.Scene->Meshes.Size() > 0)
		{
			const u32 NumMeshes = (u32)Graph.Scene->Meshes.Size();
			const u32 NumMeshInstances = (u32)Graph.Scene->MeshInstances.Size();
			std::vector<u32> SkinMatrixOffsets(NumMeshInstances, (u32)-1);
			u32 SkinMatrixTotal = 0;
			std::vector<u32> MorphWeightOffsets(NumMeshInstances, (u32)-1);
			u32 MorphWeightTotal = 0;

			for (u32 i = 0; i < NumMeshInstances; i++)
			{
				GPUSceneMeshInstance& Instance = Graph.Scene->MeshInstances[i];
				Instance.SkinMatricesAddress = 0;
				Instance.SkinMatrixCount = 0;
				Instance.MorphWeightsAddress = 0;
				Instance.MorphWeightCount = 0;

				if (!Instance.SkinMatrices.empty())
				{
					const u32 SkinMatrixCount = (u32)Instance.SkinMatrices.size();
					assert(SkinMatrixTotal + SkinMatrixCount <= GPUScene::MaxSkinMatrices);
					SkinMatrixOffsets[i] = SkinMatrixTotal;
					SkinMatrixTotal += SkinMatrixCount;
				}

				if (!Instance.MorphWeights.empty())
				{
					const u32 MorphWeightCount = (u32)Instance.MorphWeights.size();
					assert(MorphWeightTotal + MorphWeightCount <= GPUScene::MaxMorphWeights);
					MorphWeightOffsets[i] = MorphWeightTotal;
					MorphWeightTotal += MorphWeightCount;
				}
			}
			AddProfilingCount(CountingCounter_GPUScene_Bones, SkinMatrixTotal);
			AddProfilingCount(CountingCounter_GPUScene_Morphs, MorphWeightTotal);

			if (SkinMatrixTotal > 0)
			{
				Matrix* SkinMatricesPtr = (Matrix*)Graph.Device->UploadRing.UploadBufferMap(SkinMatrixTotal * sizeof(Matrix), 0, Graph.Scene->SkinMatricesBuffer);
				for (u32 i = 0; i < NumMeshInstances; i++)
				{
					if (SkinMatrixOffsets[i] == (u32)-1)
						continue;

					GPUSceneMeshInstance& Instance = Graph.Scene->MeshInstances[i];
					Instance.SkinMatricesAddress = Graph.Scene->SkinMatricesBuffer->GetDeviceAddress() + (u64)SkinMatrixOffsets[i] * sizeof(Matrix);
					Instance.SkinMatrixCount = (u32)Instance.SkinMatrices.size();

					Matrix* Dst = SkinMatricesPtr + SkinMatrixOffsets[i];
					for (u32 MatrixIndex = 0; MatrixIndex < Instance.SkinMatrixCount; MatrixIndex++)
						Dst[MatrixIndex] = Instance.SkinMatrices[MatrixIndex];
				}
			}

			if (MorphWeightTotal > 0)
			{
				float* MorphWeightsPtr = (float*)Graph.Device->UploadRing.UploadBufferMap(MorphWeightTotal * sizeof(float), 0, Graph.Scene->MorphWeightsBuffer);
				for (u32 i = 0; i < NumMeshInstances; i++)
				{
					if (MorphWeightOffsets[i] == (u32)-1)
						continue;

					GPUSceneMeshInstance& Instance = Graph.Scene->MeshInstances[i];
					Instance.MorphWeightsAddress = Graph.Scene->MorphWeightsBuffer->GetDeviceAddress() + (u64)MorphWeightOffsets[i] * sizeof(float);
					Instance.MorphWeightCount = (u32)Instance.MorphWeights.size();

					float* Dst = MorphWeightsPtr + MorphWeightOffsets[i];
					for (u32 WeightIndex = 0; WeightIndex < Instance.MorphWeightCount; WeightIndex++)
						Dst[WeightIndex] = Instance.MorphWeights[WeightIndex];
				}
			}

			void* Ptr = Graph.Device->UploadRing.UploadBufferMap(NumMeshes * sizeof(GPUSceneMeshCompact), 0, Graph.Scene->MeshesBuffer);

			for (u32 i = 0; i < NumMeshes; i++)
			{
				GPUSceneMesh& Mesh = Graph.Scene->Meshes[i];
				GPUSceneMeshInstance* Instance = Graph.Scene->MeshInstances.Get(Mesh.Instance);
				assert(Instance != nullptr);

				Matrix NormalMatrix = Mesh.Transform.GetInverted().GetTransposed();
				const bool bHasUploadedSkinning =
					Mesh.MeshResource->BoneIndices != nullptr &&
					Mesh.MeshResource->BoneWeights != nullptr &&
					Instance->SkinMatricesAddress != 0 &&
					Instance->SkinMatrixCount > 0;
				const bool bHasUploadedMorphing =
					Mesh.MeshResource->MorphDeltas != nullptr &&
					Mesh.MeshResource->MorphTargetCount > 0 &&
					Instance->MorphWeightsAddress != 0 &&
					Instance->MorphWeightCount > 0;
				if (bHasUploadedMorphing)
					assert(Instance->MorphWeightCount == Mesh.MeshResource->MorphTargetCount);

				const GPUMeshDeform* Deform = Mesh.Deform;
				const bool bUseDeform =
					(bHasUploadedSkinning || bHasUploadedMorphing) &&
					Deform != nullptr &&
					Deform->Vertices != nullptr &&
					Deform->PrevVertices != nullptr &&
					Deform->Normals != nullptr &&
					Deform->Tangents != nullptr;
				if (bUseDeform)
					AddProfilingCount(CountingCounter_GPUScene_DeformableMeshes, 1);
				if (bUseDeform && bHasUploadedMorphing)
					AddProfilingCount(CountingCounter_GPUScene_MorphVertices, (u64)Mesh.MeshResource->VertexCount * Instance->MorphWeightCount);

				GPUSceneMeshCompact Compact{
					.Transform = Mesh.Transform,
					.PrevTransform = Mesh.PrevTransform,
					.NormalMatrix = NormalMatrix,
					.VertexBufferAddress = bUseDeform ? Deform->Vertices->GetDeviceAddress() : Mesh.MeshResource->Vertices->GetDeviceAddress(),
					.IndexBufferAddress = Mesh.MeshResource->Indices->GetDeviceAddress(),
					.Uv1BufferAddress = Mesh.MeshResource->UV1->GetDeviceAddress(),
					.Uv2BufferAddress = Mesh.MeshResource->UV2 ? Mesh.MeshResource->UV2->GetDeviceAddress() : 0,
					.NormalsBufferAddress = bUseDeform ? Deform->Normals->GetDeviceAddress() : Mesh.MeshResource->Normals->GetDeviceAddress(),
					.TangentsBufferAddress = bUseDeform ? Deform->Tangents->GetDeviceAddress() : Mesh.MeshResource->Tangents->GetDeviceAddress(),
					.VertexCount = Mesh.MeshResource->VertexCount,
					.IndexCount = Mesh.MeshResource->IndicesCount,
					.MaterialId = GetMaterialDenseIndex(Mesh.MaterialId),
					.LightmapId = Mesh.LightmapId,
					.PrevVertexBufferAddress = bUseDeform ? Deform->PrevVertices->GetDeviceAddress() : 0,
				};

				((GPUSceneMeshCompact*)Ptr)[i] = Compact;
			}
		}

		// Materials
		if (Graph.Scene->Materials.Size() > 0)
		{
			u32 NumMaterials = (u32)Graph.Scene->Materials.Size();

			void* Ptr = Graph.Device->UploadRing.UploadBufferMap(NumMaterials * sizeof(GPUMaterialCompact), 0, Graph.Scene->MaterialsBuffer);

			for (u32 i = 0; i < NumMaterials; i++)
			{
				Material& Mat = Graph.Scene->Materials[i];
				GPUMaterialCompact Compact{
					.AlbedoFactor = Mat.AlbedoFactor,
					.EmissiveFactor = Mat.EmissiveFactor,
					.AlbedoId = GetTextureDenseIndex(Mat.AlbedoId),
					.NormalId = GetTextureDenseIndex(Mat.NormalId),
					.OrmID = GetTextureDenseIndex(Mat.OrmId),
					.EmissiveId = GetTextureDenseIndex(Mat.EmissiveId),
					.Roughness = Mat.Roughness,
					.Metallic = Mat.Metallic,
					.Flags = GetMaterialFlags(Mat),
					.ShadingMode = (int)Mat.ShadingMode,
					.AlphaCutoff = Mat.AlphaCutoff,
				};
				((GPUMaterialCompact*)Ptr)[i] = Compact;
			}
		}

		// Particles
		{
			u32 NumEmitters = (u32)Graph.Scene->Particles.Size();
			for (u32 i = 0; i < NumEmitters; i++)
			{
				const GPUSceneParticles& Particles = Graph.Scene->Particles[i];

				u32 NumParticles = (u32)Particles.ParticleInstance->Particles.Count;

				if (NumParticles == 0)
					continue;

				GPUParticleCompact* Ptr = (GPUParticleCompact*)Graph.Device->UploadRing.UploadBufferMap(NumParticles * sizeof(GPUParticleCompact), 0, Particles.DataBuffer);

				// pack particles
				for (u32 j = 0; j < NumParticles; j++)
				{
					auto& Container = Particles.ParticleInstance->Particles;

					const float NormalisedAge = Math::Clamp(Container.Ages[j] / Math::Max(Container.Lifetimes[j], 0.001f), 0.0f, 1.0f);

					GPUParticleCompact Particle{
						.Position_Rotation = Vector4(Container.Positions[j], Container.Rotations[j]),
						.Size_Frame = Vector4(Container.Sizes[j], NormalisedAge),
						.Colour = Container.Colors[j]
					};

					Ptr[j] = Particle;
				}
			}
		}

		Graph.Device->UploadRing.FlushUploads();

		DeformMeshesRG(Graph);

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Graph.AddPass("Build TLAS", RenderGraphPassType::Compute, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterBuildTLAS, Context);

			// TLAS
			{
				AccelerationStructureDesc& Desc = Context.Scene->TLAS->GetDescMut();
				Desc.Instances.clear();

				for (int i = 0; i < (int)Context.Scene->Meshes.Size(); i++)
				{
					const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];

					AccelerationStructureInstance& Instance = Desc.Instances.emplace_back();
					Instance.Blas = MeshNeedsDeform(*Context.Scene, Context.Scene->Meshes[i]) && Mesh.Deform->BLAS != nullptr
						? Mesh.Deform->BLAS
						: Mesh.MeshResource->BLAS;
					Instance.Transform = Mesh.Transform;
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
