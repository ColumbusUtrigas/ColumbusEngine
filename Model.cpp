#include <Model.h>

namespace C
{

	C_Mesh::C_Mesh(std::vector<C_Vertex> aVert)
	{
		mPos = C_Vector3(0, 0, 0);
		mRot = C_Vector3(0, 0, 0);
		mScale = C_Vector3(1, 1, 1);

		mVert = aVert;

		std::vector<float> v;

		for (size_t i = 0; i < mVert.size(); i++)
		{
			v.push_back(mVert[i].pos.x);
			v.push_back(mVert[i].pos.y);
			v.push_back(mVert[i].pos.z);
		}

		if (v.size() > 0)
			buf = new C_Buffer(v.data(), v.size() * sizeof(float));
		v.clear();

		std::vector<float> t;

		for (size_t i = 0; i < mVert.size(); i++)
		{
			t.push_back(mVert[i].UV.x);
			t.push_back(mVert[i].UV.y);
		}

		if(t.size() > 0)
			tbuf = new C_Buffer(t.data(), t.size() * sizeof(float));
		t.clear();

		std::vector<float> n;

		for (size_t i = 0; i < mVert.size(); i++)
		{
			n.push_back(mVert[i].normal.x);			
			n.push_back(mVert[i].normal.y);
			n.push_back(mVert[i].normal.z);
		}

		if (v.size() > 0)
			nbuf = new C_Buffer(v.data(), v.size() * sizeof(float));
		v.clear();
	}

	void C_Mesh::draw(C_Shader aShader)
	{
		aShader.bind();

		if (buf == NULL && buf == nullptr)
			return;
		buf->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		if (tbuf != NULL && tbuf != nullptr)
		{
			tbuf->bind();
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
		}

		if (nbuf != NULL && nbuf != nullptr)
		{
			nbuf->bind();
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(2);
		}

		if (mMat.getTexture() != NULL && tbuf != nullptr)
		{
			mMat.getTexture()->bind();
			mMat.getTexture()->sampler2D(0);
		}

		mMatrix = glm::translate(glm::mat4(1.0f), mPos.toGLM());
		mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.z), glm::vec3(0, 0, 1));
		mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.x), glm::vec3(1, 0, 0));
		mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.y), glm::vec3(0, 1, 0));
		mMatrix = glm::scale(mMatrix, mScale.toGLM());

		glm::mat4 normalMat = glm::inverse(glm::transpose(mMatrix));

		aShader.setUniform1i("uMaterial.diffuseTex", 0);
		aShader.setUniform4f("uMaterial.color", mMat.getColor());
		aShader.setUniformMatrix("uModel", glm::value_ptr(mMatrix));
		aShader.setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
		aShader.setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));
		aShader.setUniformMatrix("uNormal", glm::value_ptr(normalMat));

		glDrawArrays(GL_TRIANGLES, 0, mVert.size());

		buf->unbind();

		C_Texture::unbind();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		aShader.unbind();
	}

	C_Mesh::~C_Mesh()
	{

	}

}




