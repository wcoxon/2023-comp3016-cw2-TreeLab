
#version 400 core

layout( location = 0 ) in vec3 vPosition;
layout( location = 1 ) in vec4 vColour;
layout (location = 2) in vec2 vertTexCoord;
layout (location = 3) in vec3 vNormal;

out vec4 fragColour;
out vec2 fragTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 colour;

//vec3 lightDirection = normalize(vec3(1.0f,-1.0f,0.0f));

void main(){

    gl_Position = projection*view*model * vec4 (vPosition,1.0);

	fragTexCoord = vertTexCoord;

	//float diffuse = max(dot(vNormal,-lightDirection),0);
	//float ambient = 0.2;

	fragColour = vec4(0.0f,0.0f,0.0f,1.0f);//vec4(1.0f,1.0f,1.0f,1.0f)*vec4(vec3(ambient+diffuse),1.0);

}
