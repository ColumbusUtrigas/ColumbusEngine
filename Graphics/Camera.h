/************************************************
*                  Camera.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#include <System/System.h>
#include <Math/Matrix4.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	C_Matrix4& C_GetProjectionMatrix();
	C_Matrix4 C_GetViewMatrix();
	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF);
	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF);
	void C_SetCamera(C_Vector3 aEye, C_Vector3 aRef);

	class C_Camera
	{
	private:
		glm::vec3 mPos;
		glm::vec3 mRot;
		glm::vec3 mTarget;
		glm::vec3 mCameraDirection;
		glm::vec3 mCameraRight;
		glm::vec3 mCameraUp;

		bool rotMode = false;
		bool preTargeted = false;
	public:
		C_Camera();
		
		void update();
		
		void setPos(C_Vector3 aPos);
		void addPos(C_Vector3 aPos);
		C_Vector3 getPos() const;

		void setRot(C_Vector3 aRot);
		void addRot(C_Vector3 aRot);
		C_Vector3 getRot() const;

		void setTarget(C_Vector3 aTarget);
		void addTarget(C_Vector3 aTarget);
		C_Vector3 getTarget() const;
		
		C_Vector3 direction() const;
		C_Vector3 right() const;
		C_Vector3 up() const;
		C_Vector3 pos() const;
		
		~C_Camera();
	};

}
