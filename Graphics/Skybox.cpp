#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/ScreenQuad.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

#include <Graphics/ShaderCompiler.h>

namespace Columbus
{

	static float Vertices[24] = 
	{
		-1, -1, -1,
		-1, -1, +1,
		-1, +1, -1,
		-1, +1, +1,
		+1, -1, -1,
		+1, -1, +1,
		+1, +1, -1,
		+1, +1, +1
	};

	static uint16 Indices[36] = 
	{
		2, 0, 4, 4, 6, 2,
		1, 0, 2, 2, 3, 1,
		4, 5, 7, 7, 6, 4,
		1, 3, 7, 7, 5, 1,
		2, 6, 7, 7, 3, 2,
		0, 1, 4, 4, 1, 5
	};

	Matrix CaptureProjection;
	Matrix CaptureViews[6];
	GraphicsPipeline* SkyboxPipeline;

	const char* SkyboxShaderCode =
R"(
#shader vertex VS
#input(float3, pos, POSITION, 0)
#output(float4, pos, SV_POSITION, 0)
#output(float3, wpos, TEXCOORD0, 1)

begin_cbv(Params)
	float4x4 viewproj;
end_cbv

VS_OUTPUT VS(VS_INPUT i)
{
	VS_OUTPUT o;
	o.pos = mul(viewproj, float4(i.pos, 1));
	o.wpos = i.pos;
	return o;
}

#shader pixel PS
#input(float4, pos, SV_POSITION, 0)
#input(float3, wpos, TEXCOORD0, 1)
#output(float4, rt0, SV_TARGET, 0)

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

PS_OUTPUT PS(PS_INPUT i)
{
    PS_OUTPUT o;
    //o.rt0 = float4(i.wpos.xyz, 1);

	float4 col = t1.Sample(s1, i.wpos.xy);
	o.rt0 = col;

    return o;
}
)";

	static void PrepareMatrices()
	{
		CaptureProjection.Perspective(90.0f, 1.0f, 0.1f, 10.0f);
		CaptureViews[0].LookAt({ 0, 0, 0 }, { +1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[1].LookAt({ 0, 0, 0 }, { -1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[2].LookAt({ 0, 0, 0 }, { 0, +1, 0 }, { 0,  0, +1 });
		CaptureViews[3].LookAt({ 0, 0, 0 }, { 0, -1, 0 }, { 0,  0, -1 });
		CaptureViews[4].LookAt({ 0, 0, 0 }, { 0, 0, +1 }, { 0, -1,  0 });
		CaptureViews[5].LookAt({ 0, 0, 0 }, { 0, 0, -1 }, { 0, -1,  0 });
	}

	static void PrepareGraphicsPipeline()
	{
		GraphicsPipelineDesc desc;

		// TODO
		// desc.layout.Elements = {
		// 	InputLayoutElementDesc{ "POSITION", 0, 0, 3 }
		// };
		// desc.textures = {
		// };
		// desc.cbs = {
		// 	ShaderResourceDesc{ "Params", 0 }
		// };

		desc.blendState.IndependentBlendEnable = false;
		// desc.blendState.RenderTarget[0].BlendEnable = false;

		desc.depthStencilState.DepthEnable = true;
		desc.depthStencilState.DepthFunc = ComparisonFunc::LEqual;
		desc.depthStencilState.DepthWriteMask = false;

		desc.rasterizerState.Cull = CullMode::Front;
		desc.rasterizerState.Fill = FillMode::Solid;
		desc.topology = PrimitiveTopology::TriangleList;

		auto prog = Graphics::ShaderCompiler::Compile(SkyboxShaderCode, gDevice->GetCslBackendLang(), {});
		desc.VS = prog.VS;
		desc.PS = prog.PS;

		gDevice->CreateGraphicsPipeline(desc, &SkyboxPipeline);
	}

	static void CreateSkyboxBuffer(SPtr<Buffer>& vbuf, SPtr<Buffer>& ibuf, SPtr<Buffer>& cbuf)
	{
		{
			BufferDesc vertexDesc;
			vertexDesc.BindFlags = BufferType::Array;
			vertexDesc.CpuAccess = BufferCpuAccess::Write;
			vertexDesc.MiscFlags = 0;
			vertexDesc.Size = sizeof(Vertices);
			vertexDesc.StructureByteStride = sizeof(Vertices[0]) * 3;
			vertexDesc.Usage = BufferUsage::Static;

			SubresourceData vertexData;
			vertexData.pSysMem = Vertices;
			vertexData.SysMemPitch = sizeof(Vertices);

			vbuf = gDevice->CreateBufferShared(vertexDesc, &vertexData);
		}

		{
			BufferDesc indexDesc;
			indexDesc.BindFlags = BufferType::Index;
			indexDesc.CpuAccess = BufferCpuAccess::Write;
			indexDesc.MiscFlags = 0;
			indexDesc.Size = sizeof(Indices);
			indexDesc.StructureByteStride = sizeof(Indices[0]);
			indexDesc.Usage = BufferUsage::Static;

			SubresourceData indexData;
			indexData.pSysMem = Indices;
			indexData.SysMemPitch = sizeof(Indices);

			ibuf = gDevice->CreateBufferShared(indexDesc, &indexData);
		}

		{
			BufferDesc desc;
			desc.BindFlags = BufferType::Constant;
			desc.Usage = BufferUsage::Static;
			desc.CpuAccess = BufferCpuAccess::Write;
			desc.Size = sizeof(Matrix);

			cbuf = gDevice->CreateBufferShared(desc, nullptr);
		}
	}

	static void CreateCubemap(Texture* BaseMap, Texture*& Cubemap, Buffer* VertexBuffer, Buffer* IndexBuffer, std::shared_ptr<InputLayout> Layout)
	{
		auto SkyboxCubemapGenerationShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->SkyboxCubemapGeneration.get();

		iVector2 Resolution(1024);

		BaseMap->SetFlags(Texture::Flags(Texture::Filter::Trilinear, Texture::Anisotropy::Anisotropy16, Texture::Wrap::Clamp));

		Cubemap = gDevice->CreateTexture();
		Cubemap->CreateCube(TextureDesc(Resolution.X, Resolution.Y, 0, 0, TextureFormat::R11G11B10F));
		Cubemap->SetFlags(Texture::Flags(Texture::Filter::Trilinear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Repeat));

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		SkyboxCubemapGenerationShader->Bind();
		SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("Projection"), false, CaptureProjection);
		SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("BaseMap"), (TextureOpenGL*)BaseMap, 0);

		gDevice->IASetInputLayout(Layout.get());
		gDevice->IASetVertexBuffers(0, 1, &VertexBuffer);
		gDevice->IASetIndexBuffer(IndexBuffer, IndexFormat::Uint16, 0);
		COLUMBUS_ASSERT_MESSAGE(false, "TODO");
		//gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, Cubemap, i);
			Frame->Prepare({ 0, 0, 0, 0 }, { 0, 0 }, Resolution);
			SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("View"), false, CaptureViews[i]);

			gDevice->DrawIndexed(36, 0, 0);
		}

		Frame->Unbind();
		SkyboxCubemapGenerationShader->Unbind();

		Cubemap->GenerateMipmap();

		delete Frame;
	}

	static void CreateIrradianceMap(Texture* BaseMap, Texture*& IrradianceMap, Buffer* VertexBuffer, Buffer* IndexBuffer, std::shared_ptr<InputLayout> Layout)
	{
		auto IrradianceShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->IrradianceGeneration.get();

		if (IrradianceMap == nullptr)
		{
			IrradianceMap = gDevice->CreateTexture();
			IrradianceMap->CreateCube(TextureDesc{ 32, 32, 0, 0, TextureFormat::RGB16F });
			IrradianceMap->SetFlags(Texture::Flags(Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Repeat));
		}

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		glDepthMask(GL_FALSE);

		IrradianceShader->Bind();
		IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("Projection"), false, CaptureProjection);
		IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("EnvironmentMap"), (TextureOpenGL*)BaseMap, 0);

		gDevice->IASetInputLayout(Layout.get());
		gDevice->IASetVertexBuffers(0, 1, &VertexBuffer);
		gDevice->IASetIndexBuffer(IndexBuffer, IndexFormat::Uint16, 0);
		COLUMBUS_ASSERT_MESSAGE(false, "TODO");
		//gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, IrradianceMap, i);
			Frame->Prepare({ 0, 0, 0, 0 }, { 0 }, { 32, 32 });
			IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("View"), false, CaptureViews[i]);

			gDevice->DrawIndexed(36, 0, 0);
		}

		Frame->Unbind();
		IrradianceShader->Unbind();
		glDepthMask(GL_TRUE);

		delete Frame;
	}

	static void CreatePrefilterMap(Texture* BaseMap, Texture*& PrefilterMap, Buffer* VertexBuffer, Buffer* IndexBuffer, std::shared_ptr<InputLayout> Layout)
	{
		auto PrefilterShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->PrefilterGeneration.get();

		uint32 MaxMips = 8;
		uint32 Resolution = 256;

		if (PrefilterMap == nullptr)
		{
			Texture::Flags Flags;
			Flags.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;
			Flags.Filtering = Texture::Filter::Trilinear;
			Flags.Wrapping = Texture::Wrap::Clamp;

			PrefilterMap = gDevice->CreateTexture();
			PrefilterMap->CreateCube(TextureDesc{ Resolution, Resolution, 0, 0, TextureFormat::RGB16F });
			PrefilterMap->GenerateMipmap();
			PrefilterMap->SetFlags(Flags);
		}

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		glDepthMask(GL_FALSE);
		PrefilterShader->Bind();
		PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("Projection"), false, CaptureProjection);
		PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("EnvironmentMap"), (TextureOpenGL*)BaseMap, 0);

		for (uint32 Mip = 0; Mip < MaxMips; Mip++)
		{
			uint32 Width  = Resolution >> Mip;
			uint32 Height = Resolution >> Mip;

			float Roughness = (float)Mip / (float)(MaxMips - 1);

			PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("Roughness"), Roughness);

			gDevice->IASetInputLayout(Layout.get());
			gDevice->IASetVertexBuffers(0, 1, &VertexBuffer);
			gDevice->IASetIndexBuffer(IndexBuffer, IndexFormat::Uint16, 0);
			COLUMBUS_ASSERT_MESSAGE(false, "TODO");
			//gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

			for (uint32 i = 0; i < 6; i++)
			{
				Frame->SetTextureCube(Framebuffer::Attachment::Color0, PrefilterMap, i, Mip);
				Frame->Prepare({ 0, 0, 0, 0 }, { 0, 0 }, { (int)Width, (int)Height });
				PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("View"), false, CaptureViews[i]);

				gDevice->DrawIndexed(36, 0, 0);
			}
		}

		Frame->Unbind();
		PrefilterShader->Unbind();
		glDepthMask(GL_TRUE);

		delete Frame;
	}

	Skybox::Skybox()
	{
		PrepareMatrices();
		CreateSkyboxBuffer(vbuffer, ibuffer, cbuffer);
		PrepareGraphicsPipeline();
	}

	Skybox::Skybox(SPtr<Texture> hdri) : Skybox()
	{
		if (hdri->GetType() == Texture::Type::Texture2D)
		{

		}
	}

	Skybox::Skybox(Texture* InTexture) : Skybox()
	{
		//PrepareMatrices();
		//CreateSkyboxBuffer(vbuffer, ibuffer, cbuffer);
		//PrepareGraphicsPipeline();

		//if (InTexture->GetType() == Texture::Type::Texture2D) CreateCubemap(InTexture, Tex, VertexBuffer, IndexBuffer, Layout);
		//CreateIrradianceMap(Tex, IrradianceMap, VertexBuffer, IndexBuffer, Layout);
		//CreatePrefilterMap(Tex, PrefilterMap, VertexBuffer, IndexBuffer, Layout);
	}

	void Skybox::Render()
	{
		auto view = ViewCamera.GetViewMatrix();
		view.SetRow(3, Vector4(0, 0, 0, 1));
		view.SetColumn(3, Vector4(0, 0, 0, 1));

		gDevice->BeginMarker("Skybox");

		void* map;
		gDevice->MapBuffer(cbuffer.get(), BufferMapAccess::Write, map);
		((Matrix*)map)[0] = view * ViewCamera.GetProjectionMatrix();
		gDevice->UnmapBuffer(cbuffer.get());

		Buffer* vbufs[] = { vbuffer.get() };
		gDevice->SetGraphicsPipeline(SkyboxPipeline);
		gDevice->IASetVertexBuffers(0, 1, vbufs);
		gDevice->IASetIndexBuffer(ibuffer.get(), IndexFormat::Uint16, 0);
		gDevice->SetGraphicsCBV(0, cbuffer.get());
		gDevice->DrawIndexed(36, 0, 0);

		gDevice->EndMarker();
	}

	void Skybox::SetCamera(const Camera& Cam)
	{
		ViewCamera = Cam;
	}

	Skybox::~Skybox()
	{
		delete Tex;
		delete IrradianceMap;
		delete PrefilterMap;
	}

}
