#pragma once
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Texture.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Shader.h>
#include <System/System.h>

namespace C
{

	class C_Material
	{
	private:
		C_Vector4 color;
		C_Vector3 diffuse;
		C_Vector3 ambient;
		C_Vector3 specular;
		C_Texture* texture = NULL;
		C_Texture* specmap = NULL;
		C_Shader* shader = NULL;

		C_Cubemap* envRefl = NULL;

		float shininess = 32;
	public:
		C_Material();
		C_Material(const char* aFile);

		void setColor(const C_Vector4 aColor);

		void setDiffuse(const C_Vector3 aDiffuse);

		void setAmbient(const C_Vector3 aAmbient);

		void setSpecular(const C_Vector3 aSpecular);

		void setShininess(const float aShininess);

		void setTexture(const C_Texture* aTexture);

		void setSpecMap(const C_Texture* aSpecMap);

		void setShader(const C_Shader* aShader);

		void setReflection(const C_Cubemap* aReflection);

		C_Vector4 getColor();

		C_Vector3 getDiffuse();

		C_Vector3 getAmbient();

		C_Vector3 getSpecular();

		C_Texture* getTexture();

		C_Texture* getSpecMap();

		float getShininess();

		C_Shader* getShader();

		C_Cubemap* getReflection();

		void saveToFile(const char* aFile);

		void loadFromFile(const char* aFile);

		~C_Material();
	};

}
