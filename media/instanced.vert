#version 400 core

layout( location = 0 ) in vec3 vertPosition;
layout( location = 1 ) in vec4 vertColour;
layout (location = 2) in vec2 vertTexCoord;
layout (location = 3) in vec3 vertNormal;
layout (location = 4) in mat4 instanceTransform;

out vec4 fragColour;
out vec2 fragTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec4 colour;
uniform float time;

uniform vec3 lightDirection;

void main(){
	float factor = radians(15)*sin(instanceTransform[3].x+4*time);
	
	mat3 windRotation = mat3(
	1,0,0, 
	0,cos(factor),-sin(factor), 
	0,sin(factor),cos(factor)
	);

	gl_Position = projection*view*instanceTransform * vec4 (windRotation*vertPosition,1.0);

	vec3 transformedNormal = transpose(inverse(mat3(instanceTransform)))*windRotation*vertNormal;

	float diffuse = max(dot(transformedNormal,-lightDirection),0);
	float ambient = 0.2;

	fragColour = vec4(vec3(clamp(ambient+diffuse,0,1)),1.0);
	fragTexCoord = vertTexCoord;
}
