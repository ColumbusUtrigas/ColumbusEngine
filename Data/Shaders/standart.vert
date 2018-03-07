in vec3 aPos;
in vec2 aUV;
in vec3 aNorm;
in vec3 aTang;
in vec3 aBitang;

varying vec3 varPos;
varying vec2 varUV;
varying vec3 varNormal;
varying vec3 varFragPos;
varying mat3 varTBN;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uNormal;

void main()
{
	Position = uProjection * uView * uModel * vec4(aPos, 1.0);

	vec3 normal = normalize(vec3(uNormal * vec4(aNorm, 0.0)));
	vec3 tangent = normalize(vec3(uNormal * vec4(aTang, 0.0)));
	vec3 bitangent = cross(normal, tangent);

	varPos = vec3(uModel * vec4(aPos, 1.0));
	varUV = aUV;
	varNormal = normal;
	varFragPos = vec3(uModel * vec4(aPos, 1.0));
	varTBN = transpose(mat3(tangent, bitangent, normal));
}





