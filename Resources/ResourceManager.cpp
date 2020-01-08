#include <Resources/ResourceManager.h>
#include <Graphics/Device.h>
#include <Graphics/Material.h>

namespace Columbus
{

	std::shared_ptr<Texture> ResourceManager2<Texture>::CreateResource(const std::string& Name)
	{
		auto t = gDevice->CreateTexture();
		t->Load(Name.c_str());
		return std::shared_ptr<Texture>(t);
	}

	std::shared_ptr<ShaderProgram> ResourceManager2<ShaderProgram>::CreateResource(const std::string& Name)
	{
		auto s = gDevice->CreateShaderProgram();
		s->Load(Name.c_str());
		return std::shared_ptr<ShaderProgram>(s);
	}

	std::shared_ptr<Material> ResourceManager2<Material>::CreateResource(const std::string& Name)
	{
		return std::shared_ptr<Material>(new Material());
	}

	std::shared_ptr<Mesh> ResourceManager2<Mesh>::CreateResource(const std::string& Name)
	{
		auto m = gDevice->CreateMesh();
		m->Load(Name.c_str());
		return std::shared_ptr<Mesh>(m);
	}

}
