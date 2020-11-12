#include <Graphics/Material.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>

#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Resources/ResourceManager.h>

namespace Columbus
{

	Material::Material()
	{
		DepthStencilState* dss;
		DepthStencilStateDesc dssd;
		dssd.DepthEnable = true;
		dssd.DepthWriteMask = true;
		dssd.DepthFunc = ComparisonFunc::LEqual;
		gDevice->CreateDepthStencilState(dssd, &dss);

		BlendState* bs;
		BlendStateDesc bsd;
		gDevice->CreateBlendState(bsd, &bs);

		RasterizerState* rs;
		RasterizerStateDesc rsd;
		rsd.Cull = CullMode::Back;
		rsd.Fill = FillMode::Solid;
		rsd.FrontCounterClockwise = true;
		gDevice->CreateRasterizerState(rsd, &rs);

		DSS = std::shared_ptr<DepthStencilState>(dss);
		BS = std::shared_ptr<BlendState>(bs);
		RS = std::shared_ptr<RasterizerState>(rs);
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
		if (!InShader)
			return;

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

		if (J["EnvMapMode"].GetString() == "None") EnvMapMode = EnvMap::None;
		if (J["EnvMapMode"].GetString() == "Sky")  EnvMapMode = EnvMap::Sky;
		if (J["EnvMapMode"].GetString() == "Auto") EnvMapMode = EnvMap::Auto;

		DepthWriting = (bool)J["DepthWriting"];
		Transparent  = (bool)J["Transparent"];
		Lighting     = (bool)J["Lighting"];
		Tiling       = J["Tiling"];
		DetailTiling = J["DetailTiling"];
		Albedo       = J["Albedo"];

		Roughness        = static_cast<float>(J["Roughness"]);
		Metallic         = static_cast<float>(J["Metallic"]);
		EmissionStrength = static_cast<float>(J["EmissionStrength"]);

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

	Vector2 operator+(const Vector2& v1, const Vector2& v2)
	{
		Vector2 ans;
		return ans;
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

		switch (EnvMapMode)
		{
		case EnvMap::None: J["EnvMapMode"] = "None"; break;
		case EnvMap::Sky:  J["EnvMapMode"] = "Sky";  break;
		case EnvMap::Auto: J["EnvMapMode"] = "Auto"; break;
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


