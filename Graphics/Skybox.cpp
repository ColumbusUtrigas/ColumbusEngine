#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

#include <Graphics/Framebuffer.h>
#include <GL/glew.h>

namespace Columbus
{

	static float SkyboxVertices[108] =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	ShaderProgram* IrradianceShader = nullptr;

	static ShaderProgram* CreateSkyboxShader()
	{
		ShaderProgram* tShader = gDevice->CreateShaderProgram();
		tShader->Load(ShaderProgram::StandartProgram::Skybox);
		tShader->Compile();

		tShader->AddUniform("uViewProjection");
		tShader->AddUniform("uSkybox");

		return tShader;
	}

	static void CreateSkyboxBuffer(uint32& VBO, uint32& VAO)
	{
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), SkyboxVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	static void CreateIrradianceMap(Texture* BaseMap, Texture*& IrradianceMap, uint32 VAO)
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

		for (int i = 0; i < 6; i++)
		{
			Frame->SetTextureCube(Framebuffer::Attachment::Color0, IrradianceMap, i);
			Frame->prepare({ 0, 0, 0, 0 }, { 32, 32 });
			IrradianceShader->SetUniformMatrix("View", &CaptureViews[i].M[0][0]);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		Frame->unbind();
		IrradianceShader->Unbind();
		glDepthMask(GL_TRUE);

		IrradianceMap->generateMipmap();

		delete Frame;
	}

	Skybox::Skybox() :
		Tex(nullptr),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, VAO);
	}
	
	Skybox::Skybox(Texture* InTexture) :
		Tex(InTexture),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(VBO, VAO);
		CreateIrradianceMap(Tex, IrradianceMap, VAO);
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
			glDrawArrays(GL_TRIANGLES, 0, 36);
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
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

}


