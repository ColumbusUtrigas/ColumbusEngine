#include <Graphics/Material.h>

namespace C
{

	C_Material::C_Material()
	{
		color = C_Vector4(1, 1, 1, 1);
		diffuse = C_Vector3(1, 1, 1);
		ambient = C_Vector3(0.25, 0.25, 0.25);
		specular = C_Vector3(1, 1, 1);
		shininess = 32;
	}

	C_Material::C_Material(const char* aFile)
	{
		loadFromFile(aFile);
	}

	void C_Material::setColor(const C_Vector4 aColor)
	{
		color = (C_Vector4)aColor;
	}

	void C_Material::setDiffuse(const C_Vector3 aDiffuse)
	{
		diffuse = (C_Vector3)aDiffuse;
	}

	void C_Material::setAmbient(const C_Vector3 aAmbient)
	{
		ambient = (C_Vector3)aAmbient;
	}

	void C_Material::setSpecular(const C_Vector3 aSpecular)
	{
		specular = (C_Vector3)aSpecular;
	}

	void C_Material::setShininess(const float aShininess)
	{
		shininess = (float)aShininess;
	}

	void C_Material::setTexture(const C_Texture* aTexture)
	{
		texture = (C_Texture*)aTexture;
	}

	void C_Material::setSpecMap(const C_Texture* aSpecMap)
	{
		specmap = (C_Texture*)aSpecMap;
	}

	void C_Material::setShader(const C_Shader* aShader)
	{
		shader = (C_Shader*)aShader;
	}

	void C_Material::setReflection(const C_Cubemap* aReflecction)
	{
		envRefl = (C_Cubemap*)aReflecction;
	}

	C_Vector4 C_Material::getColor()
	{
		return color;
	}

	C_Vector3 C_Material::getDiffuse()
	{
		return diffuse;
	}

	C_Vector3 C_Material::getAmbient()
	{
		return ambient;
	}

	C_Vector3 C_Material::getSpecular()
	{
		return specular;
	}

	float C_Material::getShininess()
	{
		return shininess;
	}

	C_Texture* C_Material::getTexture()
	{
		return texture;
	}

	C_Texture* C_Material::getSpecMap()
	{
		return specmap;
	}

	C_Shader* C_Material::getShader()
	{
		return shader;
	}
	
	C_Cubemap* C_Material::getReflection()
	{
		return envRefl;
	}

	void C_Material::saveToFile(const char* aFile)
	{
		C_XMLDoc doc;
		C_XMLNode* root = doc.NewElement("Material");
		doc.InsertFirstChild(root);

		C_XMLElement* tmp = doc.NewElement("Color");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't save Material color: %s\n", aFile);
			return;
		}
		tmp->SetAttribute("R", color.x);
		tmp->SetAttribute("G", color.y);
		tmp->SetAttribute("B", color.z);
		tmp->SetAttribute("A", color.w);
		root->InsertEndChild(tmp);

		tmp = NULL;

		tmp = doc.NewElement("Diffuse");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't save Material diffuse: %s\n", aFile);
			return;
		}
		tmp->SetAttribute("R", diffuse.x);
		tmp->SetAttribute("G", diffuse.y);
		tmp->SetAttribute("B", diffuse.z);
		root->InsertEndChild(tmp);

		tmp = NULL;

		tmp = doc.NewElement("Ambient");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't save Material ambient: %s\n", aFile);
			return;
		}
		tmp->SetAttribute("R", ambient.x);
		tmp->SetAttribute("G", ambient.y);
		tmp->SetAttribute("B", ambient.z);
		root->InsertEndChild(tmp);

		tmp = NULL;

		tmp = doc.NewElement("Specular");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't save Material specular: %s\n", aFile);
			return;
		}
		tmp->SetAttribute("R", specular.x);
		tmp->SetAttribute("G", specular.y);
		tmp->SetAttribute("B", specular.z);
		root->InsertEndChild(tmp);

		tmp = NULL;

		tmp = doc.NewElement("Shininess");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't save Material shininess: %s\n", aFile);
			return;
		}
		tmp->SetText(shininess);
		root->InsertEndChild(tmp);

		if(doc.SaveFile(aFile) != C_XML_SUCCESS)
		{
			printf("Error: Can't save Material: %s\n", aFile);
			return;
		}

		printf("Material successfuly saved: %s\n", aFile);
	}

	void C_Material::loadFromFile(const char* aFile)
	{
		C_XMLDoc doc;
		if(doc.LoadFile(aFile) != C_XML_SUCCESS)
		{
			printf("Error: Can't load Material: %s\n", aFile);
			return;
		}

		C_XMLElement* root = doc.FirstChildElement("Material");
		if(root == NULL && root == nullptr)
		{
			printf("Error: Can't load Material: %s\n", aFile);
			return;
		}

		C_XMLElement* tmp = root->FirstChildElement("Color");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't load Material Color: %s\n", aFile);
			return;
		}

		tmp->QueryFloatAttribute("R", &color.x);
		tmp->QueryFloatAttribute("G", &color.y);
		tmp->QueryFloatAttribute("B", &color.z);
		tmp->QueryFloatAttribute("A", &color.w);

		tmp = NULL;

		tmp = root->FirstChildElement("Ambient");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't load Material Ambient: %s\n", aFile);
			return;
		}

		tmp->QueryFloatAttribute("R", &ambient.x);
		tmp->QueryFloatAttribute("G", &ambient.y);
		tmp->QueryFloatAttribute("B", &ambient.z);

		tmp = NULL;

		tmp = root->FirstChildElement("Diffuse");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't load Material Diffuse: %s\n", aFile);
			return;
		}

		tmp->QueryFloatAttribute("R", &diffuse.x);
		tmp->QueryFloatAttribute("G", &diffuse.y);
		tmp->QueryFloatAttribute("B", &diffuse.z);

		tmp = NULL;

		tmp = root->FirstChildElement("Specular");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't load Material Specular: %s\n", aFile);
			return;
		}

		tmp->QueryFloatAttribute("R", &specular.x);
		tmp->QueryFloatAttribute("G", &specular.y);
		tmp->QueryFloatAttribute("B", &specular.z);

		tmp = NULL;

		tmp = root->FirstChildElement("Shininess");
		if(tmp == NULL && tmp == nullptr)
		{
			printf("Error: Can't load Material Shininess: %s\n", aFile);
			return;
		}

		tmp->QueryFloatText(&shininess);

		tmp = NULL;

		printf("Material successfuly loaded: %s\n", aFile);
	}

	C_Material::~C_Material()
	{

	}

}
