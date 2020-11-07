#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/ShaderCompiler.h>
#include <Graphics/Device.h>
#include <Math/Quaternion.h>

namespace Columbus
{

	auto ParticlesLightingProg =
		#include <Graphics/Shaders/ParticlesLighting.csl>
		;
//std::string acomputeprog =
//R"(#version 430 core
//layout(local_size_x = 1, local_size_y = 1) in;
//
//#define MAX_LIGHTS_COUNT 128
//
//struct Light
//{
//	vec3 color; float range;
//	vec3 pos; float innerCutoff;
//	vec3 dir; float outerCutoff;
//	int type;
//	int hasShadow;
//	mat4 lightView;
//	vec4 shadowRect;
//};
//
//// TODO: cbuffer abstraction, binding number
//layout(std140, binding = 0) uniform cb_Lighting
//{
//	Light lights[MAX_LIGHTS_COUNT];
//	int count;
//} u_Lights;
//
//layout(std430, binding = 1) buffer colors
//{
//	vec4 color[];
//} outColors;
//
//layout(std430, binding = 2) buffer positions
//{
//	vec4 pos[];
//} inPositions;
//
//#define LIGHT u_Lights.lights[i]
//
//void main(void)
//{
//	uint id = gl_GlobalInvocationID.x;
//
//	vec3 lighting = vec3(0,0,0);
//	for (int i = 0; i < u_Lights.count; ++i)
//	{
//		vec3 L = normalize(LIGHT.pos - inPositions.pos[id].xyz); if (LIGHT.type == 0) L = normalize(-LIGHT.dir);
//		float distance = length(LIGHT.pos - inPositions.pos[id].xyz);
//		float attenuation = 1.0f;
//		if (LIGHT.type != 0) attenuation = clamp(1.0 - distance * distance / (LIGHT.range * LIGHT.range), 0.0, 1.0);
//		attenuation *= attenuation;
//
//		if (LIGHT.type == 2) // spotlight
//		{
//			float angle;
//			angle = dot(LIGHT.dir, -L);
//			angle = max(angle, 0);
//			angle = acos(angle);
//
//			if (angle < LIGHT.innerCutoff)
//				attenuation *= 1.0;
//			else if (angle < LIGHT.outerCutoff)
//				attenuation *= (1.0 - smoothstep(LIGHT.innerCutoff, LIGHT.outerCutoff, angle));
//			else
//				attenuation = 0.0;
//		}
//
//		lighting += LIGHT.color * attenuation;
//	}
//
//	outColors.color[id] *= vec4(lighting, 1);
//}
//)";

	ComputePipelineState* CPS;

	void ParticlesRenderer::Allocate(size_t NewSize)
	{
		MaxSize = NewSize;

		auto UAV = BufferType::UAV;
		auto Dynamic = BufferUsage::Dynamic;
		auto Write = BufferCpuAccess::Write;

		gDevice->CreateBuffer(BufferDesc(MaxSize * sizeof(Vector4), UAV, Dynamic, Write), nullptr, &PositionsUAV);
		gDevice->CreateBuffer(BufferDesc(MaxSize * sizeof(Vector4), UAV, Dynamic, Write), nullptr, &SizesUAV);
		gDevice->CreateBuffer(BufferDesc(MaxSize * sizeof(Vector4), UAV, Dynamic, Write), nullptr, &ColorsUAV);
		gDevice->CreateBuffer(BufferDesc(MaxSize * sizeof(Vector2), UAV, Dynamic, Write), nullptr, &OtherUAV);
	}

	ParticlesRenderer::ParticlesRenderer(size_t MaxSize)
	{
		Allocate(MaxSize);

		ComputePipelineStateDesc CPSD;
		CPSD.CS = ShaderCompiler::Compile(ParticlesLightingProg, {}).shaders[0].source;
		gDevice->CreateComputePipelineState(CPSD, &CPS);
	}

