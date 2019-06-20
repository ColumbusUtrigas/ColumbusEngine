#shader vertex

#attribute vec2 aPos 0
#attribute vec2 aUV  1

#uniform vec2 Pos
#uniform vec2 Size

out vec2 UV;

void main(void)
{
	Position = vec4(aPos * Size + Pos, 0, 1);
	UV = aUV;
}


#shader fragment

#uniform Texture2D Texture

in vec2 UV;

void main()
{
	FragData[0] = Sample2D(Texture, UV);
}


