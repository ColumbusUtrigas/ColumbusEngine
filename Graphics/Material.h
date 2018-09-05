#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/Matrix.h>
#include <Graphics/Texture.h>
//#include <Graphics/Cubemap.h>
#include <Graphics/Shader.h>
#include <System/System.h>
#include <System/Log.h>
#include <System/Serializer.h>

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
		bool mLighting = true;

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
		Vector3 AmbientColor;

		float Roughness = 1.0f;
		float Metallic = 0.1f;

		float ReflectionPower = 0.2f;
		float EmissionStrength = 0.5f;
		float DetailNormalStrength = 0.3f;

		float Rim = 1.0f;
		float RimPower = 8.0f;
		float RimBias = 0.3f;
		Vector3 RimColor;
	public:
		Material();
		Material(std::string aFile);

		bool Prepare();
		void SetBool(std::string Name, bool Value);
		void SetInt(std::string Name, int Value);
		void SetFloat(std::string Name, float Value);
		void SetFloatArray(std::string Name, const float* Value, uint32 Size);
		void SetVector2(std::string Name, Vector2 Value);
		void SetVector3(std::string Name, Vector3 Value);
		void SetVector4(std::string Name, Vector4 Value);
		void SetMatrix(std::string Name, Matrix Value);
		void SetTexture(std::string Name, Texture* Value, uint32 Sampler);

		void SetShader(ShaderProgram* InShader);
		void setLighting(const bool aLighting);

		ShaderProgram* GetShader() const;
		bool getLighting() const;

		int getTextureID() const;
		int getNormMapID() const;
		int GetRoughnessMapID() const;
		int GetMetallicMapID() const;
		int GetOcclusionMapID() const;
		int GetEmissionMapID() const;
		int GetDetailDiffuseMapID() const;
		int GetDetailNormalMapID() const;

		bool saveToXML(std::string aFile) const;
		bool loadFromXML(std::string aFile);

		bool operator==(Material Other) const;
		bool operator!=(Material Other) const;

		~Material();
	};

}



