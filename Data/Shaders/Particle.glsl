#ifdef VertexShader

	//@Attribute aPos 0
	//@Attribute aUV 1
	//@Attribute aPositions 2
	//@Attribute aTimes 3
	//@Attribute aColors 4
	//@Attribute aSizes 5

	in vec3 aPos;
	in vec2 aUV;
	in vec3 aPositions;
	in vec2 aTimes;
	in vec4 aColors;
	in vec3 aSizes;

	out vec2 varTexCoord;
	out vec4 varColor;

	//@Uniform uView
	//@Uniform uProjection	
	//@Uniform uSubUV
	//@Uniform uBillboard

	uniform mat4 uView;
	uniform mat4 uProjection;
	uniform vec2 uSubUV;
	uniform float uBillboard;

	void RotationMatrix(vec3 axis, float angle, out mat4 Result)
	{
	    axis = normalize(axis);
	    float s = sin(angle);
	    float c = cos(angle);
	    float oc = 1.0 - c;
	    
	    Result =  mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	                   oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	                   oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	                   0.0,                                0.0,                                0.0,                                1.0);
	}

	vec2 ExtractSubUV(in float frame)
	{
		int FrameNumber = int(frame);
		int FrameHorizontal = FrameNumber % int(uSubUV.x);
		int FrameVertical = FrameNumber / int(uSubUV.x) - 1;
		vec2 Begin = 1.0 / uSubUV;

		return vec2(aUV.x * Begin.x + Begin.x * FrameHorizontal,
		            aUV.y * Begin.y + Begin.y * FrameVertical);
	}

	void main(void)
	{
		vec3 Pos = aPositions;
		vec2 Times = aTimes;
		vec3 Size = aSizes;
		vec4 Color = aColors;
		vec2 SubUV = ExtractSubUV(Times.y);

		mat4 Rotation;
		RotationMatrix(vec3(0, 0, 1), Times.x * 0.011111, Rotation);

		if (uBillboard != 0.0)
		{
			Position = uProjection * (uView * vec4(Pos, 1.0) + vec4(aPos, 0.0) * vec4(Size, 0.0) * Rotation);
		} else
		{
			Position = uProjection * uView * (vec4(Pos, 1.0) + vec4(aPos * Size, 0.0) * Rotation);
		}

		varTexCoord = SubUV;
		varColor = Color;
	}

#endif

#ifdef FragmentShader

	in vec2 varTexCoord;
	in vec4 varColor;

	//@Uniform uTex
	uniform Texture2D uTex;

	void main()
	{
		FragColor = varColor;

		if (textureSize(uTex, 0).x >= 1)
		{
			FragColor *= Sample2D(uTex, varTexCoord);
		}

		if (FragColor.w < 0.01) discard;
	}

#endif


