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

	C_Matrix4 C_GetProjectionMatrix();
	C_Matrix4 C_GetViewMatrix();
	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF);
	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF);

	class C_Camera
	{
	private:
		vec3 mPos;
		vec3 mRot;
		vec3 mTarget;
		vec3 mCameraDirection;
		vec3 mCameraRight;
		vec3 mCameraUp;

		bool rotMode = false;
		bool preTargeted = false;
	public:
		C_Camera();
		
		void update();
		
		void setPos(C_Vector3 aPos);
		void addPos(C_Vector3 aPos);
		vec3 getPos() const;

		void setRot(C_Vector3 aRot);
		void addRot(C_Vector3 aRot);
		vec3 getRot() const;

		void setTarget(C_Vector3 aTarget);
		void addTarget(C_Vector3 aTarget);
		vec3 getTarget() const;
		
		vec3 direction() const;
		vec3 right() const;
		vec3 up() const;
		
		~C_Camera();
	};

}
