#include "BakedLightingRuntime.h"

#include "Core/CVar.h"
#include "Graphics/GPUScene.h"
#include "Math/Geometry.h"

#include <algorithm>

namespace Columbus
{
	extern ConsoleVariable<float> CVar_MaxRoughness;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Irradiance volumes

	namespace IrradianceVolumes
	{
		Prepared Prepare(RenderGraph& Graph, RenderGraphBufferRef FallbackBuffer, bool bEnabled)
		{
			Prepared Result{};
			Result.Constants.Value.CountAndFlags = iVector4(0, bEnabled ? 1 : 0, 0, 0);

			for (int i = 0; i < MaxVolumes; i++)
			{
				Result.ProbeBuffers[i] = FallbackBuffer;
			}

			if (!bEnabled || Graph.Scene == nullptr)
			{
				return Result;
			}

			int VolumeIndex = 0;
			for (const GPUIrradianceVolume& Volume : Graph.Scene->IrradianceVolumes)
			{
				if (Volume.Probes == nullptr || VolumeIndex >= MaxVolumes)
				{
					continue;
				}

				RenderGraphBufferRef ProbeBuffer = Graph.RegisterExternalBuffer(Volume.Probes, "IrradianceProbes");
				Result.ProbeBuffers[VolumeIndex] = ProbeBuffer;
				Result.Constants.Value.Volumes[VolumeIndex] = VolumeDesc{
					.PositionIntensity = Vector4(Volume.Position, Volume.Intensity),
					.ExtentNormalBias = Vector4(Volume.Extent, Volume.NormalBias),
					.BlendPriority = Vector4(Volume.BlendDistance, Volume.Priority, 0.0f, 0.0f),
					.ProbesCountAndBufferIndex = iVector4(Volume.GetProbesCount(), VolumeIndex),
				};

				VolumeIndex++;
			}

			Result.Constants.Value.CountAndFlags.X = VolumeIndex;
			if (VolumeIndex == 0)
			{
				Result.Constants.Value.CountAndFlags.Y = 0;
			}

			return Result;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Reflection captures

	namespace ReflectionCaptures
	{
		ConsoleVariable<bool> CVar_ReflectionCaptures("r.ReflectionCaptures", "Sample baked reflection captures", true);
		ConsoleVariable<float> CVar_ReflectionCaptureFadeWidth("r.ReflectionCaptures.FadeWidth", "Roughness fade width around RT reflection max roughness", 0.08f);

		struct CaptureCandidate
		{
			const GPUReflectionCapture* Capture = nullptr;
			float Distance = 0.0f;
			int DenseIndex = 0;
		};

		static bool IsBetterCandidate(const CaptureCandidate& A, const CaptureCandidate& B)
		{
			if (B.Capture == nullptr)
				return true;

			if (A.Capture == nullptr)
				return false;

			if (A.Capture->Priority != B.Capture->Priority)
				return A.Capture->Priority > B.Capture->Priority;

			if (A.Distance != B.Distance)
				return A.Distance < B.Distance;

			return A.DenseIndex < B.DenseIndex;
		}

		static bool IsWorseCandidate(const CaptureCandidate& A, const CaptureCandidate& B)
		{
			return IsBetterCandidate(B, A);
		}

		static float GetCaptureCameraDistance(const GPUReflectionCapture& Capture, const Vector3& CameraPosition)
		{
			const Vector3 HalfExtent = Capture.Extent * 0.5f;
			return Geometry::PointBoxDistance(CameraPosition, Box(Capture.Position - HalfExtent, Capture.Position + HalfExtent));
		}

		Prepared Prepare(RenderGraph& Graph, const RenderView& View, bool bEnabled, bool bRTReflectionsAvailable)
		{
			Prepared Result{};
			bEnabled = bEnabled && CVar_ReflectionCaptures.GetValue();
			Result.Constants.Value.CountAndFlags = iVector4(0, bEnabled ? 1 : 0, 0, 0);
			Result.Constants.Value.Params = Vector4(CVar_MaxRoughness.GetValue(), CVar_ReflectionCaptureFadeWidth.GetValue(), bRTReflectionsAvailable ? 1.0f : 0.0f, 0.0f);

			if (!bEnabled || Graph.Scene == nullptr)
			{
				return Result;
			}

			std::array<CaptureCandidate, MaxCaptures> Candidates{};
			int CandidateCount = 0;
			int DenseIndex = -1;
			for (const GPUReflectionCapture& Capture : Graph.Scene->ReflectionCaptures)
			{
				DenseIndex++;

				if (Capture.Radiance == nullptr || Capture.Intensity <= 0.0f)
					continue;

				CaptureCandidate Candidate{
					.Capture = &Capture,
					.Distance = GetCaptureCameraDistance(Capture, View.CameraCur.Pos),
					.DenseIndex = DenseIndex,
				};

				if (CandidateCount < MaxCaptures)
				{
					Candidates[CandidateCount++] = Candidate;
					continue;
				}

				int WorstCandidate = 0;
				for (int i = 1; i < MaxCaptures; i++)
				{
					if (IsWorseCandidate(Candidates[i], Candidates[WorstCandidate]))
					{
						WorstCandidate = i;
					}
				}

				if (IsBetterCandidate(Candidate, Candidates[WorstCandidate]))
				{
					Candidates[WorstCandidate] = Candidate;
				}
			}

			std::sort(Candidates.begin(), Candidates.begin() + CandidateCount, IsBetterCandidate);

			for (int CaptureIndex = 0; CaptureIndex < CandidateCount; CaptureIndex++)
			{
				const GPUReflectionCapture& Capture = *Candidates[CaptureIndex].Capture;

				Result.Radiance[CaptureIndex] = Capture.Radiance;
				Result.Constants.Value.Captures[CaptureIndex] = CaptureDesc{
					.PositionIntensity = Vector4(Capture.Position, Capture.Intensity),
					.ExtentBlendDistance = Vector4(Capture.Extent, Capture.BlendDistance),
					.PriorityMipEnabled = Vector4(Capture.Priority, (float)Capture.MipCount, 1.0f, 0.0f),
				};
			}

			Result.Constants.Value.CountAndFlags.X = CandidateCount;
			if (CandidateCount == 0)
			{
				Result.Constants.Value.CountAndFlags.Y = 0;
			}

			return Result;
		}

		void SetShaderResources(ShaderResources& Resources, const Prepared& Captures, Texture2* BRDFLUT)
		{
			Resources.Constants = Captures.Constants;
			Resources.BRDFLUT = BRDFLUT;
			Resources.RadianceTextures.Set(Captures.Radiance.data(), (u32)Math::Max(Captures.Constants.Value.CountAndFlags.X, 0));
		}

		void BindReflectionCaptureResources(ShaderBinder& Binder, const ShaderResources& Resources, int Set)
		{
			Binder.Bind(Resources.Constants, Set, 0);
			Binder.Bind(Resources.Sampler, Set, 1);
			Binder.Bind(Resources.BRDFLUT, Set, 2);
			Binder.Bind(Resources.RadianceTextures, Set, 3);
		}
	}
}
