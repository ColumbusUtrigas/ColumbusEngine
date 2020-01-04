#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <Math/Quaternion.h>

namespace Columbus
{

	void ParticlesRenderer::Allocate(size_t NewSize)
	{
		MaxSize = NewSize;

		auto Array = BufferType::Array;
		auto Dynamic = BufferUsage::Dynamic;
		auto Write = BufferCpuAccess::Write;

		size_t VerticesCount = MaxSize * 6;

		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector3), Array, Dynamic, Write), nullptr, &PositionsBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector3), Array, Dynamic, Write), nullptr, &SizesBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector4), Array, Dynamic, Write), nullptr, &ColorsBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector2), Array, Dynamic, Write), nullptr, &OtherDataBuffer);
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
			Quaternion Q(Vector3(Math::Radians(-MainCamera.Rot.X), Math::Radians(MainCamera.Rot.Y), 0));
			Matrix Billboard = Q.ToMatrix();

			Shader->SetUniform("View", false, MainCamera.GetViewMatrix());
			Shader->SetUniform("Projection", false, MainCamera.GetProjectionMatrix());
			Shader->SetUniform("Billboard", false, Billboard);
			Shader->SetUniform("Frame", iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));

			if (Mat.AlbedoMap == nullptr)
				Shader->SetUniform("Texture", gDevice->GetDefaultTextures()->White.get(), 0);
			else
				Shader->SetUniform("Texture", Mat.AlbedoMap, 0);

			Vector3* Positions;
			Vector3* Sizes;
			Vector4* Colors;
			Vector2* OtherData;

			gDevice->MapBuffer(PositionsBuffer, BufferMapAccess::Write, (void*&)Positions);
			gDevice->MapBuffer(SizesBuffer,     BufferMapAccess::Write, (void*&)Sizes);
			gDevice->MapBuffer(ColorsBuffer,    BufferMapAccess::Write, (void*&)Colors);
			gDevice->MapBuffer(OtherDataBuffer, BufferMapAccess::Write, (void*&)OtherData);

			for (size_t i = 0; i < Particles.Particles.Count; i++)
			{
				for (size_t a = 0; a < 6; a++)
				{
					Positions[i * 6 + a]   = Particles.Particles.Positions[i].XYZ();
					    Sizes[i * 6 + a]   = Particles.Particles.Sizes[i];
					   Colors[i * 6 + a]   = Particles.Particles.Colors[i];
					OtherData[i * 6 + a].X = Particles.Particles.Rotations[i];
					OtherData[i * 6 + a].Y = Particles.Particles.Frames[i];
				}
			}

			gDevice->UnmapBuffer(PositionsBuffer);
			gDevice->UnmapBuffer(SizesBuffer);
			gDevice->UnmapBuffer(ColorsBuffer);
			gDevice->UnmapBuffer(OtherDataBuffer);

			InputLayout layout;
			layout.NumElements = 4;
			layout.Elements[0] = InputLayoutElementDesc{ 0, 3 };
			layout.Elements[1] = InputLayoutElementDesc{ 1, 3 };
			layout.Elements[2] = InputLayoutElementDesc{ 2, 4 };
			layout.Elements[3] = InputLayoutElementDesc{ 3, 2 };

			Buffer* ppBuffers[] = { PositionsBuffer, SizesBuffer, ColorsBuffer, OtherDataBuffer };
			gDevice->IASetInputLayout(&layout);
			gDevice->IASetVertexBuffers(0, 4, ppBuffers);
			gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
			gDevice->Draw(Particles.Particles.Count * 6, 0);
		}
	}

	ParticlesRenderer::~ParticlesRenderer()
	{		
	}

}
