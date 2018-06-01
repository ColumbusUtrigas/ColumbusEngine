#pragma once

#include <vector>
#include <string>
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

		std::map<uint32, Mesh*> Childs;
		std::vector<Light*> Lights;

		uint32 VerticesCount;
	public:
		//std::vector<Vertex> Vertices;
		Material mMat;
	public:
		Mesh() : VerticesCount(0) { }
		Mesh(std::vector<Vertex> Vertices) : VerticesCount(0) { SetVertices(Vertices); }
		Mesh(std::vector<Vertex> Vertices, Material InMaterial) : VerticesCount(0) {  mMat = InMaterial; SetVertices(Vertices); }

		virtual void SetVertices(std::vector<Vertex> InVertices) {}
		virtual void Render(Transform aTransform) {}

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }
		void SetParent(Mesh* InParent) { Parent = InParent; }
		void AddChild(uint32 ID, Mesh* InChild)
		{
			if (InChild != nullptr)
			{
				Childs[ID] = InChild;
				InChild->SetParent(this);
			}
		}

		void SetLights(std::vector<Light*>& InLights) { Lights = InLights; }

		OBB GetOBB() const { return BoundingBox; }
		Material& GetMaterial() { return mMat; }

		void Clear() {}

		virtual ~Mesh() {}
	};

}
