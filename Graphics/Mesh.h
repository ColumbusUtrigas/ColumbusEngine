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
#include <Math/Box.h>

namespace Columbus
{

	class Mesh
	{
	protected:
		Vector3 Position;
		Camera ObjectCamera;
		Box BoundingBox;

		uint32 VerticesCount;
	public:
		Material mMat;
		std::vector<Light*> Lights;
	public:
		Mesh() : VerticesCount(0) { }
		Mesh(std::vector<Vertex> Vertices) : VerticesCount(0) { SetVertices(Vertices); }
		Mesh(std::vector<Vertex> Vertices, Material InMaterial) : VerticesCount(0) {  mMat = InMaterial; SetVertices(Vertices); }

		virtual void SetVertices(std::vector<Vertex> InVertices) {}
		virtual void Bind() {}
		virtual uint32 Render(Transform InTransform, ShaderProgram* InShader) { return 0; }
		virtual void Unbind() {}
		virtual uint64 GetMemoryUsage() const { return 0;  }

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }

		void SetLights(std::vector<Light*>& InLights) { Lights = InLights; }

		Box GetBoundingBox() const { return BoundingBox; }
		Material& GetMaterial() { return mMat; }
		Camera& GetCamera() { return ObjectCamera; }

		void Clear() {}

		virtual ~Mesh() {}
	};

}
