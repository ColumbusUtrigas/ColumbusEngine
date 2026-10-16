#include "ShadowMaps.h"
#include "Math/Frustum.h"
#include "Math/Geometry.h"
#include "Math/MathUtil.h"
#include "Core/CVar.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Material.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "System/Log.h"
#include <cstdio>
#include <unordered_set>

namespace Columbus
{
	IMPLEMENT_CPU_PROFILING_COUNTER("Shadow preparation", "Render", CpuCounterShadowPreparation);
	IMPLEMENT_CPU_PROFILING_COUNTER("Shadow caster culling", "Render", CpuCounterShadowCulling);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Shadow caster draws possible", "Shadows", CountShadowDrawsPossible, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Shadow caster range rejects", "Shadows", CountShadowRangeRejects, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Shadow caster frustum rejects", "Shadows", CountShadowFrustumRejects, true);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Shadow caster draws", "Shadows", CountShadowDraws, true);
	IMPLEMENT_GPU_PROFILING_COUNTER("Shadow maps", "RenderGraphGPU", GpuCounterShadowMaps);
	IMPLEMENT_MEMORY_PROFILING_COUNTER("Shadow atlas set", "Shadows", MemoryCounterShadowAtlases);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Shadow atlas pages", "Shadows", CountShadowAtlasPages, false);
	IMPLEMENT_COUNTING_PROFILING_COUNTER("Raster shadow views", "Shadows", CountShadowViews, true);

	ConsoleVariable<int> CVar_ShadowMode(
		"r.Shadows.Mode",
		"Shadow-enabled lights: 0=per-light default, 1=force raster, 2=force ray traced (raster fallback without RT support)",
		0
	);

	ConsoleVariable<bool> CVar_ShadowCasterCulling(
		"r.Shadows.CasterCulling",
		"Cull shadow casters against light ranges and padded shadow-view frustums",
		true
	);

	ConsoleVariable<int> CVar_ShadowAtlasSize(
		"r.Shadows.AtlasSize",
		"Size of newly allocated square shadow atlas pages (power of two, 256..4096)",
		4096
	);

	u64 ShadowLightKey(const GPUScene& Scene, u32 DenseIndex)
	{
		const auto Handle = Scene.Lights.GetHandleAtDenseIndex(DenseIndex);
		return (u64(Handle.generation) << 32) | Handle.index;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Persistent atlas resources and allocations

	void ShadowRenderState::Destroy(SPtr<DeviceVulkan> Device)
	{
		if (!Pages.empty())
		{
			Device->QueueWaitIdle();
		}

		for (auto& Page : Pages)
		{
			RemoveProfilingMemory(MemoryCounterShadowAtlases, Page.Texture->GetSize());
			RemoveProfilingCount(CountShadowAtlasPages, 1);
			Device->DestroyTexture(Page.Texture.get());
		}

		Pages.clear();
		Allocations.clear();

		for (auto& [Key, History] : RTHistories)
		{
			History.Destroy(Device);
		}

		RTHistories.clear();
		Frame         = {};
		AtlasSize     = 0;
		SceneIdentity = nullptr;
	}

	static void ReleaseAllocation(ShadowRenderState& State, const ShadowRenderState::LightAllocation& Allocation)
	{
		for (const auto& Tile : Allocation.Tiles)
		{
			const bool Freed = State.Pages[Tile.AtlasIndex].Allocator.Free(Tile.Block);
			COLUMBUS_ASSERT(Freed);
		}
	}

	static std::optional<ShadowTile> AllocateTile(RenderGraph& Graph, ShadowRenderState& State, u32 Resolution)
	{
		for (u32 Page = 0; Page < State.Pages.size(); ++Page)
		{
			if (auto Block = State.Pages[Page].Allocator.Allocate(Resolution))
			{
				return ShadowTile{ Page, *Block };
			}
		}

		// This is the binding capacity, not a memory budget or a resolution downgrade policy.
		if (State.Pages.size() >= ShadowAtlasDescriptorCount)
		{
			Log::Error("Shadow atlas descriptor capacity exceeded");
			COLUMBUS_ASSERT_MESSAGE(false, "Shadow atlas descriptor capacity exceeded");
			return {};
		}

		TextureDesc2 Desc;
		Desc.Width     = State.AtlasSize;
		Desc.Height    = State.AtlasSize;
		Desc.Format    = TextureFormat::Depth32F;
		Desc.Usage     = TextureUsage::RenderTargetDepth | TextureUsage::Sampled;

		Desc.MinFilter = TextureFilter2::Nearest;
		Desc.MagFilter = TextureFilter2::Nearest;
		Desc.MipFilter = TextureFilter2::Nearest;
		Desc.AddressU  = TextureAddressMode::ClampToEdge;
		Desc.AddressV  = TextureAddressMode::ClampToEdge;

		ShadowRenderState::AtlasPage Page(State.AtlasSize);
		const auto Block = Page.Allocator.Allocate(Resolution);
		COLUMBUS_ASSERT(Block.has_value());

		if (!Block)
		{
			Log::Error("Cannot allocate a %u shadow tile in a %u atlas page", Resolution, State.AtlasSize);
			return {};
		}

		Page.Texture = SPtr<Texture2>(Graph.Device->CreateTexture(Desc));

		char Name[64];
		snprintf(Name, sizeof(Name), "ShadowAtlas %u", (u32)State.Pages.size());
		Graph.Device->SetDebugName(Page.Texture.get(), Name);

		AddProfilingMemory(MemoryCounterShadowAtlases, Page.Texture->GetSize());
		AddProfilingCount(CountShadowAtlasPages, 1);
		State.Pages.push_back(std::move(Page));

		return ShadowTile{ (u32)State.Pages.size() - 1, *Block };
	}

	static bool EnsureLightAllocation(RenderGraph& Graph, ShadowRenderState& State, u64 Key, const GPULightCompact& Light)
	{
		if (State.Allocations.contains(Key))
		{
			return true;
		}

		ShadowRenderState::LightAllocation Allocation{ Light.ShadowResolution, {} };
		Allocation.Tiles.reserve(Light.ShadowViewCount);

		for (u32 View = 0; View < Light.ShadowViewCount; ++View)
		{
			const auto Tile = AllocateTile(Graph, State, Light.ShadowResolution);

			if (!Tile)
			{
				ReleaseAllocation(State, Allocation);
				return false;
			}

			Allocation.Tiles.push_back(*Tile);
		}

		State.Allocations.emplace(Key, std::move(Allocation));
		return true;
	}

	static void ReleaseObsoleteAllocations(ShadowRenderState& State, const std::unordered_set<u64>& RasterKeys)
	{
		for (auto It = State.Allocations.begin(); It != State.Allocations.end();)
		{
			const u64 Key = It->first;
			const auto& Allocation = It->second;
			const bool IsRasterLight = RasterKeys.contains(Key);

			if (!IsRasterLight)
			{
				ReleaseAllocation(State, Allocation);
				It = State.Allocations.erase(It);
			}
			else
			{
				++It;
			}
		}
	}

	static void ReleaseChangedAllocation(ShadowRenderState& State, u64 Key, const GPULightCompact& Light)
	{
		auto Existing = State.Allocations.find(Key);

		if (Existing == State.Allocations.end())
		{
			return;
		}

		const auto& Allocation = Existing->second;

		const bool IsRasterLight     = Light.ShadowViewCount != 0;
		const bool ResolutionChanged = Allocation.Resolution != Light.ShadowResolution;
		const bool ViewCountChanged  = Allocation.Tiles.size() != Light.ShadowViewCount;

		if (!IsRasterLight || ResolutionChanged || ViewCountChanged)
		{
			ReleaseAllocation(State, Allocation);
			State.Allocations.erase(Existing);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Shadow view preparation

	static Vector3 StableUp(const Vector3& Direction)
	{
		return fabsf(Direction.Y) < 0.999f ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
	}

	static Matrix PerspectiveView(const Vector3& Position, const Vector3& Direction, const Vector3& Up, float Fov, float Aspect, float NearPlane, float FarPlane)
	{
		Matrix View;
		Matrix Projection;
		View.LookAt(Position, Position + Direction, Up);
		Projection.Perspective(Fov, Aspect, NearPlane, FarPlane);
		return Projection * View;
	}

	static void SetAtlasPlacement(ShadowDrawView& View, u32 AtlasSize)
	{
		const auto& Block = View.Tile.Block;

		const float Scale = float(Block.Size - 2 * ShadowTilePadding) / float(AtlasSize);
		const float X     = float(Block.X + ShadowTilePadding) / float(AtlasSize);
		const float Y     = float(Block.Y + ShadowTilePadding) / float(AtlasSize);

		View.GPU.AtlasScaleBias = Vector4(Scale, Scale, X, Y);
		View.GPU.SampleBounds   = Vector4((Block.X + 0.5f) / AtlasSize, (Block.Y + 0.5f) / AtlasSize, (Block.X + Block.Size - 0.5f) / AtlasSize, (Block.Y + Block.Size - 0.5f) / AtlasSize);
		View.GPU.AtlasIndex     = View.Tile.AtlasIndex;
	}

	struct CascadeBounds
	{
		Vector3 Center = Vector3(0);
		float   Radius = 0;
	};

	static CascadeBounds CalculateCascadeBounds(const Matrix& InverseProjection, float Start, float End)
	{
		std::array<Vector3, 8> Corners;
		CascadeBounds Bounds;
		u32 Index = 0;

		for (float Depth : { Start, End })
		{
			for (float Y : { -1.0f, 1.0f })
			{
				for (float X : { -1.0f, 1.0f })
				{
					const Vector4 Ray    = InverseProjection * Vector4(X, Y, 1, 1);
					const Vector3 Corner = Ray.XYZ() * (Depth / -Ray.Z);

					Corners[Index++] = Corner;
					Bounds.Center += Corner / 8.0f;
				}
			}
		}

		for (const Vector3& Corner : Corners)
		{
			Bounds.Radius = std::max(Bounds.Radius, (Corner - Bounds.Center).Length());
		}

		return Bounds;
	}

	struct DirectionalShadowBasis
	{
		Matrix  InverseView;
		Matrix  LightRotation;
		Matrix  InverseRotation;
		Vector3 TowardLight;
	};

	static void BuildDirectionalProjection(ShadowDrawView& Draw, const CascadeBounds& Bounds, const DirectionalShadowBasis& Basis, const GPULightCompact& Light)
	{
		float Radius = Bounds.Radius;

		// Radius depends only on the frustum slice, not camera orientation or location.
		Radius = ceilf(Radius * 16.0f) / 16.0f;

		const float Usable = float(Draw.Tile.Block.Size - 2 * ShadowTilePadding);
		Radius *= Usable / (Usable - 2.0f); // Reserve one texel for snapping on both edges.

		const float Texel = 2.0f * Radius / Usable;

		Vector3 Center   = (Basis.InverseView * Vector4(Bounds.Center, 1)).XYZ();
		Vector3 CenterLS = (Basis.LightRotation * Vector4(Center, 1)).XYZ();

		CenterLS.X = roundf(CenterLS.X / Texel) * Texel;
		CenterLS.Y = roundf(CenterLS.Y / Texel) * Texel;

		Center = (Basis.InverseRotation * Vector4(CenterLS, 1)).XYZ();

		// Conservative depth extrusion includes off-camera casters towards the light.
		// It intentionally does not use camera-visible geometry to determine caster coverage.
		const float Extrusion = Light.ShadowDistance;
		const Vector3 Eye = Center + Basis.TowardLight * (Radius + Extrusion);

		Matrix View;
		Matrix Projection;

		View.LookAt(Eye, Center, StableUp(Basis.TowardLight));
		Projection.Ortho(-Radius, Radius, -Radius, Radius, 0.0f, 2 * Radius + 2 * Extrusion);

		Draw.GPU.WorldToClip = Projection * View;
	}

	static void BuildDirectionalViews(const RenderView& RenderView, const GPULightCompact& Light, std::vector<ShadowDrawView>& Views, size_t First)
	{
		const Camera& Camera = RenderView.CameraCurUnjittered;

		const Matrix InverseProjection = Camera.GetProjectionMatrix().GetInverted();
		const Matrix InverseView       = Camera.GetViewMatrix().GetInverted();

		const Vector4 NearH = InverseProjection * Vector4(0, 0, 1, 1);
		const Vector4 FarH  = InverseProjection * Vector4(0, 0, 0, 1);

		const float NearDepth = std::max(0.001f, -NearH.Z / NearH.W);
		const float CameraFar = fabsf(FarH.W) > 1e-8f ? -FarH.Z / FarH.W : Light.ShadowDistance;
		const float FarDepth  = std::max(NearDepth + 0.01f, std::min(Light.ShadowDistance, CameraFar));

		std::array<float, 5> Splits{};
		Splits[0] = NearDepth;

		for (u32 i = 1; i <= Light.ShadowViewCount; ++i)
		{
			const float T           = float(i) / Light.ShadowViewCount;
			const float Linear      = NearDepth + (FarDepth - NearDepth) * T;
			const float Logarithmic = NearDepth * powf(FarDepth / NearDepth, T);
			Splits[i]               = Linear + (Logarithmic - Linear) * Light.ShadowSplitLambda;
		}

		DirectionalShadowBasis Basis;
		Basis.InverseView = InverseView;
		Basis.TowardLight = Light.Direction.XYZ().Normalized();
		Basis.LightRotation.LookAt(Vector3(0), -Basis.TowardLight, StableUp(Basis.TowardLight));
		Basis.InverseRotation = Basis.LightRotation.GetTransposed();

		for (u32 Cascade = 0; Cascade < Light.ShadowViewCount; ++Cascade)
		{
			auto& Draw = Views[First + Cascade];

			float Start = NearDepth;

			if (Cascade > 0)
			{
				Start = Splits[Cascade] - (Splits[Cascade] - Splits[Cascade - 1]) * Light.ShadowCascadeBlend;
			}

			const float End = Splits[Cascade + 1];

			const CascadeBounds Bounds = CalculateCascadeBounds(InverseProjection, Start, End);

			BuildDirectionalProjection(Draw, Bounds, Basis, Light);

			Draw.GPU.Cascade = Vector4(Splits[Cascade], End, End - (End - Splits[Cascade]) * Light.ShadowCascadeBlend, 0);
		}
	}

	static Matrix BuildLocalLightView(const GPULightCompact& Light, u32 ViewIndex, float NearPlane)
	{
		static const Vector3 PointDirections[6] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
		static const Vector3 PointUps[6]        = { { 0, 1, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }, { 0, 1, 0 }, { 0, 1, 0 } };

		const Vector3 TowardLight    = Light.Direction.XYZ().Normalized();
		const bool    UsesLightRange = Light.Type == LightType::Spot || Light.Type == LightType::Point;
		const float   FarPlane       = std::max(NearPlane + 0.01f, UsesLightRange ? Light.Range : Light.ShadowDistance);

		Vector3 Direction = -TowardLight;
		Vector3 Up        = StableUp(Direction);
		float   Aspect    = 1;
		float   Fov       = Light.ShadowConeAngle;

		if (Light.Type == LightType::Point)
		{
			Direction = PointDirections[ViewIndex];
			Up        = PointUps[ViewIndex];
			Fov       = 90;
		}
		else if (Light.Type == LightType::Spot)
		{
			Fov = std::clamp(2 * acosf(std::clamp(Light.SizeOrSpotAngles.Y, -1.0f, 1.0f)) * 180.0f / float(Math::Pi), 1.0f, 175.0f);
		}
		else
		{
			const Vector3 BaseX = Vector3::Cross(StableUp(TowardLight), TowardLight).Normalized();
			const Vector3 BaseY = Vector3::Cross(TowardLight, BaseX);

			Up        = -sinf(Light.Direction.W) * BaseX + cosf(Light.Direction.W) * BaseY;
			Direction = ViewIndex == 0 ? TowardLight : -TowardLight; // Planar emitter normal points into the emitting half space.
			Aspect    = std::clamp(Light.SizeOrSpotAngles.X / std::max(Light.SizeOrSpotAngles.Y, 0.001f), 0.01f, 100.0f);

			// Shadow cone applies to the wider frustum axis.
			if (Aspect > 1)
			{
				Fov = 2 * atanf(tanf(Fov * float(Math::Pi) / 360) / Aspect) * 180 / float(Math::Pi);
			}
		}

		return PerspectiveView(Light.Position.XYZ(), Direction, Up, Fov, Aspect, NearPlane, FarPlane);
	}

	static void BuildLightViews(RenderGraph& Graph, const RenderView& View, ShadowRenderState& State, u32 LightIndex)
	{
		auto& Frame = State.Frame;
		auto& Light = Frame.Lights[LightIndex];
		const u64 Key = ShadowLightKey(*Graph.Scene, LightIndex);

		if (!EnsureLightAllocation(Graph, State, Key, Light))
		{
			Light.Flags = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::Shadow));
			Light.ShadowViewCount = 0;
			return;
		}

		Light.FirstShadowView = (u32)Frame.Views.size();

		const auto& Allocation = State.Allocations.find(Key)->second;

		const float NearPlane = std::max(Graph.Scene->Lights[LightIndex].ShadowNearPlane, 0.001f);

		for (u32 ViewIndex = 0; ViewIndex < Light.ShadowViewCount; ++ViewIndex)
		{
			ShadowDrawView Draw{};
			Draw.Tile      = Allocation.Tiles[ViewIndex];
			Draw.SlopeBias = Light.ShadowSlopeBias;
			SetAtlasPlacement(Draw, State.AtlasSize);

			if (Light.Type != LightType::Directional)
			{
				Draw.GPU.WorldToClip = BuildLocalLightView(Light, ViewIndex, NearPlane);
			}

			Frame.Views.push_back(std::move(Draw));
		}

		if (Light.Type == LightType::Directional)
		{
			BuildDirectionalViews(View, Light, Frame.Views, Light.FirstShadowView);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Shadow caster preparation

	static Box CasterWorldBounds(GPUScene& Scene, const GPUSceneMesh& Mesh)
	{
		const auto& Resource = *Mesh.MeshResource;

		if (!Resource.HasBounds)
		{
			return Box(Vector3(1), Vector3(-1));
		}

		Box Local = Resource.Bounds;

		if (Mesh.Deform)
		{
			const auto* Instance = Scene.MeshInstances.Get(Mesh.Instance);

			if (!Instance)
			{
				return Box(Vector3(1), Vector3(-1));
			}

			const std::span<const float> Weights = Resource.MorphDeltas ? std::span<const float>(Instance->MorphWeights) : std::span<const float>();
			const std::span<const Matrix> Skins = Resource.BoneIndices && Resource.BoneWeights ? std::span<const Matrix>(Instance->SkinMatrices) : std::span<const Matrix>();

			Local = Local.CalcDeformedBox(Resource.MorphPositionBounds, Weights, Skins);
		}

		return Local.CalcTransformedBox(Mesh.Transform);
	}

	struct ShadowCaster
	{
		u32 ObjectId;
		Box Bounds;
	};

	static std::vector<ShadowCaster> GatherShadowCasters(GPUScene& Scene, bool Cull)
	{
		std::vector<ShadowCaster> Casters;
		Casters.reserve(Scene.Meshes.Size());

		for (u32 i = 0; i < Scene.Meshes.Size(); ++i)
		{
			const auto&     Mesh     = Scene.Meshes[i];
			const Material* Material = Scene.Materials.Get(Mesh.MaterialId);

			const bool HasGeometry   = Mesh.MeshResource && Mesh.MeshResource->IndicesCount;
			const bool IsTransparent = Material && Material->ShadingMode == MaterialShadingMode::Transparent;
			const bool IsRefractive  = Material && Material->ShadingMode == MaterialShadingMode::Refractive;

			if (!HasGeometry || IsTransparent || IsRefractive)
			{
				continue;
			}

			Casters.push_back({ i, Cull ? CasterWorldBounds(Scene, Mesh) : Box() });
		}

		return Casters;
	}

	static void GatherLightCandidates(const GPULightCompact& Light, const std::vector<ShadowCaster>& Casters, bool Cull, std::vector<size_t>& Candidates)
	{
		Candidates.clear();

		// A caster on a light-to-receiver segment cannot lie outside the local
		// influence sphere. Account for the shader's receiver normal offset.
		const bool UsesLightRange = Light.Type == LightType::Point || Light.Type == LightType::Spot;
		const bool CullRange      = Cull && Light.Type != LightType::Directional;
		const float Radius = (UsesLightRange ? Light.Range : Light.ShadowDistance) + Light.ShadowNormalBias;

		const Geometry::Sphere Influence = { Light.Position.XYZ(), Radius };
		const float RangeTolerance = 0.0001f * std::max(1.0f, std::max(Radius, 0.0f));

		for (size_t i = 0; i < Casters.size(); ++i)
		{
			if (!CullRange || Geometry::BoxSphereIntersection(Casters[i].Bounds, Influence, RangeTolerance))
			{
				Candidates.push_back(i);
			}
		}
	}

	static void CullShadowView(ShadowDrawView& View, const std::vector<ShadowCaster>& Casters, const std::vector<size_t>& Candidates, bool Cull)
	{
		Matrix GuardScale(1);
		GuardScale.M[0][0] = GuardScale.M[1][1] = float(View.Tile.Block.Size - 2 * ShadowTilePadding) / View.Tile.Block.Size;
		View.RasterWorldToClip = GuardScale * View.GPU.WorldToClip;

		const Frustum ViewFrustum(View.RasterWorldToClip, Frustum::DepthRange::ZeroToOne);
		View.Casters.reserve(Candidates.size());

		for (size_t Candidate : Candidates)
		{
			const auto& Caster = Casters[Candidate];

			if (!Cull || ViewFrustum.Intersects(Caster.Bounds))
			{
				View.Casters.push_back(Caster.ObjectId);
			}
		}
	}

	static void PrepareShadowCasters(GPUScene& Scene, ShadowFrameData& Frame)
	{
		PROFILE_CPU(CpuCounterShadowCulling);

		if (Frame.Views.empty())
		{
			return;
		}

		const bool Cull = CVar_ShadowCasterCulling.GetValue();
		const std::vector<ShadowCaster> Casters = GatherShadowCasters(Scene, Cull);

		std::vector<size_t> Candidates;
		Candidates.reserve(Casters.size());

		u64 Possible       = 0;
		u64 RangeRejects   = 0;
		u64 FrustumRejects = 0;
		u64 Draws          = 0;

		for (const auto& Light : Frame.Lights)
		{
			if (!Light.ShadowViewCount)
			{
				continue;
			}

			GatherLightCandidates(Light, Casters, Cull, Candidates);
			Possible     += u64(Casters.size()) * Light.ShadowViewCount;
			RangeRejects += u64(Casters.size() - Candidates.size()) * Light.ShadowViewCount;

			for (u32 i = 0; i < Light.ShadowViewCount; ++i)
			{
				auto& View = Frame.Views[Light.FirstShadowView + i];
				CullShadowView(View, Casters, Candidates, Cull);

				FrustumRejects += Candidates.size() - View.Casters.size();
				Draws          += View.Casters.size();
			}
		}

		AddProfilingCount(CountShadowDrawsPossible, Possible);
		AddProfilingCount(CountShadowRangeRejects, RangeRejects);
		AddProfilingCount(CountShadowFrustumRejects, FrustumRejects);
		AddProfilingCount(CountShadowDraws, Draws);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Frame light preparation and GPU resources

	static u32 RasterShadowViewCount(const GPULightCompact& Light)
	{
		switch (Light.Type)
		{
		case LightType::Directional:
			return std::clamp(Light.ShadowCascadeCount, 1u, 4u);
		case LightType::Point:
			return 6;
		case LightType::Spot:
			return 1;
		case LightType::Rectangle:
		case LightType::Disc:
			return (Light.Flags & ELightFlags::TwoSided) != 0 ? 2 : 1;
		default:
			return 0;
		}
	}

	static GPULightCompact PrepareFrameLight(RenderGraph& Graph, ShadowRenderState& State, u32 LightIndex, int Mode)
	{
		const auto& Source = Graph.Scene->Lights[LightIndex];
		GPULightCompact Light = CreateCompact(Source);

		Light.FirstShadowView   = 0;
		Light.ShadowViewCount   = 0;
		Light.RTShadowMaskIndex = 0;

		const bool CastsShadow = (Light.Flags & ELightFlags::Shadow) != 0;

		if (!CastsShadow)
		{
			return Light;
		}

		// Override only the frame proxy; authored light flags remain unchanged.
		if (Mode == 1)
		{
			Light.Flags = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::RayTracedShadow));
		}
		else if (Mode == 2)
		{
			Light.Flags = Light.Flags | ELightFlags::RayTracedShadow;
		}

		const bool WantsRT    = (Light.Flags & ELightFlags::RayTracedShadow) != 0;
		const bool SupportsRT = Graph.Device->SupportsRayTracing();

		if (WantsRT && SupportsRT)
		{
			if (State.Frame.RTLightCount >= RTShadowMaskDescriptorCount)
			{
				Log::Error("Ray-traced shadow mask descriptor capacity exceeded");
				COLUMBUS_ASSERT_MESSAGE(false, "Ray-traced shadow mask descriptor capacity exceeded");
				Light.Flags = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::Shadow));
				return Light;
			}

			Light.RTShadowMaskIndex = State.Frame.RTLightCount++;
			return Light;
		}

		Light.Flags            = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::RayTracedShadow));
		Light.ShadowResolution = (u32)Math::UpperPowerOf2(std::clamp(Light.ShadowResolution, 32u, 4096u));

		if (Light.ShadowResolution > State.AtlasSize)
		{
			Log::Error("ShadowResolution exceeds r.Shadows.AtlasSize; choose a larger atlas page");
			COLUMBUS_ASSERT_MESSAGE(false, "ShadowResolution exceeds r.Shadows.AtlasSize; choose a larger atlas page");
			Light.Flags = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::Shadow));
			return Light;
		}

		Light.ShadowViewCount = RasterShadowViewCount(Light);

		if (!Light.ShadowViewCount)
		{
			Light.Flags = ELightFlags(u32(Light.Flags) & ~u32(ELightFlags::Shadow));
		}

		return Light;
	}

	static void ReleaseObsoleteRTHistories(RenderGraph& Graph, ShadowRenderState& State, const std::unordered_set<u64>& RTKeys)
	{
		for (auto It = State.RTHistories.begin(); It != State.RTHistories.end();)
		{
			if (!RTKeys.contains(It->first))
			{
				It->second.Destroy(Graph.Device);
				It = State.RTHistories.erase(It);
			}
			else
			{
				++It;
			}
		}
	}

	static void PrepareFrameLights(RenderGraph& Graph, ShadowRenderState& State)
	{
		const int Mode = CVar_ShadowMode.GetValue();
		auto& Lights = State.Frame.Lights;
		Lights.reserve(Graph.Scene->Lights.Size());

		std::unordered_set<u64> RasterKeys;
		std::unordered_set<u64> RTKeys;

		// Release obsolete or resized allocations before servicing new requests.
		// This avoids growing pages unnecessarily when lights switch mode in the same frame.
		for (u32 i = 0; i < Graph.Scene->Lights.Size(); ++i)
		{
			const GPULightCompact Light = PrepareFrameLight(Graph, State, i, Mode);
			const u64 Key = ShadowLightKey(*Graph.Scene, i);

			const bool IsRasterLight = Light.ShadowViewCount != 0;
			const bool IsRTLight     = (Light.Flags & ELightFlags::Shadow) != 0 && (Light.Flags & ELightFlags::RayTracedShadow) != 0;

			if (IsRasterLight)
			{
				RasterKeys.insert(Key);
			}

			if (IsRTLight && Graph.Scene->Lights[i].NeedsRTShadowDenoiser())
			{
				RTKeys.insert(Key);
			}

			Lights.push_back(Light);
			ReleaseChangedAllocation(State, Key, Light);
		}

		ReleaseObsoleteAllocations(State, RasterKeys);
		ReleaseObsoleteRTHistories(Graph, State, RTKeys);
	}

	static void RegisterShadowAtlases(RenderGraph& Graph, ShadowRenderState& State)
	{
		auto& Frame = State.Frame;

		// Retain pages for reuse, but bind none when no raster views are scheduled.
		const u32 AtlasBindingCount = Frame.Views.empty() ? 0 : (u32)State.Pages.size();
		Frame.Atlases.reserve(AtlasBindingCount);

		for (u32 i = 0; i < AtlasBindingCount; ++i)
		{
			char Name[64];
			snprintf(Name, sizeof(Name), "ShadowAtlas %u", i);
			Frame.Atlases.push_back(Graph.RegisterExternalTexture(State.Pages[i].Texture, Name));
		}
	}

	static void UploadShadowViews(RenderGraph& Graph, ShadowFrameData& Frame)
	{
		// A valid one-record buffer also supports frames without any raster lights.
		std::vector<GPUShadowView> GPUViews(std::max<size_t>(Frame.Views.size(), 1));

		for (size_t i = 0; i < Frame.Views.size(); ++i)
		{
			GPUViews[i] = Frame.Views[i].GPU;
		}

		BufferDesc ViewBufferDesc;
		ViewBufferDesc.Size                = GPUViews.size() * sizeof(GPUShadowView);
		ViewBufferDesc.BindFlags           = BufferType::UAV;
		ViewBufferDesc.MiscFlags           = 0;
		ViewBufferDesc.StructureByteStride = sizeof(GPUShadowView);

		Frame.ViewBuffer = Graph.CreateBuffer(ViewBufferDesc, "ShadowViews");

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.WriteBuffer(Frame.ViewBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("Upload Shadow Views", RenderGraphPassType::Compute, {}, Dependencies,
			[Buffer = Frame.ViewBuffer, Data = std::move(GPUViews)](RenderGraphContext& Context)
			{
				Context.CommandBuffer->UpdateBuffer(Context.GetRenderGraphBuffer(Buffer).get(), Data.data(), Data.size() * sizeof(GPUShadowView));
			}
		);
	}

	void PrepareShadows(RenderGraph& Graph, const RenderView& View, ShadowRenderState& State)
	{
		PROFILE_CPU(CpuCounterShadowPreparation);

		const u32 AtlasSize = (u32)Math::UpperPowerOf2(std::clamp(CVar_ShadowAtlasSize.GetValue(), 256, 4096));

		const bool SceneChanged     = State.SceneIdentity != Graph.Scene.get();
		const bool AtlasSizeChanged = State.AtlasSize != 0 && State.AtlasSize != AtlasSize;

		if (SceneChanged || AtlasSizeChanged)
		{
			State.Destroy(Graph.Device);
		}

		State.SceneIdentity = Graph.Scene.get();
		State.AtlasSize     = AtlasSize;
		State.Frame         = {};

		PrepareFrameLights(Graph, State);

		for (u32 i = 0; i < State.Frame.Lights.size(); ++i)
		{
			if (State.Frame.Lights[i].ShadowViewCount)
			{
				BuildLightViews(Graph, View, State, i);
			}
		}

		PrepareShadowCasters(*Graph.Scene, State.Frame);
		AddProfilingCount(CountShadowViews, State.Frame.Views.size());

		RegisterShadowAtlases(Graph, State);
		UploadShadowViews(Graph, State.Frame);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Shadow depth rendering

	struct ShadowDrawConstants
	{
		Matrix WorldToClip;
		u32    ObjectId;
	};

	struct ShadowDepthShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/ShadowDepth.csd";

		struct Permutation
		{
			int AlphaMasked = 0;
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
			Builder.Axis("ALPHA_MASKED", &Permutation::AlphaMasked);
		}

		struct PipelinePermutation
		{
			float SlopeBias = 0;
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "ShadowDepth";

			Desc.rasterizerState.Cull                 = CullMode::No; // Thin and mirrored casters remain valid.
			Desc.rasterizerState.SlopeScaledDepthBias = -Permutation.SlopeBias;

			Desc.depthStencilState.DepthFunc = ComparisonFunc::GEqual;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
		}
	};

	static void RenderShadowView(RenderGraphContext& Context, const ShadowDrawView& View, const ShadowDepthShader::Parameters& ShaderParams)
	{
		GraphicsPipeline* Pipelines[2]  = {};
		GraphicsPipeline* BoundPipeline = nullptr;
		const auto& Tile = View.Tile.Block;

		Context.CommandBuffer->SetViewport(float(Tile.X), float(Tile.Y), float(Tile.Size), float(Tile.Size), 0, 1);
		// Rasterise the guard region too, with the same projection-to-texel mapping.
		Context.CommandBuffer->SetScissor(Tile.X, Tile.Y, Tile.Size, Tile.Size);

		for (u32 ObjectId : View.Casters)
		{
			const auto&     Mesh     = Context.Scene->Meshes[ObjectId];
			const Material* Material = Context.Scene->Materials.Get(Mesh.MaterialId);
			const bool      Masked   = Material && Material->ShadingMode == MaterialShadingMode::Masked;

			GraphicsPipeline*& Pipeline = Pipelines[Masked ? 1 : 0];

			if (!Pipeline)
			{
				Pipeline = GetGraphicsPipeline<ShadowDepthShader>(Context, { Masked ? 1 : 0 }, { View.SlopeBias });
			}

			// Keep the prepared caster order; sorting is a separate change.
			if (Pipeline != BoundPipeline)
			{
				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
				Context.BindGraphicsParameters<ShadowDepthShader>(Pipeline, ShaderParams);
				BoundPipeline = Pipeline;
			}

			ShadowDrawConstants Constants{ View.RasterWorldToClip, ObjectId };
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Constants), &Constants);
			Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
		}
	}

	static void AddShadowAtlasPass(RenderGraph& Graph, RenderGraphTextureRef Atlas, u32 AtlasIndex, std::vector<ShadowDrawView> Views)
	{
		RenderPassParameters Parameters;
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Atlas, AttachmentClearValue{ {}, 0, 0 } };
		Parameters.ViewportSize           = Graph.GetTextureSize2D(Atlas);

		RenderPassDependencies Dependencies(Graph.Allocator);
		ShadowDepthShader::Parameters ShaderParams;
		ShaderParams.Scene.UseCombinedSampler = false;
		Dependencies.Bind<ShadowDepthShader>(ShaderParams);

		char Name[64];
		snprintf(Name, sizeof(Name), "ShadowMap Atlas %u", AtlasIndex);

		Graph.AddPass(Name, RenderGraphPassType::Raster, Parameters, Dependencies,
			[Views = std::move(Views), ShaderParams](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterShadowMaps, Context);

				for (const auto& View : Views)
				{
					RenderShadowView(Context, View, ShaderParams);
				}
			}
		);
	}

	void RenderShadowMaps(RenderGraph& Graph, const ShadowFrameData& Frame)
	{
		RENDER_GRAPH_SCOPED_MARKER(Graph, "Raster Shadows");

		for (u32 Atlas = 0; Atlas < Frame.Atlases.size(); ++Atlas)
		{
			std::vector<ShadowDrawView> Views;

			for (const auto& View : Frame.Views)
			{
				if (View.Tile.AtlasIndex == Atlas)
				{
					Views.push_back(View);
				}
			}

			if (!Views.empty())
			{
				AddShadowAtlasPass(Graph, Frame.Atlases[Atlas], Atlas, std::move(Views));
			}
		}
	}
}
