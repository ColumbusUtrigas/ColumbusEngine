#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/ScreenQuad.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

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

	static void PrepareStates(BlendState*& BS, DepthStencilState*& DSS, RasterizerState*& RS)
	{
		BlendStateDesc bsd;
		DepthStencilStateDesc dssd;
		RasterizerStateDesc rsd;

		bsd.IndependentBlendEnable = false;
		bsd.RenderTarget[0].BlendEnable = false;

		dssd.DepthEnable = true;
		dssd.DepthFunc = ComparisonFunc::LEqual;
		dssd.DepthWriteMask = false;

		rsd.Cull = CullMode::Front;
		rsd.Fill = FillMode::Solid;

		gDevice->CreateBlendState(bsd, &BS);
		gDevice->CreateDepthStencilState(dssd, &DSS);
		gDevice->CreateRasterizerState(rsd, &RS);
	}

	static void CreateSkyboxBuffer(Buffer*& VertexBuffer, Buffer*& IndexBuffer, std::shared_ptr<InputLayout>& Layout)
	{
		BufferDesc vertexDesc;
		SubresourceData vertexData;
		vertexDesc.BindFlags = BufferType::Array;
		vertexDesc.CpuAccess = BufferCpuAccess::Write;
		vertexDesc.MiscFlags = 0;
		vertexDesc.Size = sizeof(Vertices);
		vertexDesc.StructureByteStride = sizeof(Vertices[0]) * 3;
		vertexDesc.Usage = BufferUsage::Static;
		vertexData.pSysMem = Vertices;
		gDevice->CreateBuffer(vertexDesc, &vertexData, &VertexBuffer);

		BufferDesc indexDesc;
		SubresourceData indexData;
		indexDesc.BindFlags = BufferType::Index;
		indexDesc.CpuAccess = BufferCpuAccess::Write;
		indexDesc.MiscFlags = 0;
		indexDesc.Size = sizeof(Indices);
		indexDesc.StructureByteStride = sizeof(Indices[0]);
		indexDesc.Usage = BufferUsage::Static;
		indexData.pSysMem = Indices;
		gDevice->CreateBuffer(indexDesc, &indexData, &IndexBuffer);
	
		Layout = std::make_shared<InputLayout>();
		Layout->NumElements = 1;
		Layout->Elements[0].Slot = 0;
		Layout->Elements[0].Components = 3;
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
		gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

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
		gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

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
			gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

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
		PrepareStates(BS, DSS, RS);
		CreateSkyboxBuffer(VertexBuffer, IndexBuffer, Layout);
	}

	Skybox::Skybox(Texture* InTexture)
	{
		PrepareMatrices();
		PrepareStates(BS, DSS, RS);
		CreateSkyboxBuffer(VertexBuffer, IndexBuffer, Layout);

		if (InTexture->GetType() == Texture::Type::Texture2D) CreateCubemap(InTexture, Tex, VertexBuffer, IndexBuffer, Layout);
		CreateIrradianceMap(Tex, IrradianceMap, VertexBuffer, IndexBuffer, Layout);
		CreatePrefilterMap(Tex, PrefilterMap, VertexBuffer, IndexBuffer, Layout);
	}

	void Skybox::Render()
	{
		if (Tex != nullptr)
		{
			gDevice->SetShader(gDevice->GetDefaultShaders()->Skybox.get());

			auto View = ViewCamera.GetViewMatrix();
			View.SetRow(3, Vector4(0, 0, 0, 1));
			View.SetColumn(3, Vector4(0, 0, 0, 1));

			auto ShaderOpenGL = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->Skybox.get());
			ShaderOpenGL->SetUniform("ViewProjection", false, View * ViewCamera.GetProjectionMatrix());
			ShaderOpenGL->SetUniform("Skybox", Tex, 0);

			gDevice->OMSetDepthStencilState(DSS, 0);
			gDevice->OMSetBlendState(BS, nullptr, RGBA_MASK(255, 255, 255, 255));
			gDevice->RSSetState(RS);

			gDevice->IASetInputLayout(Layout.get());
			gDevice->IASetVertexBuffers(0, 1, &VertexBuffer);
			gDevice->IASetIndexBuffer(IndexBuffer, IndexFormat::Uint16, 0);
			gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
			gDevice->DrawIndexed(36, 0,  0);
		}
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
