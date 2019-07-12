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

		ShaderProg = ShadersManager.Find(J["Shader"].IsString() ? J["Shader"].GetString() : "");

		AlbedoMap       = TexturesManager.Find(J["Textures"]["Albedo"]      .IsString() ? J["Textures"]["Albedo"]      .GetString() : "");
		NormalMap       = TexturesManager.Find(J["Textures"]["Normal"]      .IsString() ? J["Textures"]["Normal"]      .GetString() : "");
		RoughnessMap    = TexturesManager.Find(J["Textures"]["Roughness"]   .IsString() ? J["Textures"]["Roughness"]   .GetString() : "");
		MetallicMap     = TexturesManager.Find(J["Textures"]["Metallic"]    .IsString() ? J["Textures"]["Metallic"]    .GetString() : "");
		OcclusionMap    = TexturesManager.Find(J["Textures"]["Occlusion"]   .IsString() ? J["Textures"]["Occlusion"]   .GetString() : "");
		EmissionMap     = TexturesManager.Find(J["Textures"]["Emission"]    .IsString() ? J["Textures"]["Emission"]    .GetString() : "");
		DetailAlbedoMap = TexturesManager.Find(J["Textures"]["DetailAlbedo"].IsString() ? J["Textures"]["DetailAlbedo"].GetString() : "");
		DetailNormalMap = TexturesManager.Find(J["Textures"]["DetailNormal"].IsString() ? J["Textures"]["DetailNormal"].GetString() : "");

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


