#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/PostEffect.h>
#include <Graphics/ScreenQuad.h>
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

	ShaderProgram* IrradianceShader = nullptr;
	ShaderProgram* PrefilterShader = nullptr;
	ShaderProgram* IntegrationShader = nullptr;

	static ShaderProgram* CreateSkyboxShader()
	{
		ShaderProgram* tShader = gDevice->CreateShaderProgram();
		tShader->Load(ShaderProgram::StandartProgram::Skybox);
		tShader->Compile();

		tShader->AddUniform("uViewProjection");
		tShader->AddUniform("uSkybox");

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

	static void CreateIrradianceMap(Texture* BaseMap, Texture*& IrradianceMap, uint32 VAO, uint32 IBO)
	{
		Matrix CaptureProjection;
		Matrix CaptureViews[6];

		CaptureProjection.Perspective(90.0f, 1.0f, 0.1f, 10.0f);
		CaptureViews[0].LookAt({ 0, 0, 0 }, { +1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[1].LookAt({ 0, 0, 0 }, { -1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[2].LookAt({ 0, 0, 0 }, { 0, +1, 0 }, { 0,  0, +1 });
		CaptureViews[3].LookAt({ 0, 0, 0 }, { 0, -1, 0 }, { 0,  0, -1 });
		CaptureViews[4].LookAt({ 0, 0, 0 }, { 0, 0, +1 }, { 0, -1,  0 });
		CaptureViews[5].LookAt({ 0, 0, 0 }, { 0, 0, -1 }, { 0, -1,  0 });

		if (IrradianceShader == nullptr)
		{
			IrradianceShader = gDevice->CreateShaderProgram();
			IrradianceShader->Load("Data/Shaders/IrradianceGeneration.glsl");
			IrradianceShader->Compile();
		}

		if (IrradianceMap == nullptr)
		{
			IrradianceMap = gDevice->CreateTexture();
			IrradianceMap->CreateCube(Texture::Properties{ 32, 32, 0, TextureFormat::RGBA8 });
		}

		Framebuffer* Frame = gDevice->createFramebuffer();

		glDepthMask(GL_FALSE);

		IrradianceShader->Bind();
		IrradianceShader->SetUniformMatrix("Projection", &CaptureProjection.M[0][0]);
		IrradianceShader->SetUniform1i("EnvironmentMap", 0);
		glActiveTexture(GL_TEXTURE0);
		BaseMap->bind();

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, IrradianceMap, i);
			Frame->prepare({ 0, 0, 0, 0 }, { 32, 32 });
			IrradianceShader->SetUniformMatrix("View", &CaptureViews[i].M[0][0]);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		Frame->unbind();
		IrradianceShader->Unbind();
		glDepthMask(GL_TRUE);

		IrradianceMap->generateMipmap();

		delete Frame;
	}

	static void CreatePrefilterMap(Texture* BaseMap, Texture*& PrefilterMap, uint32 VAO, uint32 IBO)
	{
		Matrix CaptureProjection;
		Matrix CaptureViews[6];

		CaptureProjection.Perspective(90.0f, 1.0f, 0.1f, 10.0f);
		CaptureViews[0].LookAt({ 0, 0, 0 }, { +1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[1].LookAt({ 0, 0, 0 }, { -1, 0, 0 }, { 0, -1,  0 });
		CaptureViews[2].LookAt({ 0, 0, 0 }, { 0, +1, 0 }, { 0,  0, +1 });
		CaptureViews[3].LookAt({ 0, 0, 0 }, { 0, -1, 0 }, { 0,  0, -1 });
		CaptureViews[4].LookAt({ 0, 0, 0 }, { 0, 0, +1 }, { 0, -1,  0 });
		CaptureViews[5].LookAt({ 0, 0, 0 }, { 0, 0, -1 }, { 0, -1,  0 });

		if (PrefilterShader == nullptr)
		{
			PrefilterShader = gDevice->CreateShaderProgram();
			PrefilterShader->Load("Data/Shaders/PrefilterGeneration.glsl");
			PrefilterShader->Compile();
		}

		if (PrefilterMap == nullptr)
		{
			Texture::Flags Flags;
			Flags.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;
			Flags.Filtering = Texture::Filter::Trilinear;
			Flags.Wrapping = Texture::Wrap::ClampToEdge;

			PrefilterMap = gDevice->CreateTexture();
			PrefilterMap->CreateCube(Texture::Properties{ 128, 128, 0, TextureFormat::RGBA16F });
			PrefilterMap->generateMipmap();
			PrefilterMap->SetFlags(Flags);
			BaseMap->SetFlags(Flags);
		}

		Framebuffer* Frame = gDevice->createFramebuffer();

		glDepthMask(GL_FALSE);
		PrefilterShader->Bind();
		PrefilterShader->SetUniformMatrix("Projection", &CaptureProjection.M[0][0]);
		PrefilterShader->SetUniform1i("EnvironmentMap", 0);
		glActiveTexture(GL_TEXTURE0);
		BaseMap->bind();

		uint32 MaxMips = 8;

		for (uint32 Mip = 0; Mip < MaxMips; Mip++)
		{
			uint32 Width = 128 >> Mip;
			uint32 Height = 128 >> Mip;

			float Roughness = (float)Mip / (float)(MaxMips - 1);

			PrefilterShader->SetUniform1f("Roughness", Roughness);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

			for (uint32 i = 0; i < 6; i++)
			{
				Frame->SetTextureCube(Framebuffer::Attachment::Color0, PrefilterMap, i, Mip);
				Frame->prepare({ 0, 0, 0, 0 }, { (float)Width, (float)Height });
				PrefilterShader->SetUniformMatrix("View", &CaptureViews[i].M[0][0]);

				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		Frame->unbind();
		PrefilterShader->Unbind();
		glDepthMask(GL_TRUE);

		delete Frame;
	}

	static void CreateIntegrationMap(Texture*& IntegrationMap)
	{
		if (IntegrationShader == nullptr)
		{
			IntegrationShader = gDevice->CreateShaderProgram();
			IntegrationShader->Load("Data/Shaders/IntegrationGeneration.glsl");
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

		Frame.Bind({ 0 }, { 1, 1 });

		auto Tmp = Frame.ColorTextures[0];
		Frame.ColorTextures[0] = IntegrationMap;

		Frame.Bind({ 0 }, { 512, 512 });
		IntegrationShader->Bind();
		Quad.Render();
		IntegrationShader->Unbind();
		Frame.Unbind();

		Frame.ColorTextures[0] = Tmp;
	}

	Skybox::Skybox() :
		Tex(nullptr),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, IBO, VAO);
	}

	Skybox::Skybox(Texture* InTexture) :
		Tex(InTexture),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, IBO, VAO);
		CreateIrradianceMap(Tex, IrradianceMap, VAO, IBO);
		CreatePrefilterMap(Tex, PrefilterMap, VAO, IBO);
		CreateIntegrationMap(IntegrationMap);
	}

	void Skybox::Render()
	{
		if (Shader != nullptr && Tex != nullptr)
		{
			glDepthMask(GL_FALSE);

			Shader->Bind();

			auto View = ViewCamera.GetViewMatrix();
			View.SetRow(3, Vector4(0, 0, 0, 1));
			View.SetColumn(3, Vector4(0, 0, 0, 1));

			Shader->SetUniformMatrix("uViewProjection", &(View * ViewCamera.GetProjectionMatrix()).M[0][0]);

			glActiveTexture(GL_TEXTURE0);
			Shader->SetUniform1i("uSkybox", 0);
			Tex->bind();

			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			Shader->Unbind();
			Tex->unbind();

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


