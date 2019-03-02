#pragma once

#include <vector>

#include <Graphics/Light.h>
#include <Common/Model/Model.h>
#include <Math/Box.h>

namespace Columbus
{

	class Mesh
	{
	protected:
		Vector3 Position;
		Box BoundingBox;

		uint32 VerticesCount;
		bool LightsSorted = false;
	public:
		std::vector<Light*> Lights;
	public:
		Mesh() : VerticesCount(0) {}

		bool Load(const char* FileName)
		{
			Model TmpModel;

			if (!TmpModel.Load(FileName))
			{
				return false;
			}

			return Load(TmpModel);
		}

		virtual void SetVertices(const std::vector<Vertex>& InVertices) = 0;
		virtual bool Load(const Model& InModel) = 0;
		virtual void Bind() = 0;
		virtual uint32 Render() = 0;
		virtual void Unbind() = 0;
		virtual uint64 GetMemoryUsage() const = 0;

		void SetLights(const std::vector<Light*>& InLights) { Lights = InLights; }

		Box GetBoundingBox() const { return BoundingBox; }

		void Clear() {}

		virtual ~Mesh() {};
	};

}


