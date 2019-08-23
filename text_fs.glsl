#version 450 core

struct Meta
{
    int ax; // X advance
	int ay; // Y advance
	uint bw; // Bitmap width
	uint bh; // Bitmap rows
	int lb; // Left bearing
	int tb; // Top bearing
	
	//OpenGL texel coordinates with origin in lower left
	int tx; // Texel X base
	int ty; // Texel Y base
};

layout(std430, binding = 0) buffer AtlasMap
{
	Meta meta[];
} glyph;

layout(binding = 0) uniform usampler2D bitmap;

layout(location = 0) out vec4 pixel;

in GS
{
	vec3 color;
	flat ivec2 base; //Lower left base of quad in screen coordinates
	flat uint index;
} fs_in;

void main()
{
	const ivec2 relative = ivec2(gl_FragCoord) - fs_in.base;
	
	ivec2 atlas_pixel = ivec2(
		glyph.meta[fs_in.index].tx + relative.x,
		glyph.meta[fs_in.index].ty - relative.y
	);

	pixel = vec4(fs_in.color, 1.0) * vec4(1.0, 1.0, 1.0, float(texelFetch(bitmap, atlas_pixel, 0).r) / 256.0);

	//pixel = vec4(1.0, 1.0, 1.0, float(texelFetch(bitmap, ivec2(gl_FragCoord), 0).r) / 256.0);
}