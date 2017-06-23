#pragma once

#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <System.h>

namespace C
{

	glm::mat4 C_GetProjectionMatrix();

	glm::mat4 C_GetViewMatrix();

	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF);

	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF);

	void C_SetCamera(C_Vector3 aEye, C_Vector3 aRef);

	class C_Camera
	{
	private:
		glm::vec3 mPos;
		glm::vec3 mTarget;
		glm::vec3 mCameraDirection;
		glm::vec3 mCameraRight;
		glm::vec3 mCameraUp;
	public:
		C_Camera();

		void update();

		~C_Camera();
	};

}





