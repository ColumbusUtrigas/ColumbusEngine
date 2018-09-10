#pragma once

#include <Graphics/Mesh.h>

namespace Columbus
{

	class MeshOpenGL : public Mesh
	{
	private:
		uint32 VBuf = 0;
		uint32 VAO = 0;

		uint64 VOffset = 0;
		uint64 UOffset = 0;
		uint64 NOffset = 0;
		uint64 TOffset = 0;

		void SortLights();
	public:
		MeshOpenGL();
		MeshOpenGL(std::vector<Vertex> Vertices);

		void SetVertices(std::vector<Vertex> Vertices) override;
		void Bind() override;
		uint32 Render() override;
		void Unbind() override;
		uint64 GetMemoryUsage() const override;

		~MeshOpenGL() override;
	};

}













