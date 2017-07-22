/************************************************
*              	   Model.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Model.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
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

		if (n.size() > 0)
			nbuf = new C_Buffer(n.data(), n.size() * sizeof(float));
		n.clear();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Mesh::C_Mesh(std::string aFile)
	{
		Importer::C_Importer importer;
		if(importer.load(aFile))
		{

		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 3
  C_Mesh::C_Mesh()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw mesh
	void C_Mesh::draw()
	{
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

		if (mMat.getShader() != nullptr)
		{
			mMat.getShader()->bind();

			C_Texture::unbind();

			mMatrix = glm::translate(glm::mat4(1.0f), mPos.toGLM());
			mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.z), glm::vec3(0, 0, 1));
			mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.x), glm::vec3(1, 0, 0));
			mMatrix = glm::rotate(mMatrix, C_DegToRads(mRot.y), glm::vec3(0, 1, 0));
			mMatrix = glm::scale(mMatrix, mScale.toGLM());

			glm::mat4 normalMat = glm::inverse(glm::transpose(mMatrix));

			mMat.getShader()->setUniform4f("uMaterial.color", mMat.getColor());
			mMat.getShader()->setUniform3f("uMaterial.ambient", mMat.getAmbient());
			mMat.getShader()->setUniform3f("uMaterial.diffuse", mMat.getDiffuse());
			mMat.getShader()->setUniform3f("uMaterial.specular", mMat.getSpecular());
			mMat.getShader()->setUniform3f("uLight.color", C_Vector3(1, 1, 1));
			mMat.getShader()->setUniform3f("uLight.pos", mCamera.pos());
			mMat.getShader()->setUniform1i("uLight.type", 1);
			mMat.getShader()->setUniform1f("uLight.constant", 1);
			mMat.getShader()->setUniform1f("uLight.linear", 0.09f);
			mMat.getShader()->setUniform1f("uLight.quadratic", 0.032f);
			mMat.getShader()->setUniform3f("uCamera.pos", mCamera.pos());

			mMat.getShader()->setUniformMatrix("uModel", glm::value_ptr(mMatrix));
			mMat.getShader()->setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
			mMat.getShader()->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));
			mMat.getShader()->setUniformMatrix("uNormal", glm::value_ptr(normalMat));

			if (mMat.getTexture() != nullptr)
			{
				mMat.getShader()->setUniform1i("uMaterial.diffuseMap", 0);
				mMat.getTexture()->sampler2D(0);
			}

			if (mMat.getSpecMap() != nullptr)
			{
				mMat.getShader()->setUniform1i("uMaterial.specularMap", 1);
				mMat.getSpecMap()->sampler2D(1);
			}

			if (mMat.getReflection() != nullptr)
			{
				glActiveTexture(GL_TEXTURE2);
				mMat.getShader()->setUniform1i("uReflectionMap", 2);
				mMat.getReflection()->bind();
			}
		}


		glDrawArrays(GL_TRIANGLES, 0, mVert.size());

		C_Cubemap::unbind();

		C_Buffer::unbind();

		C_Texture::unbind();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		C_Shader::unbind();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set camera
	void C_Mesh::setCamera(C_Camera aCamera)
	{
		mCamera = aCamera;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set mesh position
	void C_Mesh::setPos(C_Vector3 aPos)
	{
		mPos = aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set mesh rotation
	void C_Mesh::setRot(C_Vector3 aRot)
	{
		mRot = aRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set mesh scale
	void C_Mesh::setScale(C_Vector3 aScale)
	{
		mScale = aScale;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add position to current
	void C_Mesh::addPos(C_Vector3 aPos)
	{
		mPos += aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add rotation to current
	void C_Mesh::addRot(C_Vector3 aRot)
	{
		mRot += aRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add scale to current
	void C_Mesh::addScale(C_Vector3 aScale)
	{
		mScale += aScale;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set parent mesh
	void C_Mesh::setParent(C_Mesh* aParent)
	{
		mParent = aParent;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add child mesh
	void C_Mesh::addChild(C_Mesh* aChild)
	{
		if (aChild == nullptr)
			return;

		mChilds.push_back(aChild);
		aChild->setParent(this);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Mesh::~C_Mesh()
	{

	}

}
