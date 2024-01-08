#version 450 core

in vec4 fragColour;
in vec2 fragTexCoord;

out vec4 colour;

uniform sampler2D ourTexture;

void main()
{
	colour = texture(ourTexture, fragTexCoord)*fragColour;
}
