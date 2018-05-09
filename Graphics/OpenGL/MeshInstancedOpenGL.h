#pragma once

#include <Graphics/MeshInstanced.h>

namespace Columbus
{

	class MeshInstancedOpenGL : public MeshInstanced
	{
	private:
		uint32 VBO = 0;
		uint32 VAO = 0;

		uint64 VOffset = 0;
		uint64 UOffset = 0;
		uint64 NOffset = 0;
		uint64 TOffset = 0;

		float LightUniform[120];
		float MaterialUnif[15];
		float UniformModelMatrix[16];
		float UniformViewMatrix[16];
		float UniformProjectionMatrix[16];
	public:
		MeshInstancedOpenGL();

		void SetVertices(std::vector<Vertex> InVertices) override;
		void Render() override;

		~MeshInstancedOpenGL() override;
	};

}





