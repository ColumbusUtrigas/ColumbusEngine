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

		enum class DepthTest
		{
			Less,
			Greater,
			LEqual,
			GEqual,
			Equal,
			NotEqual,
			Never,
			Always
		};
	private:
		int AlbedoMapID = -1;
		int NormalMapID = -1;
		int RoughnessMapID = -1;
		int MetallicMapID = -1;
		int OcclusionMapID = -1;
		int EmissionMapID = -1;
		int DetailAlbedoMapID = -1;
		int DetailNormalMapID = -1;
	public:
		Cull Culling = Cull::Back;
		DepthTest DepthTesting = DepthTest::LEqual;
		bool DepthWriting = true;
		bool Transparent = false;
		bool Lighting = true;

		ShaderProgram* ShaderProg = nullptr;

		Texture* AlbedoMap = nullptr;
		Texture* NormalMap = nullptr;
		Texture* RoughnessMap = nullptr;
		Texture* MetallicMap = nullptr;
		Texture* OcclusionMap = nullptr;
		Texture* EmissionMap = nullptr;
		Texture* DetailAlbedoMap = nullptr;
		Texture* DetailNormalMap = nullptr;
		Texture* ReflectionMap = nullptr;

		Vector2 Tiling = Vector2(1, 1);
		Vector2 DetailTiling = Vector2(4, 4);

		Vector4 Albedo;

		float Roughness = 1.0f;
		float Metallic = 0.1f;
		float EmissionStrength = 0.5f;
	public:
		Material();
		Material(const char* FileName);

		bool Prepare();

		void SetShader(ShaderProgram* InShader);
		ShaderProgram* GetShader() const;

		int GetAlbedoMapID() const;
		int GetNormalMapID() const;
		int GetRoughnessMapID() const;
		int GetMetallicMapID() const;
		int GetOcclusionMapID() const;
		int GetEmissionMapID() const;
		int GetDetailAlbedoMapID() const;
		int GetDetailNormalMapID() const;

		bool Load(const char* FileName);

		bool operator==(const Material& Other) const;
		bool operator!=(const Material& Other) const;

		~Material();
	};

}



