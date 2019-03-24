#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/PostEffect.h>
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

	static unsigned char Indices[36] = 
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

	ShaderProgramOpenGL* SkyboxCubemapGenerationShader = nullptr;
	ShaderProgramOpenGL* IrradianceShader = nullptr;
	ShaderProgramOpenGL* PrefilterShader = nullptr;
	ShaderProgramOpenGL* IntegrationShader = nullptr;

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

	static ShaderProgram* CreateSkyboxShader()
	{
		ShaderProgram* tShader = gDevice->CreateShaderProgram();
		tShader->Load(ShaderProgram::StandartProgram::Skybox);
		tShader->Compile();

		return tShader;
	}

	static void CreateSkyboxBuffer(uint32& VBO, uint32& IBO, uint32& VAO)
	{
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	static void CreateCubemap(Texture* BaseMap, Texture*& Cubemap, uint32 VAO, uint32 IBO)
	{
		Cubemap = new TextureOpenGL();
		Cubemap->CreateCube(Texture::Properties(2048, 2048, 0, TextureFormat::RGB16F));
		Cubemap->SetFlags(Texture::Flags(Texture::Filter::Trilinear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Repeat));

		if (SkyboxCubemapGenerationShader == nullptr)
		{
			SkyboxCubemapGenerationShader = new ShaderProgramOpenGL();
			SkyboxCubemapGenerationShader->Load(ShaderProgram::StandartProgram::SkyboxCubemapGeneration);
			SkyboxCubemapGenerationShader->Compile();
		}

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		glDepthMask(GL_FALSE);

		SkyboxCubemapGenerationShader->Bind();
		SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("Projection"), false, CaptureProjection);
		SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("BaseMap"), (TextureOpenGL*)BaseMap, 0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, Cubemap, i);
			Frame->Prepare({ 0, 0, 0, 0 }, { 0, 0 }, { 2048, 2048 });
			SkyboxCubemapGenerationShader->SetUniform(SkyboxCubemapGenerationShader->GetFastUniform("View"), false, CaptureViews[i]);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		Frame->Unbind();
		SkyboxCubemapGenerationShader->Unbind();
		glDepthMask(GL_TRUE);

		Cubemap->GenerateMipmap();

		delete Frame;
	}

	static void CreateIrradianceMap(Texture* BaseMap, Texture*& IrradianceMap, uint32 VAO, uint32 IBO)
	{
		if (IrradianceShader == nullptr)
		{
			IrradianceShader = (ShaderProgramOpenGL*)gDevice->CreateShaderProgram();
			IrradianceShader->Load(ShaderProgram::StandartProgram::IrradianceGeneration);
			IrradianceShader->Compile();
		}

		if (IrradianceMap == nullptr)
		{
			IrradianceMap = gDevice->CreateTexture();
			IrradianceMap->CreateCube(Texture::Properties{ 32, 32, 0, TextureFormat::RGB16F });
			IrradianceMap->SetFlags(Texture::Flags(Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Repeat));
		}

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		glDepthMask(GL_FALSE);

		IrradianceShader->Bind();
		IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("Projection"), false, CaptureProjection);
		IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("EnvironmentMap"), (TextureOpenGL*)BaseMap, 0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, IrradianceMap, i);
			Frame->Prepare({ 0, 0, 0, 0 }, { 0 }, { 32, 32 });
			IrradianceShader->SetUniform(IrradianceShader->GetFastUniform("View"), false, CaptureViews[i]);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		Frame->Unbind();
		IrradianceShader->Unbind();
		glDepthMask(GL_TRUE);

		delete Frame;
	}

	static void CreatePrefilterMap(Texture* BaseMap, Texture*& PrefilterMap, uint32 VAO, uint32 IBO)
	{
		if (PrefilterShader == nullptr)
		{
			PrefilterShader = (ShaderProgramOpenGL*)gDevice->CreateShaderProgram();
			PrefilterShader->Load(ShaderProgram::StandartProgram::PrefilterGeneration);
			PrefilterShader->Compile();
		}

		if (PrefilterMap == nullptr)
		{
			Texture::Flags Flags;
			Flags.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;
			Flags.Filtering = Texture::Filter::Trilinear;
			Flags.Wrapping = Texture::Wrap::ClampToEdge;

			PrefilterMap = gDevice->CreateTexture();
			PrefilterMap->CreateCube(Texture::Properties{ 128, 128, 0, TextureFormat::RGB16F });
			PrefilterMap->GenerateMipmap();
			PrefilterMap->SetFlags(Flags);
		}

		Framebuffer* Frame = gDevice->CreateFramebuffer();

		glDepthMask(GL_FALSE);
		PrefilterShader->Bind();
		PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("Projection"), false, CaptureProjection);
		PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("EnvironmentMap"), (TextureOpenGL*)BaseMap, 0);

		uint32 MaxMips = 8;
		uint32 Resolution = 128;

		for (uint32 Mip = 0; Mip < MaxMips; Mip++)
		{
			uint32 Width  = Resolution >> Mip;
			uint32 Height = Resolution >> Mip;

			float Roughness = (float)Mip / (float)(MaxMips - 1);

			PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("Roughness"), Roughness);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

			for (uint32 i = 0; i < 6; i++)
			{
				Frame->SetTextureCube(Framebuffer::Attachment::Color0, PrefilterMap, i, Mip);
				Frame->Prepare({ 0, 0, 0, 0 }, { 0, 0 }, { (int)Width, (int)Height });
				PrefilterShader->SetUniform(PrefilterShader->GetFastUniform("View"), false, CaptureViews[i]);

				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		Frame->Unbind();
		PrefilterShader->Unbind();
		glDepthMask(GL_TRUE);

		delete Frame;
	}

	static void CreateIntegrationMap(Texture*& IntegrationMap)
	{
		if (IntegrationShader == nullptr)
		{
			IntegrationShader = (ShaderProgramOpenGL*)gDevice->CreateShaderProgram();
			IntegrationShader->Load(ShaderProgram::StandartProgram::IntegrationGeneration);
			IntegrationShader->Compile();
		}

		if (IntegrationMap == nullptr)
		{
			Texture::Flags Flags;
			Flags.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;
			Flags.Filtering = Texture::Filter::Linear;
			Flags.Wrapping = Texture::Wrap::ClampToEdge;

			IntegrationMap = gDevice->CreateTexture();
			IntegrationMap->Create2D(Texture::Properties{ 512, 512, 0, TextureFormat::RG16F });
			IntegrationMap->SetFlags(Flags);
		}

		PostEffect Frame;
		ScreenQuad Quad;
		Frame.ColorTexturesEnablement[0] = true;
		Frame.ColorTexturesFormats[0] = TextureFormat::RG16F;

		Frame.Bind({ 0 }, { 0, 0 }, { 1, 1 });

		auto Tmp = Frame.ColorTextures[0];
		Frame.ColorTextures[0] = IntegrationMap;

		Frame.Bind({ 0 }, { 0, 0}, { 512, 512 });
		IntegrationShader->Bind();
		Quad.Render();
		IntegrationShader->Unbind();
		Frame.Unbind();

		Frame.ColorTextures[0] = Tmp;
	}

	Skybox::Skybox()
	{
		PrepareMatrices();
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, IBO, VAO);
	}

	Skybox::Skybox(Texture* InTexture)
	{
		//Tex = InTexture;
		PrepareMatrices();
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, IBO, VAO);
		if (InTexture->GetType() == Texture::Type::Texture2D) CreateCubemap(InTexture, Tex, VAO, IBO);
		CreateIrradianceMap(Tex, IrradianceMap, VAO, IBO);
		CreatePrefilterMap(Tex, PrefilterMap, VAO, IBO);
		CreateIntegrationMap(IntegrationMap);
	}

	void Skybox::Render()
	{
		if (Shader != nullptr && Tex != nullptr)
		{
			auto ShaderOpenGL = (ShaderProgramOpenGL*)Shader;

			glDepthMask(GL_FALSE);

			ShaderOpenGL->Bind();

			auto View = ViewCamera.GetViewMatrix();
			View.SetRow(3, Vector4(0, 0, 0, 1));
			View.SetColumn(3, Vector4(0, 0, 0, 1));

			ShaderOpenGL->SetUniform(ShaderOpenGL->GetFastUniform("ViewProjection"), false, View * ViewCamera.GetProjectionMatrix());
			ShaderOpenGL->SetUniform(ShaderOpenGL->GetFastUniform("Skybox"), (TextureOpenGL*)Tex, 0);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			ShaderOpenGL->Unbind();
			((TextureOpenGL*)Tex)->Unbind();

			glDepthMask(GL_TRUE);
		}
	}

	void Skybox::SetCamera(const Camera& Cam)
	{
		ViewCamera = Cam;
	}

	Skybox::~Skybox()
	{
		delete Shader;
		delete Tex;
		delete IrradianceMap;
		delete PrefilterMap;
		delete IntegrationMap;
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
		glDeleteVertexArrays(1, &VAO);
	}

}


