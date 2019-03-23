#pragma once

#include <Graphics/Device.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
#include <GL/glew.h>

namespace Columbus
{

	struct PostEffect
	{
	private:
		iVector2 PreviousSize;
		bool SizeChanged = false;
	public:
		static constexpr int TexturesCount = 4;
		/*static constexpr Framebuffer::Attachment Attachments[TexturesCount] =
		{ Framebuffer::Attachment::Color0,
		  Framebuffer::Attachment::Color1,
		  Framebuffer::Attachment::Color2,
		  Framebuffer::Attachment::Color3 };*/

		Framebuffer* FB = nullptr;
		Texture* DepthTexture = nullptr;

		bool DepthTextureEnablement = false;
		bool DepthTextureMipmaps = false;

		Texture* ColorTextures[TexturesCount];
		bool ColorTexturesEnablement[TexturesCount];
		bool ColorTexturesMipmaps[TexturesCount];
		TextureFormat ColorTexturesFormats[TexturesCount];
	public:
		PostEffect()
		{
			FB = gDevice->createFramebuffer();
			DepthTexture = nullptr;

			for (int i = 0; i < TexturesCount; i++)
			{
				ColorTextures[i] = nullptr;
				ColorTexturesEnablement[i] = false;
				ColorTexturesMipmaps[i] = false;
				ColorTexturesFormats[i] = TextureFormat::RGBA8;
			}
		}

		void Bind(const Vector4& Color, const iVector2& Size)
		{
			static Framebuffer::Attachment Attachments[TexturesCount] =
			{ Framebuffer::Attachment::Color0,
			  Framebuffer::Attachment::Color1,
			  Framebuffer::Attachment::Color2,
			  Framebuffer::Attachment::Color3 };

			Texture::Flags Flags { Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1 };

			if (Size != PreviousSize)
			{
				for (int i = 0; i < TexturesCount; i++)
				{
					if (ColorTexturesEnablement[i])
					{
						if (ColorTextures[i] == nullptr)
						{
							ColorTextures[i] = gDevice->CreateTexture();
						}

						ColorTextures[i]->Create2D(Texture::Properties(Size.X, Size.Y, 0, ColorTexturesFormats[i]));
						ColorTextures[i]->SetFlags(Flags);
						FB->setTexture2D(Attachments[i], ColorTextures[i]);
					}
				}

				if (DepthTextureEnablement)
				{
					if (DepthTexture == nullptr)
					{
						DepthTexture = gDevice->CreateTexture();
					}

					DepthTexture->Create2D(Texture::Properties(Size.X, Size.Y, 0, TextureFormat::Depth24));
					DepthTexture->SetFlags(Flags);
					FB->setTexture2D(Framebuffer::Attachment::Depth, DepthTexture);
				}

				SizeChanged = true;
			}

			PreviousSize = Size;

			FB->prepare(Color, PreviousSize);

			GLenum DrawBuffers[TexturesCount];
			int DrawBuffersNum = 0;

			for (int i = 0; i < TexturesCount; i++)
			{
				if (ColorTexturesEnablement[i])
				{
					DrawBuffers[DrawBuffersNum] = GL_COLOR_ATTACHMENT0 + i;
					DrawBuffersNum++;
				}
			}

			glDrawBuffers(DrawBuffersNum, DrawBuffers);
		}

		void Unbind()
		{
			if (SizeChanged || true)
			{
				for (int i = 0; i < TexturesCount; i++)
				{
					if (ColorTexturesEnablement[i] && ColorTexturesMipmaps[i])
					{
						ColorTextures[i]->generateMipmap();
					}
				}

				if (DepthTextureEnablement && DepthTextureMipmaps)
				{
					DepthTexture->generateMipmap();
				}
			}

			SizeChanged = false;

			FB->unbind();
		}

		~PostEffect()
		{
			delete FB;
			delete DepthTexture;

			for (int i = 0; i < TexturesCount; i++) delete ColorTextures[i];
		}
	};

}



