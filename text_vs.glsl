#version 450 core

layout(location = 0) in ivec2 origin;
layout(location = 1) in vec3 color;
layout(location = 2) in uint index;

out VS
{
	ivec2 origin;
	vec3 color;
	uint index;
} vs_out;

layout(location = 0) uniform mat4 Ortho;

void main()
{
	vs_out.origin = origin;
	vs_out.color = color;
	vs_out.index = index;

	gl_Position = Ortho * vec4(origin, 0, 1);
}