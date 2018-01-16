/************************************************
*                  MeshOpenGL.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   16.01.2018                  *
*************************************************/
#include <Graphics/OpenGL/MeshOpenGL.h>

namespace Columbus
{

	C_MeshOpenGL::C_MeshOpenGL()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_MeshOpenGL::C_MeshOpenGL(std::vector<C_Vertex> aVert)
	{
		setVertices(aVert);
	}
	//////////////////////////////////////////////////////////////////////////////
	C_MeshOpenGL::C_MeshOpenGL(std::vector<C_Vertex> aVert, C_Material aMaterial)
	{
		mMat = aMaterial;
		setVertices(aVert);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::setVertices(std::vector<C_Vertex> aVert)
	{
		glGenBuffers(1, &mVBuf);

		mVert.clear();
		mVert = aVert;

		float* v = new float[mVert.size() * 3]; //Vertex buffer
		float* u = new float[mVert.size() * 2]; //UV buffer
		float* n = new float[mVert.size() * 3]; //Normal buffer
		float* t = new float[mVert.size() * 3]; //Tangent buffer
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;
		uint64_t tcounter = 0;

		for (auto Vertex : mVert)
		{
			v[vcounter++] = Vertex.pos.x;
			v[vcounter++] = Vertex.pos.y;
			v[vcounter++] = Vertex.pos.z;

			u[ucounter++] = Vertex.UV.x;
			u[ucounter++] = Vertex.UV.y;

			n[ncounter++] = Vertex.normal.x;
			n[ncounter++] = Vertex.normal.y;
			n[ncounter++] = Vertex.normal.z;

			t[tcounter++] = Vertex.tangent.x;
			t[tcounter++] = Vertex.tangent.y;
			t[tcounter++] = Vertex.tangent.z;
		}

		size_t size = (sizeof(float) * mVert.size() * 3)
		            + (sizeof(float) * mVert.size() * 2)
		            + (sizeof(float) * mVert.size() * 3)
		            + (sizeof(float) * mVert.size() * 3);

		mVOffset = 0;
		mUOffset = mVOffset + (sizeof(float) * mVert.size() * 3);
		mNOffset = mUOffset + (sizeof(float) * mVert.size() * 2);
		mTOffset = mNOffset + (sizeof(float) * mVert.size() * 3);

		glBindBuffer(GL_ARRAY_BUFFER, mVBuf);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);


		glBufferSubData(GL_ARRAY_BUFFER, mVOffset, mVert.size() * 3 * sizeof(float), v);
		glBufferSubData(GL_ARRAY_BUFFER, mUOffset, mVert.size() * 2 * sizeof(float), u);
		glBufferSubData(GL_ARRAY_BUFFER, mNOffset, mVert.size() * 3 * sizeof(float), n);
		glBufferSubData(GL_ARRAY_BUFFER, mTOffset, mVert.size() * 3 * sizeof(float), t);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		delete[] v;
		delete[] u;
		delete[] n;
		delete[] t;

		if (mMat.getShader() == nullptr) return;
		mMat.getShader()->addAttribute("aPos", 0);
		mMat.getShader()->addAttribute("aUV", 1);
		mMat.getShader()->addAttribute("aNorm", 2);
		mMat.getShader()->addAttribute("aTang", 3);
		mMat.getShader()->compile();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::render(C_Transform aTransform)
	{
		if (!glIsBuffer(mVBuf)) return;
		if (mMat.getShader() == nullptr) return;

		size_t const offsets[4] = { mVOffset, mUOffset, mNOffset, mTOffset };
		unsigned const int strides[4] = { 3, 2, 3, 3 };
		int i;

		glBindBuffer(GL_ARRAY_BUFFER, mVBuf);

		for (i = 0; i < 4; i++)
		{
			glVertexAttribPointer(i, strides[i], GL_FLOAT, GL_FALSE, 0, (void*)offsets[i]);
			glEnableVertexAttribArray(i);
		}

		mMat.getShader()->bind();

		setShaderMatrices(aTransform);
		setShaderMaterial();
		setShaderLightAndCamera();
		setShaderTextures();

		glDrawArrays(GL_TRIANGLES, 0, mVert.size());
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::setShaderTextures()
	{
		C_Texture* textures[3] = {mMat.getTexture(), mMat.getSpecMap(), mMat.getNormMap()};
		C_Cubemap* cubemap = mMat.getReflection();
		std::string unifs[3] = {"uMaterial.diffuseMap", "uMaterial.specularMap", "uMaterial.normalMap"};
		unsigned int indices[3] = {0, 1, 3};

		for (int i = 0; i < 3; i++)
		{
			if (textures[i] != nullptr)
			{
				mMat.getShader()->setUniform1i(unifs[i].c_str(), indices[i]);
				textures[i]->sampler2D(indices[i]);
			} else
			{
				mMat.getShader()->setUniform1i(unifs[i].c_str(), indices[i]);
				if (textures[0] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE0);
				if (textures[1] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE1);
				if (textures[2] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE3);
			}
		}

		if (cubemap != nullptr)
		{
			mMat.getShader()->setUniform1i("uReflectionMap", 2);
			cubemap->samplerCube(2);
		} else
		{
			mMat.getShader()->setUniform1i("uReflectionMap", 2);
			C_DeactiveCubemapOpenGL(C_OGL_TEXTURE2);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::setShaderMatrices(C_Transform aTransform)
	{
		mPos = aTransform.getPos();
		mMat.getShader()->setUniformMatrix("uModel", aTransform.getMatrix().elements());
		mMat.getShader()->setUniformMatrix("uView", C_GetViewMatrix().elements());
		mMat.getShader()->setUniformMatrix("uProjection", C_GetProjectionMatrix().elements());
		mMat.getShader()->setUniformMatrix("uNormal", aTransform.getNormalMatrix().elements());
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::setShaderMaterial()
	{
		C_Vector4 matcol = mMat.getColor();
		C_Vector3 matamb = mMat.getAmbient();
		C_Vector3 matdif = mMat.getDiffuse();
		C_Vector3 matspc = mMat.getSpecular();

		mMaterialUnif[0] = matcol.x;
		mMaterialUnif[1] = matcol.y;
		mMaterialUnif[2] = matcol.z;
		mMaterialUnif[3] = matcol.w;
		mMaterialUnif[4] = matamb.x;
		mMaterialUnif[5] = matamb.y;
		mMaterialUnif[6] = matamb.z;
		mMaterialUnif[7] = matamb.x;
		mMaterialUnif[8] = matamb.y;
		mMaterialUnif[9] = matamb.z;
		mMaterialUnif[10] = matspc.x;
		mMaterialUnif[11] = matspc.y;
		mMaterialUnif[12] = matspc.z;
		mMaterialUnif[13] = mMat.getReflectionPower();

		mMat.getShader()->setUniformArrayf("MaterialUnif", mMaterialUnif, 14);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::setShaderLightAndCamera()
	{
		calculateLights();

		mMat.getShader()->setUniformArrayf("LightUnif", mLightUniform, 120);
		mMat.getShader()->setUniform3f("uCamera.pos", mCamera.pos());
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::calculateLights()
	{
		sortLights();
		int i, j, offset;
		//8 - max count of lights, processing in shader
		for (i = 0; i < 8; i++)
		{
			offset = i * 15;

			if (i < mLights.size() && mMat.getLighting() == true)
			{
				//Color
				mLightUniform[0 + offset] = mLights[i]->getColor().x;
				mLightUniform[1 + offset] = mLights[i]->getColor().y;
				mLightUniform[2 + offset] = mLights[i]->getColor().z;
				//Position
				mLightUniform[3 + offset] = mLights[i]->getPos().x;
				mLightUniform[4 + offset] = mLights[i]->getPos().y;
				mLightUniform[5 + offset] = mLights[i]->getPos().z;
				//Direction
				mLightUniform[6 + offset] = mLights[i]->getDir().x;
				mLightUniform[7 + offset] = mLights[i]->getDir().y;
				mLightUniform[8 + offset] = mLights[i]->getDir().z;
				//Type
				mLightUniform[9 + offset] = mLights[i]->getType();
				//Constant attenuation
				mLightUniform[10 + offset] = mLights[i]->getConstant();
				//Linear attenuation
				mLightUniform[11 + offset] = mLights[i]->getLinear();
				//Quadratic attenuation
				mLightUniform[12 + offset] = mLights[i]->getQuadratic();
				//Inner cutoff
				mLightUniform[13 + offset] = mLights[i]->getInnerCutoff();
				//Outer cutoff
				mLightUniform[14 + offset] = mLights[i]->getOuterCutoff();
			} else
			{
				for (j = 0; j < 15; j++)
					mLightUniform[j + offset] = -1;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_MeshOpenGL::sortLights()
	{
		C_Vector3 pos = mPos;

		mLights.erase(std::remove(mLights.begin(), mLights.end(), nullptr), mLights.end());

		auto func = [pos](const C_Light* a, const C_Light* b) mutable -> bool
		{
			C_Vector3 q = a->getPos();
			C_Vector3 w = b->getPos();

			return q.length(pos) < w.length(pos);
		};

		std::sort(mLights.begin(), mLights.end(), func);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_MeshOpenGL::~C_MeshOpenGL()
	{
		
	}

}













