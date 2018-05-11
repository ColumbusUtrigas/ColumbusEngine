#include <Graphics/PostEffect.h>
#include <Graphics/Device.h>

namespace Columbus
{

	PostEffect::PostEffect()
	{
		mFB = gDevice->createFramebuffer();
		mTB = gDevice->createTexture(NULL, 640, 480, true);
		mDepth = gDevice->createTexture();
		mDepth->loadDepth(NULL, 640, 480, true);

		mFB->setTexture2D(E_FRAMEBUFFER_COLOR_ATTACH, mTB);
		mFB->setTexture2D(E_FRAMEBUFFER_DEPTH_ATTACH, mDepth);

		static float Vertices[] = 
		{
			-1.0, -1.0, 0.0,
			-1.0, 1.0, 0.0,
			1.0, 1.0, 0.0,
			1.0, -1.0, 0.0
		};

		static float UVs[] = 
		{
			0.0, 0.0,
			0.0, 1.0,
			1.0, 1.0,
			1.0, 0.0
		};

		static uint16 Indices[] = 
		{
			0, 2, 1,
			0, 3, 2
		};

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(UVs), nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(UVs), UVs);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(Vertices));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glBindVertexArray(0);
	}
	
	void PostEffect::SetShader(ShaderProgram* InShader)
	{
		if (InShader != nullptr)
		{
			InShader->AddAttribute("aPos", 0);
			InShader->AddAttribute("aUV", 1);
			InShader->Compile();

			InShader->AddUniform("uColor");
			InShader->AddUniform("uDepth");

			for (auto& Name : AttributeNames)
			{
				InShader->AddUniform(Name);
			}

			Shader = InShader;
		}
	}

	void PostEffect::AddAttributeName(std::string Name)
	{
		AttributeNames.push_back(Name);

		if (Shader != nullptr)
		{
			Shader->AddUniform(Name);
		}
	}
	
	void PostEffect::addAttrib(PostEffectAttributeInt aAttrib)
	{
		mAttribsInt.push_back(aAttrib);
	}
	
	void PostEffect::addAttrib(PostEffectAttributeFloat aAttrib)
	{
		mAttribsFloat.push_back(aAttrib);
	}
	
	void PostEffect::addAttrib(PostEffectAttributeVector2 aAttrib)
	{
		mAttribsVector2.push_back(aAttrib);
	}
	
	void PostEffect::addAttrib(PostEffectAttributeVector3 aAttrib)
	{
		mAttribsVector3.push_back(aAttrib);
	}
	
	void PostEffect::addAttrib(PostEffectAttributeVector4 aAttrib)
	{
		mAttribsVector4.push_back(aAttrib);
	}
	
	void PostEffect::clearAttribs()
	{
		mAttribsInt.clear();
		mAttribsFloat.clear();
		mAttribsVector2.clear();
		mAttribsVector3.clear();
		mAttribsVector4.clear();
	}

	void PostEffect::Bind(Vector4 ClearColor, Vector2 ContextSize)
	{
		mTB->load(NULL, Math::TruncToInt(ContextSize.X), Math::TruncToInt(ContextSize.Y), true);
		mDepth->loadDepth(NULL, Math::TruncToInt(ContextSize.X), Math::TruncToInt(ContextSize.Y), true);
		mFB->prepare(ClearColor, ContextSize);
	}

	void PostEffect::Render()
	{
		if (Shader == nullptr) return;

		mTB->generateMipmap();
		mDepth->generateMipmap();
		Shader->Bind();

		Shader->SetUniform1i("uColor", 0);
		mTB->sampler2D(0);

		Shader->SetUniform1i("uDepth", 1);
		mDepth->sampler2D(1);

		for (auto& Attrib : mAttribsInt)
			Shader->SetUniform1i(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsFloat)
			Shader->SetUniform1f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector2)
			Shader->SetUniform2f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector3)
			Shader->SetUniform3f(Attrib.name, Attrib.value);

		for (auto& Attrib : mAttribsVector4)
			Shader->SetUniform4f(Attrib.name, Attrib.value);

		glDepthMask(GL_FALSE);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);

		if (mTB) mTB->unbind();
		if (Shader) Shader->Unbind();
	}

	void PostEffect::Unbind()
	{
		for (int32 i = 0; i < 5; i++)
		{
			glDisableVertexAttribArray(i);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (mTB) mTB->unbind();
		if (Shader) Shader->Unbind();

		mFB->unbind();
	}

	PostEffect::~PostEffect()
	{

	}

}



