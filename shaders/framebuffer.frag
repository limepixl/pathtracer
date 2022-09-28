#version 460 core

in vec2 frag_uvs;
out vec4 color;

uniform sampler2D tex;

void main() 
{ 
	vec4 rendered_texture = texture(tex, frag_uvs);

	if(any(isnan(rendered_texture)))
	{
		color = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}
	if(any(isinf(rendered_texture)))
	{
		color = vec4(0.0, 1.0, 0.0, 1.0);
		return;
	}

	rendered_texture.xyz = pow(rendered_texture.xyz, vec3(1.0 / 2.2));

	color = vec4(rendered_texture.xyz, 1.0);
}