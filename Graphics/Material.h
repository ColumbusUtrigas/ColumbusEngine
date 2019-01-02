#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>

namespace Columbus
{

	class Material
	{
	public:
		enum class Cull
		{
			No,
			Front,
			Back,
			FrontAndBack
		};
	private:
		int mTextureID = -1;
		int mNormMapID = -1;
		int RoughnessMapID = -1;
		int MetallicMapID = -1;
		int OcclusionMapID = -1;
		int EmissionMapID = -1;
		int DetailDiffuseMapID = -1;
		int DetailNormalMapID = -1;
	protected:
		ShaderProgram* ShaderProg = nullptr;
	public:
		Cull Culling = Cull::Back;
		bool DepthWriting = true;
		bool Transparent = false;
		bool Lighting = true;

		Texture* DiffuseTexture = nullptr;
		Texture* NormalTexture = nullptr;
		Texture* RoughnessTexture = nullptr;
		Texture* MetallicTexture = nullptr;
		Texture* OcclusionMap = nullptr;
		Texture* EmissionMap = nullptr;
		Texture* DetailDiffuseMap = nullptr;
		Texture* DetailNormalMap = nullptr;
		Texture* Reflection = nullptr;

		Vector2 Tiling = Vector2(1, 1);
		Vector2 DetailTiling = Vector2(4, 4);

		Vector4 Color;

		float Roughness = 1.0f;
		float Metallic = 0.1f;
		float EmissionStrength = 0.5f;
	public:
		Material();
		Material(std::string aFile);

		bool Prepare();

		void SetShader(ShaderProgram* InShader);
		ShaderProgram* GetShader() const;

		int getTextureID() const;
		int getNormMapID() const;
		int GetRoughnessMapID() const;
		int GetMetallicMapID() const;
		int GetOcclusionMapID() const;
		int GetEmissionMapID() const;
		int GetDetailDiffuseMapID() const;
		int GetDetailNormalMapID() const;

		bool loadFromXML(const std::string& aFile);

		bool operator==(const Material& Other) const;
		bool operator!=(const Material& Other) const;

		~Material();
	};

}



