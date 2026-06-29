#pragma once

#include <Graphics/Core/AccelerationStructure.h>
#include <Graphics/Core/Buffer.h>
#include <Graphics/Core/Texture.h>
#include <Graphics/Core/Types.h>
#include <Graphics/Core/Pipelines.h>
#include <ShaderBytecode/ShaderBytecode.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Columbus
{
	using RenderGraphTextureRef = int;
	using RenderGraphBufferRef = int;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader binding types definitions

	template <typename T>
	struct ShaderConstants;

	template <typename T>
	struct ShaderPushConstants;

	template <typename TResource>
	struct ShaderArray;

	struct ShaderGPUScene;
	struct ShaderStaticSampler;
	struct ShaderSampler;

	struct ShaderSampledTexture;
	struct ShaderStorageTexture;
	struct ShaderReadStorageTexture;

	struct ShaderReadBuffer;
	struct ShaderWriteBuffer;

	struct ShaderAccelerationStructure;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader binding value types

	template <typename T>
	struct ShaderConstants
	{
		T Value;
	};

	template <typename T>
	struct ShaderPushConstants
	{
		T Value;
		ShaderType Stages = ShaderType::AllGraphics;
	};

	struct ShaderGPUScene
	{
		bool UseCombinedSampler = true;
	};

	struct ShaderStaticSampler
	{
		SamplerDesc Desc;
	};

	struct ShaderSampler
	{
		Sampler* Value = nullptr;

		ShaderSampler& operator=(Sampler* InValue);
	};

	template <typename TResource>
	struct ShaderArray
	{
		const TResource* Data = nullptr;
		uint32_t Count = 0;

		ShaderArray& Set(const TResource* InData, uint32_t InCount);
	};

	struct ShaderSampledTexture
	{
		RenderGraphTextureRef Ref = -1;
		Texture2* External = nullptr;
		TextureBindingFlags Aspect = TextureBindingFlags::AspectColour;

		ShaderSampledTexture();
		explicit ShaderSampledTexture(TextureBindingFlags InAspect);

		ShaderSampledTexture& operator=(RenderGraphTextureRef InRef);
		ShaderSampledTexture& operator=(Texture2* InTexture);
	};

	struct ShaderStorageTexture
	{
		RenderGraphTextureRef Ref = -1;
		Texture2* External = nullptr;
		TextureBindingFlags Aspect = TextureBindingFlags::AspectColour;

		ShaderStorageTexture();
		explicit ShaderStorageTexture(TextureBindingFlags InAspect);

		ShaderStorageTexture& operator=(RenderGraphTextureRef InRef);
		ShaderStorageTexture& operator=(Texture2* InTexture);
	};

	struct ShaderReadStorageTexture
	{
		RenderGraphTextureRef Ref = -1;
		Texture2* External = nullptr;
		TextureBindingFlags Aspect = TextureBindingFlags::AspectColour;

		ShaderReadStorageTexture();
		explicit ShaderReadStorageTexture(TextureBindingFlags InAspect);

		ShaderReadStorageTexture& operator=(RenderGraphTextureRef InRef);
		ShaderReadStorageTexture& operator=(Texture2* InTexture);
	};

	struct ShaderReadBuffer
	{
		RenderGraphBufferRef Ref = -1;
		Buffer* External = nullptr;

		ShaderReadBuffer& operator=(RenderGraphBufferRef InRef);
		ShaderReadBuffer& operator=(Buffer* InBuffer);
	};

	struct ShaderWriteBuffer
	{
		RenderGraphBufferRef Ref = -1;
		Buffer* External = nullptr;

		ShaderWriteBuffer& operator=(RenderGraphBufferRef InRef);
		ShaderWriteBuffer& operator=(Buffer* InBuffer);
	};

	struct ShaderAccelerationStructure
	{
		const AccelerationStructure* Value = nullptr;

		ShaderAccelerationStructure& operator=(const AccelerationStructure* InValue);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader binding value helpers

	inline ShaderSampler& ShaderSampler::operator=(Sampler* InValue)
	{
		Value = InValue;
		return *this;
	}

	template <typename TResource>
	ShaderArray<TResource>& ShaderArray<TResource>::Set(const TResource* InData, uint32_t InCount)
	{
		Data = InData;
		Count = InCount;
		return *this;
	}

	inline ShaderSampledTexture::ShaderSampledTexture() = default;

	inline ShaderSampledTexture::ShaderSampledTexture(TextureBindingFlags InAspect) : Aspect(InAspect)
	{
	}

	inline ShaderSampledTexture& ShaderSampledTexture::operator=(RenderGraphTextureRef InRef)
	{
		Ref = InRef;
		External = nullptr;
		return *this;
	}

	inline ShaderSampledTexture& ShaderSampledTexture::operator=(Texture2* InTexture)
	{
		Ref = -1;
		External = InTexture;
		return *this;
	}

	inline ShaderStorageTexture::ShaderStorageTexture() = default;

	inline ShaderStorageTexture::ShaderStorageTexture(TextureBindingFlags InAspect) : Aspect(InAspect)
	{
	}

	inline ShaderStorageTexture& ShaderStorageTexture::operator=(RenderGraphTextureRef InRef)
	{
		Ref = InRef;
		External = nullptr;
		return *this;
	}

	inline ShaderStorageTexture& ShaderStorageTexture::operator=(Texture2* InTexture)
	{
		Ref = -1;
		External = InTexture;
		return *this;
	}

	inline ShaderReadStorageTexture::ShaderReadStorageTexture() = default;

	inline ShaderReadStorageTexture::ShaderReadStorageTexture(TextureBindingFlags InAspect) : Aspect(InAspect)
	{
	}

	inline ShaderReadStorageTexture& ShaderReadStorageTexture::operator=(RenderGraphTextureRef InRef)
	{
		Ref = InRef;
		External = nullptr;
		return *this;
	}

	inline ShaderReadStorageTexture& ShaderReadStorageTexture::operator=(Texture2* InTexture)
	{
		Ref = -1;
		External = InTexture;
		return *this;
	}

	inline ShaderReadBuffer& ShaderReadBuffer::operator=(RenderGraphBufferRef InRef)
	{
		Ref = InRef;
		External = nullptr;
		return *this;
	}

	inline ShaderReadBuffer& ShaderReadBuffer::operator=(Buffer* InBuffer)
	{
		Ref = -1;
		External = InBuffer;
		return *this;
	}

	inline ShaderWriteBuffer& ShaderWriteBuffer::operator=(RenderGraphBufferRef InRef)
	{
		Ref = InRef;
		External = nullptr;
		return *this;
	}

	inline ShaderWriteBuffer& ShaderWriteBuffer::operator=(Buffer* InBuffer)
	{
		Ref = -1;
		External = InBuffer;
		return *this;
	}

	inline ShaderAccelerationStructure& ShaderAccelerationStructure::operator=(const AccelerationStructure* InValue)
	{
		Value = InValue;
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader binder API

	struct RenderPassDependencies;
	struct RenderGraphContext;
	struct ShaderBinderBackendState
	{
	};


	class ShaderBinder
	{
	public:
		ShaderBinder(RenderPassDependencies& InDependencies);
		ShaderBinder(RenderGraphContext& InContext, const ComputePipeline* InPipeline, ShaderBinderBackendState& InBackend);
		ShaderBinder(RenderGraphContext& InContext, const GraphicsPipeline* InPipeline, ShaderBinderBackendState& InBackend);
		ShaderBinder(RenderGraphContext& InContext, const RayTracingPipeline* InPipeline, ShaderBinderBackendState& InBackend);

		bool IsCollectingDependencies() const;
		bool IsBindingRuntime() const;

		void Flush();

		void Bind(const ShaderSampledTexture& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindSampledTextureImpl(Resource, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderStorageTexture& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindStorageTextureImpl(Resource, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderReadStorageTexture& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindReadStorageTextureImpl(Resource, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderReadBuffer& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindReadBufferImpl(Resource, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderWriteBuffer& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindWriteBufferImpl(Resource, Set, Binding, ArrayIndex);
		}

		template <typename T>
		void Bind(const ShaderArray<T>& Resources, int Set, int Binding);

		void Bind(const ShaderStaticSampler& Sampler, int Set, int Binding, int ArrayIndex = 0)
		{
			BindStaticSamplerImpl(Sampler, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderSampler& Sampler, int Set, int Binding, int ArrayIndex = 0)
		{
			BindSamplerImpl(Sampler, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderAccelerationStructure& Resource, int Set, int Binding, int ArrayIndex = 0)
		{
			BindAccelerationStructureImpl(Resource, Set, Binding, ArrayIndex);
		}

		void Bind(const ShaderGPUScene& Scene)
		{
			BindGPUSceneImpl(Scene);
		}

		template <typename T>
		void Bind(const ShaderConstants<T>& Constants, int Set, int Binding, int ArrayIndex = 0);

		template <typename T>
		void Bind(const ShaderPushConstants<T>& Constants);

	private:
		enum class Mode
		{
			CollectDependencies,
			BindCompute,
			BindGraphics,
			BindRayTracing
		};

		RenderPassDependencies* Dependencies = nullptr;
		RenderGraphContext* Context = nullptr;
		const ComputePipeline* Compute = nullptr;
		const GraphicsPipeline* Graphics = nullptr;
		const RayTracingPipeline* RayTracing = nullptr;
		Mode CurrentMode = Mode::CollectDependencies;
		ShaderBinderBackendState* Backend = nullptr;

		void BindSampledTextureImpl(const ShaderSampledTexture& Resource, int Set, int Binding, int ArrayIndex);
		void BindStorageTextureImpl(const ShaderStorageTexture& Resource, int Set, int Binding, int ArrayIndex);
		void BindReadStorageTextureImpl(const ShaderReadStorageTexture& Resource, int Set, int Binding, int ArrayIndex);
		void BindReadBufferImpl(const ShaderReadBuffer& Resource, int Set, int Binding, int ArrayIndex);
		void BindWriteBufferImpl(const ShaderWriteBuffer& Resource, int Set, int Binding, int ArrayIndex);
		void BindStaticSamplerImpl(const ShaderStaticSampler& Sampler, int Set, int Binding, int ArrayIndex);
		void BindSamplerImpl(const ShaderSampler& Sampler, int Set, int Binding, int ArrayIndex);
		void BindAccelerationStructureImpl(const ShaderAccelerationStructure& Resource, int Set, int Binding, int ArrayIndex);
		void BindGPUSceneImpl(const ShaderGPUScene& Scene);
		void BindDescriptorSetImpl(int Set);
		void BindConstantsData(const void* Data, size_t Size, int Set, int Binding, int ArrayIndex);
		void BindPushConstantsData(const void* Data, size_t Size, ShaderType Stages);
	};


	template <typename T>
	void ShaderBinder::Bind(const ShaderArray<T>& Resources, int Set, int Binding)
	{
		BindDescriptorSetImpl(Set);
		for (uint32_t Index = 0; Index < Resources.Count; Index++)
		{
			Bind(Resources.Data[Index], Set, Binding, (int)Index);
		}
	}

	template <typename T>
	void ShaderBinder::Bind(const ShaderConstants<T>& Constants, int Set, int Binding, int ArrayIndex)
	{
		BindConstantsData(&Constants.Value, sizeof(Constants.Value), Set, Binding, ArrayIndex);
	}

	template <typename T>
	void ShaderBinder::Bind(const ShaderPushConstants<T>& Constants)
	{
		BindPushConstantsData(&Constants.Value, sizeof(Constants.Value), Constants.Stages);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader permutation and pipeline helpers

	template <typename TPermutation, uint32_t MaxAxes = 8>
	struct ShaderPermutationLayoutBuilder
	{
		struct AxisDesc
		{
			const char* Name = nullptr;
			int TPermutation::* Member = nullptr;
		};

		AxisDesc Axes[MaxAxes] = {};
		uint32_t Count = 0;

		void Axis(const char* Name, int TPermutation::* Member)
		{
			assert(Count < MaxAxes);
			Axes[Count++] = AxisDesc { Name, Member };
		}
	};

	template <typename TShader>
	uint32_t GetShaderPermutationIndex(const CompiledShaderData& Shader, const typename TShader::Permutation& Permutation)
	{
		static const ShaderPermutationLayoutBuilder<typename TShader::Permutation> Layout = []()
		{
			ShaderPermutationLayoutBuilder<typename TShader::Permutation> Builder;
			TShader::BuildPermutationLayout(Builder);
			return Builder;
		}();

		assert(Shader.PermutationAxes.size() == Layout.Count);

		uint32_t Index = 0;
		for (uint32_t AxisIndex = 0; AxisIndex < Layout.Count; AxisIndex++)
		{
			const CompiledShaderPermutationAxis& Axis = Shader.PermutationAxes[AxisIndex];
			const auto& LayoutAxis = Layout.Axes[AxisIndex];
			const int Value = Permutation.*LayoutAxis.Member;

			assert(Axis.Name == LayoutAxis.Name);
			assert(Value >= Axis.MinValue && Value <= Axis.MaxValue);

			const uint32_t AxisCount = (uint32_t)(Axis.MaxValue - Axis.MinValue + 1);
			Index = Index * AxisCount + (uint32_t)(Value - Axis.MinValue);
		}

		assert(Index < Shader.Permutations.size());
		return Index;
	}

	inline size_t HashBytes(const void* Data, size_t Size)
	{
		const uint8_t* Bytes = (const uint8_t*)Data;
		size_t Hash = 14695981039346656037ull;
		for (size_t i = 0; i < Size; i++)
		{
			Hash ^= Bytes[i];
			Hash *= 1099511628211ull;
		}
		return Hash;
	}

	template <typename TPipelinePermutation>
	size_t HashPipelinePermutation(const TPipelinePermutation& Permutation)
	{
		static_assert(std::is_trivially_copyable_v<TPipelinePermutation>);
		return HashBytes(&Permutation, sizeof(Permutation));
	}
}
