#include <Graphics/Skybox.h>
#include <Graphics/Device.h>

namespace Columbus
{

	static ShaderProgram* CreateSkyboxShader()
	{
		ShaderProgram* tShader = gDevice->CreateShaderProgram();
		ShaderStage *vert, *frag;
		vert = gDevice->CreateShaderStage();
		frag = gDevice->CreateShaderStage();
		vert->Load("STANDART_SKY_VERTEX", ShaderType::Vertex);
		frag->Load("STANDART_SKY_FRAGMENT", ShaderType::Fragment);
		tShader->AddStage(vert);
		tShader->AddStage(frag);
		tShader->Compile();

		tShader->AddUniform("uView");
		tShader->AddUniform("uProjection");
		tShader->AddUniform("uSkybox");

		return tShader;
	}

	static void CreateSkyboxBuffer(uint32* VBO, uint32* VAO, float* Vertices)
	{
		glGenBuffers(1, VBO);
		glGenVertexArrays(1, VAO);

		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(float), Vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(*VAO);
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	Skybox::Skybox() :
		mCubemap(nullptr),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(&VBO, &VAO, skyboxVertices);
	}
	
	Skybox::Skybox(Cubemap* aCubemap) :
		mCubemap(aCubemap),
		Shader(nullptr)
	{
		Shader = CreateSkyboxShader();
		CreateSkyboxBuffer(&VBO, &VAO, skyboxVertices);
	}
	
	void Skybox::draw()
	{
		if (Shader != nullptr && mCubemap != nullptr)
		{
			glDepthMask(GL_FALSE);

			for (int32 i = 0; i < 5; i++)
			{
				glDisableVertexAttribArray(i);
			}

			Shader->Bind();

			auto view = mCamera.getViewMatrix();
			view.SetRow(3, Vector4(0, 0, 0, 1));
			view.SetColumn(3, Vector4(0, 0, 0, 1));

			view.Elements(UniformViewMatrix);
			mCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

			Shader->SetUniformMatrix("uView", UniformViewMatrix);
			Shader->SetUniformMatrix("uProjection", UniformProjectionMatrix);

			glActiveTexture(GL_TEXTURE0);
			Shader->SetUniform1i("uSkybox", 0);
			mCubemap->bind();

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			Shader->Unbind();
			mCubemap->unbind();

			glDepthMask(GL_TRUE);
		}
	}
	
	void Skybox::setCamera(const Camera aCamera)
	{
		mCamera = static_cast<Camera>(aCamera);
	}
	
	void Skybox::setCubemap(const Cubemap* aCubemap)
	{
		mCubemap = const_cast<Cubemap*>(aCubemap);
	}
	
	Cubemap* Skybox::getCubemap() const
	{
		return mCubemap;
	}
	
	Skybox::~Skybox()
	{
		delete Shader;
		glDeleteBuffers(1, &VBO);
	}

}



