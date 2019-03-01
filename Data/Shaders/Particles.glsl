#ifdef VertexShader

	//@Attribute Vertices  0
	//@Attribute Texcoords 1
	//@Attribute Positions 2
	//@Attribute Sizes     3
	//@Attribute Colors    4
	//@Attribute OtherData 5

	in vec3 Vertices;
	in vec2 Texcoords;
	in vec3 Positions;
	in vec3 Sizes;
	in vec4 Colors;
	in vec2 OtherData;

	//@Uniform ViewProjection
	//@Uniform View
	//@Uniform Projection
	//@Uniform Billboard
	//@Uniform Frame

	uniform mat4 ViewProjection;
	uniform mat4 View;
	uniform mat4 Projection;
	uniform mat4 Billboard;
	uniform vec2 Frame;

	out vec4 Color;
	out vec2 UV;

	mat4 RotationMatrix(vec3 axis, float angle)
	{
		axis = normalize(axis);
		float s = sin(angle);
		float c = cos(angle);
		float oc = 1.0 - c;

		return  mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		             oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
		             oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
		             0.0,                                0.0,                                0.0,                                1.0);
	}

	vec2 ExtractSubUV(int FrameNumber)
	{
		int FrameHorizontal = FrameNumber % int(Frame.x);
		int FrameVertical = FrameNumber / int(Frame.x) - 1;
		vec2 Begin = 1.0 / Frame;

		return vec2(Texcoords.x * Begin.x + Begin.x * FrameHorizontal,
		            Texcoords.y * Begin.y + Begin.y * FrameVertical);
	}

	void main(void)
	{
		mat4 Rotation = RotationMatrix(vec3(0, 0, 1), 0.011111 * OtherData.x);
		vec4 VertexPosition = vec4(Vertices * Sizes, 1);
		vec4 WorldPosition = vec4(Positions, 0);

		Position = Projection * (View * (Billboard * (VertexPosition * Rotation) + WorldPosition));
		//Position = Projection * (View * vec4(Positions, 1.0) + vec4(Vertices, 0.0) * vec4(Sizes, 0.0) * Rotation);
		UV = ExtractSubUV(int(OtherData.y));
		Color = Colors;

		/*mat4 Rotation = RotationMatrix(vec3(0, 0, 1), 0.011111 * gl_FogCoord);
		vec4 VertexPosition = Model * (vec4(gl_Vertex.xyz, 1) * Rotation);
		vec4 WorldPosition = vec4(gl_Normal.xyz, 0);
		Position = Projection * (View * (VertexPosition + WorldPosition));
		Color = gl_Color;
		UV = gl_MultiTexCoord0.xy * 0.25;*/
	}

#endif

#ifdef FragmentShader

	//@Uniform Texture
	uniform Texture2D Texture;

	in vec4 Color;
	in vec2 UV;

	void main(void)
	{
		FragColor = Sample2D(Texture, UV) * Color;
		//FragColor = Color;
	}

#endif


