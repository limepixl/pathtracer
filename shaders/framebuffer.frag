#version 460 core

in vec2 frag_uvs;
out vec4 color;

uniform sampler2D tex;

void main() 
{ 
	color = texture(tex, frag_uvs); 
}