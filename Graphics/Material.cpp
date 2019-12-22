#include <Graphics/Material.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>

#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Resources/ResourceManager.h>

namespace Columbus
{

	Material::Material() {}

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
		_Properties = ShaderProg->GetProperties();
	}

	ShaderProgram* Material::GetShader() const
	{
		return ShaderProg;
	}

	bool Material::Load(const char* FileName,
		ResourceManager<ShaderProgram>& ShadersManager,
		ResourceManager<Texture>& TexturesManager)
	{
		JSON J;
		if (!J.Load(FileName)) return false;

		if (J["Culling"].GetString() == "No")           Culling = Cull::No;
		if (J["Culling"].GetString() == "Front")        Culling = Cull::Front;
		if (J["Culling"].GetString() == "Back")         Culling = Cull::Back;
		if (J["Culling"].GetString() == "FrontAndBack") Culling = Cull::FrontAndBack;
		
		if (J["DepthTesting"].GetString() == "Less")      DepthTesting = DepthTest::Less;
		if (J["DepthTesting"].GetString() == "Greater")   DepthTesting = DepthTest::Greater;
		if (J["DepthTesting"].GetString() == "LEqual")    DepthTesting = DepthTest::LEqual;
		if (J["DepthTesting"].GetString() == "GEqual")    DepthTesting = DepthTest::GEqual;
		if (J["DepthTesting"].GetString() == "Equal")     DepthTesting = DepthTest::Equal;
		if (J["DepthTesting"].GetString() == "NotEqual")  DepthTesting = DepthTest::NotEqual;
		if (J["DepthTesting"].GetString() == "Never")     DepthTesting = DepthTest::Never;
		if (J["DepthTesting"].GetString() == "Always")    DepthTesting = DepthTest::Always;

		DepthWriting = J["DepthWriting"].GetBool();
		Transparent  = J["Transparent"] .GetBool();
		Lighting     = J["Lighting"]    .GetBool();
		Tiling       = J["Tiling"]      .GetVector2<float>();
		DetailTiling = J["DetailTiling"].GetVector2<float>();
		Albedo       = J["Albedo"]      .GetVector4<float>();

		Roughness        = static_cast<float>(J["Roughness"]       .GetFloat());
		Metallic         = static_cast<float>(J["Metallic"]        .GetFloat());
		EmissionStrength = static_cast<float>(J["EmissionStrength"].GetFloat());

		SetShader(ShadersManager.Find(J["Shader"].IsString() ? J["Shader"].GetString() : ""));

		#define DESERIALIZE_TEXTURE(tex, name) \
			tex = J["Textures"][name].IsString() ? TexturesManager.Find(J["Textures"][name].GetString()) : nullptr;

		DESERIALIZE_TEXTURE(AlbedoMap, "Albedo");
		DESERIALIZE_TEXTURE(NormalMap, "Normal");
		DESERIALIZE_TEXTURE(RoughnessMap, "Roughness");
		DESERIALIZE_TEXTURE(MetallicMap, "Metallic");
		DESERIALIZE_TEXTURE(OcclusionMap, "Occlusion");
		DESERIALIZE_TEXTURE(EmissionMap, "Emission");
		DESERIALIZE_TEXTURE(DetailAlbedoMap, "DetailAlbedo");
		DESERIALIZE_TEXTURE(DetailNormalMap, "DetailNormal");

		Log::Success("Material loaded: %s", FileName);

		return true;
	}

	bool Material::Save(const char* FileName,
		ResourceManager<ShaderProgram>& ShadersManager,
		ResourceManager<Texture>& TexturesManager)
	{
		JSON J;

		switch (Culling)
		{
		case Cull::No:           J["Culling"] = "No";           break;
		case Cull::Front:        J["Culling"] = "Front";        break;
		case Cull::Back:         J["Culling"] = "Back";         break;
		case Cull::FrontAndBack: J["Culling"] = "FrontAndBack"; break;
		}

		switch (DepthTesting)
		{
		case DepthTest::Less:      J["DepthTesting"] = "Less";      break;
		case DepthTest::Greater:   J["DepthTesting"] = "Greater";   break;
		case DepthTest::LEqual:    J["DepthTesting"] = "LEqual";    break;
		case DepthTest::GEqual:    J["DepthTesting"] = "GEqual";    break;
		case DepthTest::Equal:     J["DepthTesting"] = "Equal";     break;
		case DepthTest::NotEqual:  J["DepthTesting"] = "NotEqual";  break;
		case DepthTest::Never:     J["DepthTesting"] = "Never";     break;
		case DepthTest::Always:    J["DepthTesting"] = "Always";    break;
		}

		J["DepthWriting"] = DepthWriting;
		J["Transparent"]  = Transparent;
		J["Lighting"]     = Lighting;
		J["Tiling"]       = Tiling;
		J["DetailTiling"] = DetailTiling;
		J["Albedo"]       = Albedo;

		J["Roughness"]        = Roughness;
		J["Metallic"]         = Metallic;
		J["EmissionStrength"] = EmissionStrength;

		if (ShaderProg)
			J["Shader"] = ShadersManager.Find(ShaderProg);
		else
			J["Shader"] = nullptr;

		#define SERIALIZE_TEXTURE(tex, name) \
			if (tex) \
				J["Textures"][name] = TexturesManager.Find(tex); \
			else \
				J["Textures"][name] = nullptr;

		SERIALIZE_TEXTURE(AlbedoMap, "Albedo");
		SERIALIZE_TEXTURE(NormalMap, "Normal");
		SERIALIZE_TEXTURE(RoughnessMap, "Roughness");
		SERIALIZE_TEXTURE(MetallicMap, "Metallic");
		SERIALIZE_TEXTURE(OcclusionMap, "Occlusion");
		SERIALIZE_TEXTURE(EmissionMap, "Emission");
		SERIALIZE_TEXTURE(DetailAlbedoMap, "DetailAlbedo");
		SERIALIZE_TEXTURE(DetailNormalMap, "DetailNormal");

		if (J.Save(FileName))
			Log::Success("Material saved: %s", FileName);
		else
			return false;

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