	void ParticlesRenderer::Render(const ParticleEmitterCPU& Particles, const Camera& MainCamera, const Material& Mat)
	{
		if (MaxSize < Particles.MaxParticles)
		{
			Allocate(Particles.MaxParticles);
		}

		BlendStateDesc BSDesc;
		BlendState* BState;
		BSDesc.RenderTarget[0].BlendEnable = true;
		switch (Particles.Blend)
		{
		case ParticleEmitterCPU::BlendMode::Default:
			BSDesc.RenderTarget[0].SrcBlend = Blend::SrcAlpha;
			BSDesc.RenderTarget[0].DestBlend = Blend::One;
			break;
		case ParticleEmitterCPU::BlendMode::Add:
			BSDesc.RenderTarget[0].SrcBlend = Blend::One;
			BSDesc.RenderTarget[0].DestBlend = Blend::One;
			break;
		}
		gDevice->CreateBlendState(BSDesc, &BState);

		DepthStencilStateDesc DSDesc;
		DepthStencilState* DSState;
		DSDesc.DepthEnable = true;
		DSDesc.DepthWriteMask = false;
		DSDesc.StencilEnable = false;
		gDevice->CreateDepthStencilState(DSDesc, &DSState);

		RasterizerStateDesc RSD;
		RasterizerState* RS;
		RSD.Cull = CullMode::No;
		gDevice->CreateRasterizerState(RSD, &RS);

		float blendFactor[] = { 0, 0, 0, 0 };
		gDevice->OMSetDepthStencilState(DSState, 0);
		gDevice->OMSetBlendState(BState, blendFactor, 0xFFFFFFFF);
		gDevice->RSSetState(RS);

		ShaderProgramOpenGL* Shader = static_cast<ShaderProgramOpenGL*>(Mat.GetShader());
		if (Shader != nullptr)
		{
			Quaternion Q;

			switch (Particles.Billboard)
			{
			case ParticleEmitterCPU::BillboardMode::Vertical:
				Q = Quaternion(Vector3(0, MainCamera.Rot.Y, 0));
				break;
			case ParticleEmitterCPU::BillboardMode::Horizontal:
			case ParticleEmitterCPU::BillboardMode::FaceToCamera:
				Q = Quaternion(Vector3(-MainCamera.Rot.X, MainCamera.Rot.Y, 0));
				break;
			case ParticleEmitterCPU::BillboardMode::None:
			default:
				Q = Quaternion(Vector3(0,0,0));
			}

			Vector4* PositionsNEW;
			Vector4* SizesNEW;
			Vector4* ColorsNEW;
			Vector2* OtherNEW;

			gDevice->MapBuffer(PositionsUAV, BufferMapAccess::Write, (void*&)PositionsNEW);
			gDevice->MapBuffer(SizesUAV, BufferMapAccess::Write, (void*&)SizesNEW);
			gDevice->MapBuffer(ColorsUAV, BufferMapAccess::Write, (void*&)ColorsNEW);
			gDevice->MapBuffer(OtherUAV, BufferMapAccess::Write, (void*&)OtherNEW);

			for (size_t i = 0; i < Particles.Particles.Count; i++)
			{
				PositionsNEW[i] = Vector4(Particles.Particles.Positions[i], 0);
				SizesNEW[i] = Vector4(Particles.Particles.Sizes[i], 0);
				ColorsNEW[i] = Particles.Particles.Colors[i];
				OtherNEW[i].X = Particles.Particles.Rotations[i];
				OtherNEW[i].Y = Particles.Particles.Frames[i];
			}

			gDevice->UnmapBuffer(PositionsUAV);
			gDevice->UnmapBuffer(SizesUAV);
			gDevice->UnmapBuffer(ColorsUAV);
			gDevice->UnmapBuffer(OtherUAV);

			gDevice->BindBufferBase(UBO, 0);
			gDevice->BindBufferBase(ColorsUAV, 1);
			gDevice->BindBufferBase(PositionsUAV, 2);

			if (Particles.Light)
			{
				gDevice->SetComputePipelineState(CPS);
				gDevice->Dispatch(Particles.Particles.Count, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			gDevice->SetShader(Shader);

			Matrix Billboard = Q.ToMatrix();

			Shader->SetUniform("View", false, MainCamera.GetViewMatrix());
			Shader->SetUniform("Projection", false, MainCamera.GetProjectionMatrix());
			Shader->SetUniform("Billboard", false, Billboard);
			Shader->SetUniform("Frame", iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));

			Shader->SetUniform("Texture", Mat.AlbedoMap != nullptr ? Mat.AlbedoMap : gDevice->GetDefaultTextures()->White.get(), 0);
			Shader->SetUniform("DepthTexture", Depth != nullptr ? Depth : gDevice->GetDefaultTextures()->Black.get(), 1);

			gDevice->IASetVertexBuffers(0, 0, nullptr);
			gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

			gDevice->BindBufferBase(PositionsUAV, 0);
			gDevice->BindBufferBase(SizesUAV, 1);
			gDevice->BindBufferBase(ColorsUAV, 2);
			gDevice->BindBufferBase(OtherUAV, 3);

			gDevice->Draw(Particles.Particles.Count * 6, 0);
		}
	}

	ParticlesRenderer::~ParticlesRenderer()
	{		
	}

}
