#include <Graphics/RenderState.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <GL/glew.h>

namespace Columbus
{

	void RenderState::SetCulling(Material::Cull Culling)
	{
		switch (Culling)
		{
		case Material::Cull::No:           glDisable(GL_CULL_FACE); break;
		case Material::Cull::Front:        glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT);          break;
		case Material::Cull::Back:         glEnable(GL_CULL_FACE);  glCullFace(GL_BACK);           break;
		case Material::Cull::FrontAndBack: glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT_AND_BACK); break;
		}
	}

	void RenderState::SetBlending(bool Blending)
	{
		if (Blending)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	void RenderState::SetDepthTesting(Material::DepthTest DepthTesting)
	{
		switch (DepthTesting)
		{
		case Material::DepthTest::Less:     glDepthFunc(GL_LESS);     break;
		case Material::DepthTest::Greater:  glDepthFunc(GL_GREATER);  break;
		case Material::DepthTest::LEqual:   glDepthFunc(GL_LEQUAL);   break;
		case Material::DepthTest::GEqual:   glDepthFunc(GL_GEQUAL);   break;
		case Material::DepthTest::Equal:    glDepthFunc(GL_EQUAL);    break;
		case Material::DepthTest::NotEqual: glDepthFunc(GL_NOTEQUAL); break;
		case Material::DepthTest::Never:    glDepthFunc(GL_NEVER);    break;
		case Material::DepthTest::Always:   glDepthFunc(GL_ALWAYS);   break;
		}
	}

	void RenderState::SetDepthWriting(bool DepthWriting)
	{
		glDepthMask(DepthWriting ? GL_TRUE : GL_FALSE);
	}

	void RenderState::SetMainCamera(const Camera& InMainCamera)
	{
		MainCamera = InMainCamera;
	}

	void RenderState::SetMaterial(const Material& InMaterial, const Matrix& ModelMatrix, Skybox* Sky, bool UsePixelShader)
	{
		auto CurrentShader = InMaterial.GetShader();
		if (CurrentShader != nullptr)
		{
			Texture* Textures[11] =
			{
				InMaterial.AlbedoMap,
				InMaterial.NormalMap,
				InMaterial.RoughnessMap,
				InMaterial.MetallicMap,
				InMaterial.OcclusionMap,
				InMaterial.EmissionMap,
				InMaterial.DetailAlbedoMap,
				InMaterial.DetailNormalMap,
				Sky ? Sky->GetIrradianceMap() : nullptr,
				Sky ? Sky->GetPrefilterMap() : nullptr,
				gDevice->GetDefaultTextures()->IntegrationLUT.get()
			};

			static std::string Names[11] =
			{ "AlbedoMap",
			  "NormalMap",
			  "RoughnessMap",
			  "MetallicMap",
			  "OcclusionMap",
			  "EmissionMap",
			  "DetailAlbedoMap",
			  "DetailNormalMap",
			  "IrradianceMap",
			  "EnvironmentMap",
			  "IntegrationMap" };

			ShaderProgramOpenGL* Shader = (ShaderProgramOpenGL*)CurrentShader;

			Shader->SetUniform("uModel", false, ModelMatrix);
			Shader->SetUniform("uViewProjection", false, MainCamera.GetViewProjection());

			if (UsePixelShader)
			{
				for (int32 i = 0; i < 11; i++)
				{
					Shader->SetUniform(Names[i], Textures[i], i);
				}

				Shader->SetUniform("HasAlbedoMap", InMaterial.AlbedoMap != nullptr);
				Shader->SetUniform("HasNormalMap", InMaterial.NormalMap != nullptr);
				Shader->SetUniform("HasRoughnessMap", InMaterial.RoughnessMap != nullptr);
				Shader->SetUniform("HasMetallicMap", InMaterial.MetallicMap != nullptr);
				Shader->SetUniform("HasOcclusionMap", InMaterial.OcclusionMap != nullptr);
				Shader->SetUniform("HasDetailAlbedoMap", InMaterial.DetailAlbedoMap != nullptr);
				Shader->SetUniform("HasDetailNormalMap", InMaterial.DetailNormalMap != nullptr);

				if (ShadowTexture != nullptr)
				{
					Shader->SetUniform("Shadow", (TextureOpenGL*)ShadowTexture, 14);
				}

				Shader->SetUniform("Tiling", InMaterial.Tiling);
				Shader->SetUniform("DetailTiling", InMaterial.DetailTiling);
				Shader->SetUniform("Albedo", InMaterial.Albedo);
				Shader->SetUniform("Roughness", InMaterial.Roughness);
				Shader->SetUniform("Metallic", InMaterial.Metallic);
				Shader->SetUniform("EmissionStrength", InMaterial.EmissionStrength);
				Shader->SetUniform("Transparent", InMaterial.Transparent);
				Shader->SetUniform("uCameraPosition", MainCamera.Pos);
			}
		}
	}

}


