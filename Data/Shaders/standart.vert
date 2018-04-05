in vec3 aPos;
in vec2 aUV;
in vec3 aNorm;
in vec3 aTang;

out vec3 varPos;
out vec2 varUV;
out vec3 varNormal;
out vec3 varFragPos;
out mat3 varTBN;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	//Position = vec4(aPos, 1.0) * uModel * uView * uProjection;
	//Position = uProjection * uView * vec4(aPos, 1.0) * uModel;

	vec3 normal = normalize(vec3(uModel * vec4(aNorm, 0.0)));
	vec3 tangent = normalize(vec3(uModel * vec4(aTang, 0.0)));
	vec3 bitangent = cross(normal, tangent);

	varPos = vec3(uModel * vec4(aPos, 1.0));
	varUV = aUV;
	varNormal = normal;
	varFragPos = vec3(uModel * vec4(aPos, 1.0));
	varTBN = transpose(mat3(tangent, bitangent, normal));
}



