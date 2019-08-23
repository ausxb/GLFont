#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

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

	/*
	float tlx; //Low X
	float thx; //High X
	float tly; //Low Y
	float thy; //High Y
	*/
};

layout(std430, binding = 0) buffer AtlasMap
{
	Meta meta[];
} glyph;

layout(location = 0) uniform mat4 Ortho;

in VS
{
	ivec2 origin;
	vec3 color;
	uint index;
} gs_in[];

out GS
{
	vec3 color;
	flat ivec2 base; //Lower left base of quad in screen coordinates
	flat uint index;
} gs_out;

void main()
{
	const vec2 vertex[] = vec2[](
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 1.0)
	);

	int xbase = gs_in[0].origin.x + glyph.meta[gs_in[0].index].lb; //Quad base x
	int ybase = gs_in[0].origin.y - (int(glyph.meta[gs_in[0].index].bh) - glyph.meta[gs_in[0].index].tb); //Quad base y

	//float x = xbase + vertex[gl_VertexID].x * (glyph.meta[gs_in[0].index].bw - 1);
	//float y = ybase + vertex[gl_VertexID].y * (glyph.meta[gs_in[0].index].bh - 1);

	gs_out.color = gs_in[0].color;
	gs_out.base = ivec2(xbase, ybase);
	gs_out.index = gs_in[0].index;

	gl_Position = Ortho * vec4(
		xbase,
		ybase,
		0.0,
		1.0
	);
	EmitVertex();

	gl_Position = Ortho * vec4(
		xbase + glyph.meta[gs_in[0].index].bw,
		ybase,
		0.0,
		1.0
	);
	EmitVertex();

	gl_Position = Ortho * vec4(
		xbase,
		ybase + glyph.meta[gs_in[0].index].bh,
		0.0,
		1.0
	);
	EmitVertex();

	gl_Position = Ortho * vec4(
		xbase + glyph.meta[gs_in[0].index].bw,
		ybase + glyph.meta[gs_in[0].index].bh,
		0.0,
		1.0
	);
	EmitVertex();

	EndPrimitive();

	/*
	gs_out.tex = vec2(glyph.meta[gs_in[0].index].tx, glyph.meta[gs_in[0].index].ty);
	gs_out.tex = vec2(glyph.meta[gs_in[0].index].thx, glyph.meta[gs_in[0].index].ty);
	gs_out.tex = vec2(glyph.meta[gs_in[0].index].tx, glyph.meta[gs_in[0].index].thy);
	gs_out.tex = vec2(glyph.meta[gs_in[0].index].thx, glyph.meta[gs_in[0].index].thy);
	*/
}