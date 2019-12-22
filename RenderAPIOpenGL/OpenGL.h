#pragma once

#include <Core/Types.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class OpenGL
	{
	private:
		static int32 MaxCubemapTextureSize;
		static int32 MaxTextureSize;
		static int32 MaxColorTextureSamples;
		static int32 MaxDepthTextureSamples;
		static int32 MaxComputeWorkGroupInvocations;
		static int32 MaxComputeSharedMemorySize;
		static iVector3 MaxComputeWorkGroupSize;
		static iVector3 MaxComputeWorkGroupCount;

		static bool bSupportsBufferStorage;
		static bool bSupportsUniformBuffer;
		static bool bSupportsInstancing;
		static bool bSupportsTransformFeedback;
		static bool bSupportsGeometryShader;
		static bool bSupportsTesselation;
		static bool bSupportsComputeShader;
		static bool bSupportsDirectStateAccess;
		static bool bSupportsProgramBinary;
		static bool bSupportsShaderStorageBuffer;
		static bool bSupportsShaderImageLoadStore;
		static bool bSupportsMultiBind;
	public:
		static void Init();

		static inline int32 GetMaxCubemapTextureSize() { return MaxCubemapTextureSize; }
		static inline int32 GetMaxTextureSize() { return MaxTextureSize; }
		static inline int32 GetMaxColorTextureSamples() { return MaxColorTextureSamples; }
		static inline int32 GetMaxDepthTextureSamples() { return MaxDepthTextureSamples; }
		static inline int32 GetMaxComputeWorkGroupInvocations() { return MaxComputeWorkGroupInvocations; }
		static inline int32 GetMaxComputeSharedMemorySize() { return MaxComputeSharedMemorySize; }
		static inline iVector3 GetMaxComputeWorkGroupSize() { return MaxComputeWorkGroupSize; }
		static inline iVector3 GetMaxComputeWorkGroupCount() { return MaxComputeWorkGroupCount; }

		static inline uint64 GetVideoMemorySize() { return 0; }
		static inline uint64 GetFreeVideoMemorySize() { return 0; }

		static inline bool SupportsBufferStorage() { return bSupportsBufferStorage; }
		static inline bool SupportsUniformBuffer() { return bSupportsUniformBuffer; }
		static inline bool SupportsInstancing() { return bSupportsInstancing; }
		static inline bool SupportsTransformFeedback() { return bSupportsTransformFeedback; }
		static inline bool SupportsGeometryShader() { return bSupportsGeometryShader; }
		static inline bool SupportsTesselation() { return bSupportsTesselation; }
		static inline bool SupportsComputeShader() { return bSupportsComputeShader; }
		static inline bool SupportsDirectStateAccess() { return bSupportsDirectStateAccess; }
		static inline bool SupportsProgramBinary() { return bSupportsProgramBinary; }
		static inline bool SupportsShaderStorageBuffer() { return bSupportsShaderStorageBuffer; }
		static inline bool SupportsShaderImageLoadStore() { return bSupportsShaderImageLoadStore; }
		static inline bool SupportsMultiBind() { return bSupportsMultiBind; }
	};

}


