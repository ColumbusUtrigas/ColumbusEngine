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
	}
	
	void PostEffect::SetShader(ShaderProgram* InShader)
	{
		if (InShader != nullptr)
		{
			if (!InShader->IsCompiled())
			{
				InShader->Compile();
			}

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

		C_DrawScreenQuadOpenGL();

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

		/*C_DisableDepthTestOpenGL();
		C_DisableBlendOpenGL();
		C_DisableAlphaTestOpenGL();*/
	}

	PostEffect::~PostEffect()
	{

	}

}



