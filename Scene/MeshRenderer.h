/************************************************
/************************************************
*                 MeshRenderer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Scene/Component.h>
#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Cubemap.h>

namespace Columbus
{

	class MeshRenderer : public Component
	{
	private:
		bool mCreateMoment = true;
		bool mUpdateMoment = false;

		Mesh* mMesh = nullptr;
		C_Camera mCamera;
	public:
		MeshRenderer(Mesh* aMesh);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		void setLights(std::vector<C_Light*> aLights);
		void setCamera(const C_Camera mCamera);
		Shader* getShader() const;
		void setShader(Shader* aShader);
		void setReflection(const Cubemap* aCubemap);

		~MeshRenderer();
	};

}



