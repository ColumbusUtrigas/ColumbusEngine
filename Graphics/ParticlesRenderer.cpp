#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <Math/Quaternion.h>

namespace Columbus
{

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
				Q = Quaternion(Vector3(0, Math::Radians(MainCamera.Rot.Y), 0));
				break;
			case ParticleEmitterCPU::BillboardMode::Horizontal:
			case ParticleEmitterCPU::BillboardMode::FaceToCamera:
				Q = Quaternion(Vector3(Math::Radians(-MainCamera.Rot.X), Math::Radians(MainCamera.Rot.Y), 0));
				break;
			case ParticleEmitterCPU::BillboardMode::None:
			default:
				Q = Quaternion(Vector3(0,0,0));
			}

			Matrix Billboard = Q.ToMatrix();

			Shader->SetUniform("View", false, MainCamera.GetViewMatrix());
			Shader->SetUniform("Projection", false, MainCamera.GetProjectionMatrix());
			Shader->SetUniform("Billboard", false, Billboard);
			Shader->SetUniform("Frame", iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));

			Shader->SetUniform("Texture", Mat.AlbedoMap != nullptr ? Mat.AlbedoMap : gDevice->GetDefaultTextures()->White.get(), 0);
			Shader->SetUniform("DepthTexture", Depth != nullptr ? Depth : gDevice->GetDefaultTextures()->Black.get(), 1);

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
