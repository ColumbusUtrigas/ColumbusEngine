#include <Graphics/PostEffect.h>
#include <Graphics/Device.h>

namespace Columbus
{

	PostEffect::PostEffect()
	{
		FB = gDevice->createFramebuffer();
		ColorTexture = gDevice->CreateTexture();
		NormalTexture = gDevice->CreateTexture();
		DepthTexture = gDevice->CreateTexture();

		ColorTexture->Create2D(Texture::Properties(100, 100, 0, TextureFormat::RGB8));
		NormalTexture->Create2D(Texture::Properties(100, 100, 0, TextureFormat::RGB8));
		DepthTexture->Create2D(Texture::Properties(100, 100, 0, TextureFormat::Depth16));

		FB->setTexture2D(Framebuffer::Attachment::Color0, ColorTexture);
		FB->setTexture2D(Framebuffer::Attachment::Color1, NormalTexture);
		FB->setTexture2D(Framebuffer::Attachment::Depth, DepthTexture);

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
			InShader->AddUniform("uNormal");
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
		if (ContextSize != PreviousSize)
		{
			int32 W, H;
			W = Math::TruncToInt(ContextSize.X);
			H = Math::TruncToInt(ContextSize.Y);

			ColorTexture->Create2D(Texture::Properties(W, H, 0, TextureFormat::RGBA8));
			NormalTexture->Create2D(Texture::Properties(W, H, 0, TextureFormat::RGB8));
			DepthTexture->Create2D(Texture::Properties(W, H, 0, TextureFormat::Depth16));

			FB->setTexture2D(Framebuffer::Attachment::Color0, ColorTexture);
			FB->setTexture2D(Framebuffer::Attachment::Color1, NormalTexture);
			FB->setTexture2D(Framebuffer::Attachment::Depth, DepthTexture);

			SizeChanged = true;
		}

		PreviousSize = ContextSize;

		FB->prepare(ClearColor, ContextSize);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);
	}

	void PostEffect::Render()
	{
		if (Shader == nullptr) return;

		if (SizeChanged)
		{
			ColorTexture->generateMipmap();
			DepthTexture->generateMipmap();
			NormalTexture->generateMipmap();
		}

		SizeChanged = false;

		Shader->Bind();

		glActiveTexture(GL_TEXTURE0);
		Shader->SetUniform1i("uColor", 0);
		ColorTexture->bind();

		glActiveTexture(GL_TEXTURE1);
		Shader->SetUniform1i("uNormal", 1);
		NormalTexture->bind();

		glActiveTexture(GL_TEXTURE2);
		Shader->SetUniform1i("uDepth", 2);
		DepthTexture->bind();

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
	}

	void PostEffect::Unbind()
	{
		FB->unbind();
	}

	PostEffect::~PostEffect()
	{
		delete ColorTexture;
		delete NormalTexture;
		delete DepthTexture;
		delete FB;
	}

}



