#ifdef VertexShader

	#extension GL_ARB_draw_instanced : enable
	#extension GL_EXT_draw_instanced : enable

	//@Attribute aPos  0
	//@Attribute aUV   1
	//@Attribute aNorm 2
	//@Attribute aTang 3

	//@Uniform uModels
	//@Uniform uView
	//@Uniform uProjection

	in vec3 aPos;
	in vec2 aUV;
	in vec3 aNorm;
	in vec3 aTang;

	uniform mat4 uModels[5];
	uniform mat4 uView;
	uniform mat4 uProjection;

	out vec2 varUV;

	void main()
	{
		Position = uProjection * uView * uModels[gl_InstanceID] * vec4(aPos, 1.0);
		varUV = aUV;
	}

#endif

#ifdef FragmentShader
	
	struct Material
	{
		sampler2D DiffuseMap;
		sampler2D SpecularMap;
		sampler2D NormalMap;
		samplerCube ReflectionMap;

		vec4 Color;
		vec3 AmbientColor;
		vec3 DiffuseColor;
		vec3 SpecularColor;
		float ReflectionPower;
		bool Lighting;
	};

	//@Uniform uMaterial

	uniform Material uMaterial;

	in vec3 varColor;
	in vec2 varUV;

	void main()
	{
		FragColor = texture(uMaterial.DiffuseMap, varUV);
	}
	
#endif









