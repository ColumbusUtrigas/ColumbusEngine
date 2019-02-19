#ifdef VertexShader

	//@Attribute aPos 0
	in vec3 aPos;
	out vec3 Pos;

	//@Uniform Projection
	//@Uniform View
	uniform mat4 Projection;
	uniform mat4 View;

	void main()
	{
		gl_Position = Projection * View * vec4(aPos, 1);
		Pos = aPos;
	}

#endif

#ifdef FragmentShader

	in vec3 Pos;

	//@Uniform EnvironmentMap
	uniform TextureCube EnvironmentMap;

	const float PI = 3.14159265359;

	void main()
	{
		vec3 Normal = normalize(Pos);  
		vec3 Irradiance = vec3(0.0);

		vec3 Up    = vec3(0.0, 1.0, 0.0);
		vec3 Right = cross(Up, Normal);
		Up         = cross(Normal, Right);

		float SampleDelta = 0.025;
		float NumSamples = 0.0;

		for(float Phi = 0.0; Phi < 2.0 * PI; Phi += SampleDelta)
		{
			for(float Theta = 0.0; Theta < 0.5 * PI; Theta += SampleDelta)
			{
				vec3 TangentSample = vec3(sin(Theta) * cos(Phi), sin(Theta) * sin(Phi), cos(Theta));
				vec3 SampleVec = TangentSample.x * Right + TangentSample.y * Up + TangentSample.z * Normal; 

				Irradiance += SampleCube(EnvironmentMap, SampleVec).rgb * cos(Theta) * sin(Theta);
				NumSamples += 1.0;
			}
		}

		FragColor = vec4(PI * Irradiance * (1.0 / NumSamples), 1.0);
	}

#endif


