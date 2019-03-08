#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}


#shader fragment

#uniform Texture2D Texture

in vec2 UV;

void main()
{
	FragData[0] = Sample2D(Texture, UV);

}


