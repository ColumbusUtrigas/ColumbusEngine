#include <Camera.h>

namespace C
{

	static glm::mat4 PROJECTION_MATRIX;
	static glm::mat4 VIEW_MATRIX;

	glm::mat4 C_GetProjectionMatrix()
	{
		return PROJECTION_MATRIX;
	}

	glm::mat4 C_GetViewMatrix()
	{
		return VIEW_MATRIX;
	}

	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF)
	{
		PROJECTION_MATRIX = glm::perspective(C_DegToRads(aFOV), aAspect, aN, aF);
	}

	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF)
	{
		PROJECTION_MATRIX = glm::ortho(aL, aR, aB, aT, aN, aF);
	}

	void C_SetCamera(C_Vector3 aEye, C_Vector3 aRef)
	{
		VIEW_MATRIX = glm::lookAt(aEye.toGLM(), aRef.toGLM(), glm::vec3(0, 1, 0));
	}

	C_Camera::C_Camera()
	{
		mPos = glm::vec3(0, 0, 5);
		mTarget = glm::vec3(0, 0, 0);
		mCameraDirection = glm::vec3(0, 0, -1);
		mCameraRight = glm::vec3(1, 0, 0);
		mCameraUp = glm::vec3(0, 1, 0);
	}

	void C_Camera::update()
	{
		mCameraDirection = glm::normalize(mPos - mTarget);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		mCameraRight = glm::normalize(glm::cross(up, mCameraDirection));
		mCameraUp = glm::cross(mCameraDirection, mCameraRight);

		C_Vector3 tmpPos;
		tmpPos.fromGLM(mPos);
		C_Vector3 tmpTar;
		tmpTar.fromGLM(mTarget);

		C_SetCamera(tmpPos, tmpTar);
	}

	C_Camera::~C_Camera()
	{

	}

}





