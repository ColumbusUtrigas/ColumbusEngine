/************************************************
*              	   Model.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   10.01.2018                  *
*************************************************/

#include <Graphics/Model.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	C_Mesh::C_Mesh(std::vector<C_Vertex> aVert) :
		mPos(C_Vector3(0, 0, 0))
	{
		setVertices(aVert);
	}	//////////////////////////////////////////////////////////////////////////////
	C_Mesh::C_Mesh()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_Mesh::C_Mesh(std::vector<C_Vertex> aVert, C_Material aMat)
	{
		mMat = aMat;
		setVertices(aVert);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::setVertices(std::vector<C_Vertex> aVert)
	{
		delete mVBuf;
		delete mUBuf;
		delete mNBuf;
		delete mTBuf;
		delete mBBuf;

		mVert.clear();
		mVert = aVert;

		float* v = new float[mVert.size() * 3]; //Vertex buffer
		float* u = new float[mVert.size() * 2]; //UV buffer
		float* n = new float[mVert.size() * 3]; //Normal buffer
		float* t = new float[mVert.size() * 3]; //Tangent buffer
		float* b = new float[mVert.size() * 3]; //Bitangent buffer
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;
		uint64_t tcounter = 0;
		uint64_t bcounter = 0;

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

			b[bcounter++] = Vertex.bitangent.x;
			b[bcounter++] = Vertex.bitangent.y;
			b[bcounter++] = Vertex.bitangent.z;
		}

		mVBuf = new C_Buffer(v, mVert.size() * 3 * sizeof(float), 3);
		mUBuf = new C_Buffer(u, mVert.size() * 2 * sizeof(float), 2);
		mNBuf = new C_Buffer(n, mVert.size() * 3 * sizeof(float), 3);
		mTBuf = new C_Buffer(t, mVert.size() * 3 * sizeof(float), 3);
		mBBuf = new C_Buffer(b, mVert.size() * 3 * sizeof(float), 3);

		delete[] v;
		delete[] u;
		delete[] n;
		delete[] t;
		delete[] b;

		if (mMat.getShader() == nullptr) return;
		mMat.getShader()->addAttribute("aPos", 0);
		mMat.getShader()->addAttribute("aUV", 1);
		mMat.getShader()->addAttribute("aNorm", 2);
		mMat.getShader()->addAttribute("aTang", 3);
		mMat.getShader()->addAttribute("aBitang", 4);
		mMat.getShader()->compile();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Render mesh
	void C_Mesh::render(C_Transform aTransform)
	{
		if (mVBuf == nullptr) return;
		if (mMat.getShader() == nullptr) return;

		C_Buffer* const buffers[5] = { mVBuf, mUBuf, mNBuf, mTBuf, mBBuf };
		unsigned const int indices[5] = { 0, 1, 2, 3, 4 };
		unsigned const int strides[5] = { 3, 2, 3, 3, 3 };
		int i;

		for (i = 0; i < 5; i++)
			if (buffers[i] != nullptr)
				buffers[i]->bind(indices[i], C_OGL_FALSE, strides[i] * sizeof(float));

		mMat.getShader()->bind();

		setShaderMatrices(aTransform);
		setShaderMaterial();
		setShaderLightAndCamera();
		setShaderTextures();

		C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, mVert.size());
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::setCamera(C_Camera aCamera)
	{
		mCamera = aCamera;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::setPos(C_Vector3 aPos)
	{
		mPos = aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::addPos(C_Vector3 aPos)
	{
		mPos += aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Mesh::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::setParent(C_Mesh* aParent)
	{
		mParent = aParent;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Mesh::addChild(C_Mesh* aChild)
	{
		if (aChild == nullptr) return;

		mChilds.push_back(aChild);
		aChild->setParent(this);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light casters, which will calculate to using in shaders
	void C_Mesh::setLights(std::vector<C_Light*> aLights)
	{
		mLights = aLights;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Sets textures/cubemaps as uniform in shader
	void C_Mesh::setShaderTextures()
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
	//Calculate and transfer matrices as uniform in shader
	void C_Mesh::setShaderMatrices(C_Transform aTransform)
	{
		mPos = aTransform.getPos();
		mMat.getShader()->setUniformMatrix("uModel", aTransform.getMatrix().elements());
		mMat.getShader()->setUniformMatrix("uView", C_GetViewMatrix().elements());
		mMat.getShader()->setUniformMatrix("uProjection", C_GetProjectionMatrix().elements());
		mMat.getShader()->setUniformMatrix("uNormal", aTransform.getNormalMatrix().elements());
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set all material data as uniform in shader
	void C_Mesh::setShaderMaterial()
	{
		C_Vector4 matcol = mMat.getColor();
		C_Vector3 matamb = mMat.getAmbient();
		C_Vector3 matdif = mMat.getDiffuse();
		C_Vector3 matspc = mMat.getSpecular();

		float const MaterialUnif[14] =
		{
			matcol.x, matcol.y, matcol.z, matcol.w,
			matamb.x, matamb.y, matamb.z,
			matdif.x, matdif.y, matdif.z,
			matspc.x, matspc.y, matspc.z,
			mMat.getReflectionPower()
		};

		mMat.getShader()->setUniformArrayf("MaterialUnif", MaterialUnif, 14);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set all lights and camera data as unniform in shader
	void C_Mesh::setShaderLightAndCamera()
	{
		calculateLights();

		mMat.getShader()->setUniformArrayf("LightUnif", mLightUniform, 120);
		mMat.getShader()->setUniform3f("uCamera.pos", mCamera.pos());
	}
	//////////////////////////////////////////////////////////////////////////////
	//Calculate lights
	void C_Mesh::calculateLights()
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
	//Sort lights array by distance
	void C_Mesh::sortLights()
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
	C_Mesh::~C_Mesh()
	{
		delete mVBuf;
		delete mUBuf;
		delete mNBuf;
		delete mTBuf;
		delete mBBuf;
	}

}
