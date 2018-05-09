#pragma once

#include <vector>
#include <string>
#include <list>
#include <algorithm>

#include <System/Importer.h>
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
		Vector3 Position;
		Mesh* Parent = nullptr;
		Camera ObjectCamera;
		OBB BoundingBox;

		std::list<Mesh*> Childs;
		std::vector<Light*> Lights;
	public:
		std::vector<Vertex> Vertices;
		Material mMat;
		
		Mesh() { }
		Mesh(std::vector<Vertex> InVertices) { SetVertices(InVertices); }
		Mesh(std::vector<Vertex> InVertices, Material InMaterial) {  mMat = InMaterial; SetVertices(InVertices); }

		virtual void SetVertices(std::vector<Vertex> InVertices) {}
		virtual void Render(Transform aTransform) {}

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }
		void SetParent(Mesh* InParent) { Parent = InParent; }
		void AddChild(Mesh* InChild)
		{
			if (InChild == nullptr) return;

			Childs.push_back(InChild);
			InChild->SetParent(this);
		}

		void SetLights(std::vector<Light*>& InLights) { Lights = InLights; }

		OBB GetOBB() const { return BoundingBox; }
		Material& GetMaterial() { return mMat; }

		void Clear() {}

		virtual ~Mesh() {}
	};

}
