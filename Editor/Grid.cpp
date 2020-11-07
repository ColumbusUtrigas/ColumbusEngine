#include <Editor/Grid.h>
#include <Math/Vector3.h>
#include <GL/glew.h>
#include <vector>

namespace Columbus::Editor
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

		SubresourceData data;
		data.pSysMem = vertices.data();
		gDevice->CreateBuffer(BufferDesc(
			vertices.size() * sizeof(Vector3),
			BufferType::Array,
			BufferUsage::Static,
			BufferCpuAccess::Write),
		&data,
		&GridVertices);

		Layout.NumElements = 1;
		Layout.Elements[0] = InputLayoutElementDesc{ 0, 3 };

		glGenVertexArrays(1, &VAO);
	}

	void Grid::Draw()
	{
		glBindVertexArray(VAO);
		gDevice->IASetInputLayout(&Layout);
		gDevice->IASetVertexBuffers(0, 1, &GridVertices);
		gDevice->IASetPrimitiveTopology(PrimitiveTopology::LineList);
		gDevice->Draw(VerticesCount, 0);
	}

	Grid::~Grid()
	{
		glDeleteVertexArrays(1, &VAO);
	}

}
