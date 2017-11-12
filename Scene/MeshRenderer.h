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
#include <Graphics/Model.h>

namespace Columbus
{

	class C_MeshRenderer : public C_Component
	{
	private:
		bool mCreateMoment = true;
		bool mUpdateMoment = false;

		C_Mesh* mMesh = nullptr;
	public:
		C_MeshRenderer(C_Mesh* aMesh);

		bool onCreate() override;
		bool onUpdate() override;

		void update() override;
		void render(C_Transform aTransform) override;

		~C_MeshRenderer();
	};

}



