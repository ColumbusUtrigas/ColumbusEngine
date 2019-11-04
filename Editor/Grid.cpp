#include <Editor/Grid.h>
#include <Math/Vector3.h>
#include <GL/glew.h>
#include <vector>

namespace Columbus
{

	Grid::Grid()
	{
		std::vector<Vector3> vertices;

		constexpr float min_pos = -500.0f;
		constexpr float max_pos = +500.0f;
		constexpr float step = 1.0f;

		for (float pos = min_pos; pos <= max_pos; pos += step)
		{
			vertices.emplace_back(min_pos, 0.0f, pos);
			vertices.emplace_back(max_pos, 0.0f, pos);

			vertices.emplace_back(pos, 0.0f, min_pos);
			vertices.emplace_back(pos, 0.0f, max_pos);
		}

		VerticesCount = vertices.size();

		GridVertices.CreateArray(BufferDesc(vertices.size() * sizeof(Vector3),
			BufferUsage::Write, BufferCpuAccess::Static));

		GridVertices.Load(vertices.data());

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		GridVertices.VertexAttribute<float>(0, 3, false, 0, 0);
		glBindVertexArray(0);
	}

	void Grid::Draw()
	{
		glDisable(GL_CULL_FACE);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, VerticesCount);
		glBindVertexArray(0);
		glEnable(GL_CULL_FACE);
	}

	Grid::~Grid()
	{
		glDeleteVertexArrays(1, &VAO);
	}

}


