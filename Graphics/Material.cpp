#include <Graphics/Material.h>
#include <System/Serializer.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>

namespace Columbus
{

	Material::Material() {}
	Material::Material(const char* FileName)
	{
		Load(FileName);
	}

	bool Material::Prepare()
	{
		/*if (ShaderProg != nullptr)
		{
			if (!ShaderProg->IsCompiled())
			{
				ShaderProg->Compile();
			}

			ShaderProg->Bind();

			return true;
		}*/

		return false;
	}
	
	void Material::SetShader(ShaderProgram* InShader)
	{
		ShaderProg = InShader;
	}

	ShaderProgram* Material::GetShader() const
	{
		return ShaderProg;
	}
	
	int Material::GetAlbedoMapID() const
	{
		return AlbedoMapID;
	}

	int Material::GetNormalMapID() const
	{
		return NormalMapID;
	}

	int Material::GetRoughnessMapID() const
	{
		return RoughnessMapID;
	}

	int Material::GetMetallicMapID() const
	{
		return MetallicMapID;
	}

	int Material::GetOcclusionMapID() const
	{
		return OcclusionMapID;
	}

	int Material::GetEmissionMapID() const
	{
		return EmissionMapID;
	}

	int Material::GetDetailAlbedoMapID() const
	{
		return DetailAlbedoMapID;
	}

	int Material::GetDetailNormalMapID() const
	{
		return DetailNormalMapID;
	}

	bool Material::Load(const char* FileName)
	{
		JSON J;
		if (!J.Load(FileName)) return false;

		if (J["Culling"].GetString() == "No")           Culling = Cull::No;
		if (J["Culling"].GetString() == "Front")        Culling = Cull::Front;
		if (J["Culling"].GetString() == "Back")         Culling = Cull::Back;
		if (J["Culling"].GetString() == "FrontAndBack") Culling = Cull::FrontAndBack;
		
		if (J["DepthTesting"].GetString() == "Less")    DepthTesting = DepthTest::Less;
		if (J["DepthTesting"].GetString() == "Greater") DepthTesting = DepthTest::Greater;
		if (J["DepthTesting"].GetString() == "LEqual")  DepthTesting = DepthTest::LEqual;
		if (J["DepthTesting"].GetString() == "GEqual")  DepthTesting = DepthTest::GEqual;
		if (J["DepthTesting"].GetString() == "Never")   DepthTesting = DepthTest::Never;
		if (J["DepthTesting"].GetString() == "Always")  DepthTesting = DepthTest::Always;

		DepthWriting = J["DepthWriting"].GetBool();
		Transparent  = J["Transparent"] .GetBool();
		Lighting     = J["Lighting"]    .GetBool();
		Tiling       = J["Tiling"]      .GetVector2<float>();
		DetailTiling = J["DetailTiling"].GetVector2<float>();
		Albedo       = J["Albedo"]      .GetVector4<float>();

		Roughness        = (float)J["Roughness"]       .GetFloat();
		Metallic         = (float)J["Metallic"]        .GetFloat();
		EmissionStrength = (float)J["EmissionStrength"].GetFloat();

		AlbedoMapID       = J["Textures"]["Albedo"]      .IsInt() ? (int)J["Textures"]["Albedo"]      .GetInt() : -1;
		NormalMapID       = J["Textures"]["Normal"]      .IsInt() ? (int)J["Textures"]["Normal"]      .GetInt() : -1;
		RoughnessMapID    = J["Textures"]["Roughness"]   .IsInt() ? (int)J["Textures"]["Roughness"]   .GetInt() : -1;
		MetallicMapID     = J["Textures"]["Metallic"]    .IsInt() ? (int)J["Textures"]["Metallic"]    .GetInt() : -1;
		OcclusionMapID    = J["Textures"]["Occlusion"]   .IsInt() ? (int)J["Textures"]["Occlusion"]   .GetInt() : -1;
		EmissionMapID     = J["Textures"]["Emission"]    .IsInt() ? (int)J["Textures"]["Emission"]    .GetInt() : -1;
		DetailAlbedoMapID = J["Textures"]["DetailAlbedo"].IsInt() ? (int)J["Textures"]["DetailAlbedo"].GetInt() : -1;
		DetailNormalMapID = J["Textures"]["DetailNormal"].IsInt() ? (int)J["Textures"]["DetailNormal"].GetInt() : -1;

		Log::Success("Material loaded: %s", FileName);

		return true;
	}

	bool Material::operator==(const Material& Other) const
	{
		return (Albedo == Other.Albedo &&
		        AlbedoMap == Other.AlbedoMap &&
		        NormalMap == Other.NormalMap &&
		        RoughnessMap == Other.RoughnessMap &&
		        MetallicMap == Other.MetallicMap &&
		        OcclusionMap == Other.OcclusionMap &&
		        EmissionMap == Other.EmissionMap &&
		        DetailAlbedoMap == Other.DetailAlbedoMap &&
		        DetailNormalMap == Other.DetailNormalMap &&
		        ReflectionMap == Other.ReflectionMap &&
		        ShaderProg == Other.ShaderProg &&
		        Roughness == Other.Roughness &&
		        Metallic == Other.Metallic &&
		        EmissionStrength == Other.EmissionStrength &&
		        Lighting == Other.Lighting);
	}

	bool Material::operator!=(const Material& Other) const
	{
		return !(*this == Other);
	}

	Material::~Material()
	{

	}

}


