/************************************************
*                   Model.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <vector>
#include <string>
#include <list>
#include <algorithm>

#include <System/Importer.h>
#include <RenderAPI/Buffer.h>

#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Material.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>

#include <Scene/Transform.h>
#include <Math/OBB.h>

namespace Columbus
{

	class Mesh
	{
	protected:
		Vector3 mPos;
		Mesh* mParent = nullptr;
		Camera mCamera;
		OBB mOBB;

		std::list<Mesh*> mChilds;
		std::vector<Light*> mLights;
	public:
		std::vector<Vertex> mVert;
		Material mMat;
		
		Mesh() { }
		Mesh(std::vector<Vertex> aVert) { setVertices(aVert); }
		Mesh(std::vector<Vertex> aVert, Material aMat) {  mMat = aMat; setVertices(aVert); }

		virtual void setVertices(std::vector<Vertex> aVert) { };
		virtual void render(Transform aTransform) { };

		void setCamera(Camera InCamera) { mCamera = InCamera; }
		void setParent(Mesh* InParent) { mParent = InParent; }
		void addChild(Mesh* InChild)
		{
			if (InChild == nullptr) return;

			mChilds.push_back(InChild);
			InChild->setParent(this);
		}

		void setLights(std::vector<Light*>& InLights) { mLights = InLights; }

		OBB getOBB() const { return mOBB; }

		void clear() {}

		virtual ~Mesh() {}
	};

}
