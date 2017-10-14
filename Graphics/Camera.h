/************************************************
*                  Camera.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
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

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <System/System.h>

namespace C
{

	//Return projection matrix
	glm::mat4 C_GetProjectionMatrix();
	//Return view matrix
	glm::mat4 C_GetViewMatrix();
	//Set perspective matrix
	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF);
	//Set orthographic matrix
	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF);
	//Set view matrix
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
		//Constructor
		C_Camera();
		//Update camera
		void update();
		//Set camera pos
		void setPos(C_Vector3 aPos);
		//Set camera rotation
		void setRot(C_Vector3 aRot);
		//Add position to current
		void addPos(C_Vector3 aPos);
		//Add rotation to current
		void addRot(C_Vector3 aRot);
		//Set camera target
		void setTarget(C_Vector3 aTarget);
		//Return camera direction
		C_Vector3 direction() const;
		//Return camera right direction
		C_Vector3 right() const;
		//Return camera up direction
		C_Vector3 up() const;
		//Return camera position
		C_Vector3 pos() const;
		//Destructor
		~C_Camera();
	};

}
