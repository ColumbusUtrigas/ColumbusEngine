#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class ShaderProgram;
	class Texture;

	template <typename T>
	struct ResourceManager;

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

		Vector4 Albedo = Vector4(1);

		float Roughness = 1.0f;
		float Metallic = 0.1f;
		float EmissionStrength = 0.5f;
	public:
		Material();

		bool Prepare();

		void SetShader(ShaderProgram* InShader);
		ShaderProgram* GetShader() const;

		bool Load(const char* FileName,
			ResourceManager<ShaderProgram>& ShadersManager,
			ResourceManager<Texture>& TexturesManager);

		bool operator==(const Material& Other) const;
		bool operator!=(const Material& Other) const;

		~Material();
	};

}


