#include <Graphics/ShaderCompiler.h>

namespace Columbus
{

std::string CommonShaderHeader =
R"(#version 430 core

#define Texture2D sampler2D
#define Texture3D sampler3D
#define TextureCube samplerCube

#define Texture2DMS sampler2DMS
#define Texture2DShadow sampler2DShadow

#define saturate(x) clamp(x, 0, 1)

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define float2x2 mat2x2
#define float2x3 mat2x3
#define float2x4 mat2x4
#define float3x2 mat3x2
#define float3x3 mat3x3
#define float3x4 mat3x4
#define float4x2 mat4x2
#define float4x3 mat4x3
#define float4x4 mat4x4

#if __VERSION__ < 130
	#define Sample2D(tex, uv) texture2D(tex, uv)
	#define Sample3D(tex, uv) texture3D(tex, uv)
	#define SampleCube(tex, uv) textureCube(tex, uv)

	#define Sample2DLod(tex, uv, lod) texture2DLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) texture3DLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureCubeLod(tex, uv, lod)
	#define Sample2DShadow(tex, uvc) shadow2D(tex, uvc)
#else
	#define Sample2D(tex, uv) texture(tex, uv)
	#define Sample3D(tex, uv) texture(tex, uv)
	#define SampleCube(tex, uv) texture(tex, uv)

	#define Sample2DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureLod(tex, uv, lod)

	#define Sample2DMS(tex, uv, lod) texelFetch(tex, uv, lod)
	#define Sample2DShadow(tex, uvc) texture(tex, uvc)
#endif
)";

const std::string ComputeShaderHeader =
R"(
#define begin_cbuffer(name, slot) layout(std140, binding = slot) uniform name {
#define end_cbuffer(name) } name;
#define begin_uav(name,  slot) layout(std430, binding = slot) buffer name {
#define end_uav(name) } name;
)";

	CompiledProgram ShaderCompiler::Compile(const std::string& source, const std::vector<std::string>& defines)
	{
		CompiledProgram prog;

		CompiledShader compute;
		compute.source = CommonShaderHeader + ComputeShaderHeader;
		for (const auto& def : defines)
		{
			compute.source += "#define " + def + "\n";
		}
		compute.source += source;

		prog.shaders.push_back(compute);

		return prog;
	}

}
