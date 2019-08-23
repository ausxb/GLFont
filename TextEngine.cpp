#include "TextEngine.h"

#ifdef _DEBUG
#include <iostream>
#endif

TextEngine::TextEngine(const FontManager &mgr, const glwrap::Program &prg, unsigned width, unsigned height, unsigned initCapacity) :
	scX{ width }, scY{ height },
	manager{ mgr }, program{ prg },
	texture{ GL_TEXTURE_2D, 1, GL_R8UI, mgr.mapWidth(), mgr.mapHeight(), 0 },
	range{ mgr.charpast() - mgr.charbase() }, glyphs{ 0 }, capacity{ initCapacity },
	update{ false }, text_id{ 0 }, updateIndex{ 0 }, updateOffset{ 0 },
	updateIterator{ displayList.end() },
	orthographic{ glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height)) }
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, VERTEX_BYTES * capacity, NULL, GL_MAP_WRITE_BIT);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, VERTEX_BYTES);
	
	glVertexArrayAttribIFormat(vao, 0, 2, GL_INT, 0);
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 8);
	glVertexArrayAttribIFormat(vao, 2, 1, GL_UNSIGNED_INT, 20);
	
	glVertexArrayAttribBinding(vao, 0, 0); //Origin
	glVertexArrayAttribBinding(vao, 1, 0); //Color
	glVertexArrayAttribBinding(vao, 2, 0); //Map Index

	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);
	glEnableVertexArrayAttrib(vao, 2);
	
	glCreateBuffers(1, &ssbo);
	glNamedBufferStorage(ssbo, META_BYTES * range, NULL, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
	loadMetaInfo();

	texture.store(0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, manager.raw());
	//Change to glwrap::Sampler
	/*glTextureParameteri(texture.id(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
}

TextEngine::~TextEngine()
{
	glDeleteBuffers(1, &ssbo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void TextEngine::render()
{
	if(update) updateBuffer();
	
	if(glyphs != 0)
	{
		program.use();
		program.setMat4(0, glm::value_ptr(orthographic));
		texture.bind(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, glyphs);
	}

	GLenum err = glGetError();
	if (err) exit(err);
}

unsigned __int64 TextEngine::addString(const std::string &s, glm::ivec2 &origin, glm::vec3 &color)
{
	update = true;
	
	/*
		Since the id is pushed to the back of the list first,
		the list will always have an element,
		allowing for a valid iterator to one before the end() of the list.
	*/
	displayList.push_back(text_id); 
	//TO DO: Check to make sure the list isn't empty, otherwise --displayList.end() is invalid
	std::list<unsigned __int64>::iterator pos = --displayList.end();
	display[text_id] = Info{s, origin, color, pos, glyphs * VERTEX_BYTES};
	
	if (changeUpdateInfo(pos, glyphs * VERTEX_BYTES))
		;
	else if (displayList.size() == 1)
		updateIterator = displayList.begin();
	
	glyphs += s.length();
	text_id++;
	
	return text_id - 1; //The id of the newly added string is the incremented counter minus one
}

bool TextEngine::removeString(unsigned __int64 id)
{	
	update = true;
	
	if(display.find(id) == display.end())
		return false;
	
	Info &ref = display[id];
	std::list<unsigned __int64>::iterator pos = ref.position;
	unsigned glyphs_removed = ref.str.length();
	auto update_pos = pos;

	++update_pos;
	
	glyphs -= glyphs_removed;
	
	changeUpdateInfo(update_pos, ref.offset);

	displayList.erase(pos);
	display.erase(display.find(id));
	
	return true;
}

bool TextEngine::updateString(unsigned __int64 id, const std::string &s)
{
	update = true;
	
	if(display.find(id) == display.end())
		return false;
	
	Info &ref = display[id];
	unsigned glyphs_removed = ref.str.length();
	ref.str = s;

	glyphs -= glyphs_removed;
	glyphs += s.length();
	
	changeUpdateInfo(ref.position, ref.offset);
	
	return true;
}

bool TextEngine::updateOrigin(unsigned __int64 id, const glm::ivec2 &origin)
{
	update = true;

	if (display.find(id) == display.end())
		return false;

	Info &ref = display[id];
	ref.origin = origin;

	changeUpdateInfo(ref.position, ref.offset);

	return true;
}

bool TextEngine::updateColor(unsigned __int64 id, const glm::vec3 &color)
{
	update = true;

	if (display.find(id) == display.end())
		return false;

	Info &ref = display[id];
	ref.color = color;

	changeUpdateInfo(ref.position, ref.offset);

	return true;
}

#ifdef _DEBUG
void TextEngine::printVBO()
{
	unsigned char *base = static_cast<unsigned char*>(glMapNamedBufferRange(vbo, 0, VERTEX_BYTES * glyphs, GL_MAP_READ_BIT));
	unsigned char *ptr = base;

	for (int i = 0; i < glyphs; i++)
	{
		unsigned uc = *reinterpret_cast<unsigned*>(ptr + 20) + manager.charbase();
		std::cout << static_cast<void*>(ptr) << ": '" << static_cast<char>(uc) << '\'' << std::endl;
		std::cout << "\tOrigin: (" << *reinterpret_cast<int*>(ptr) << ", "
			<< *reinterpret_cast<int*>(ptr + 4) << ')' << std::endl;
		std::cout << "\tColor: (" << *reinterpret_cast<float*>(ptr + 8) << ", "
			<< *reinterpret_cast<float*>(ptr + 12) << ", "
			<< *reinterpret_cast<float*>(ptr + 16) << ')' << std::endl;

		ptr += VERTEX_BYTES;
	}

	glUnmapNamedBuffer(vbo);
}

void TextEngine::printSSBO()
{
	unsigned char *base = static_cast<unsigned char*>(glMapNamedBufferRange(ssbo, 0, META_BYTES * range, GL_MAP_READ_BIT));
	unsigned char *ptr = base;

	for (unsigned u = 0; u < range; u++)
	{
		std::cout << static_cast<void*>(ptr) << ": '" << static_cast<char>(u + manager.charbase()) << '\'' << std::endl;
		std::cout << "\tAdvance X: " << *reinterpret_cast<int*>(ptr)
			<< " Advance Y: " << *reinterpret_cast<int*>(ptr + 4) << std::endl;
		std::cout << "\tBitmap Width: " << *reinterpret_cast<unsigned*>(ptr + 8)
			<< " Bitmap Height: " << *reinterpret_cast<unsigned*>(ptr + 12) << std::endl;
		std::cout << "\tLeft Bearing: " << *reinterpret_cast<int*>(ptr + 16)
			<< " Top Bearing: " << *reinterpret_cast<int*>(ptr + 20) << std::endl;

		std::cout << "\tTexel Base X: " << *reinterpret_cast<int*>(ptr + 24)
			<< " Texel Base Y: " << *reinterpret_cast<int*>(ptr + 28) << std::endl;

		ptr += META_BYTES;
	}

	glUnmapNamedBuffer(ssbo);
}
#endif

bool TextEngine::changeUpdateInfo(std::list<unsigned __int64>::iterator comparePosition, std::ptrdiff_t compareOffset)
{
	unsigned __int64 dist = std::distance(displayList.begin(), comparePosition);
	if (dist < updateIndex)
	{
		updateIndex = dist;
		updateOffset = compareOffset; //Update VBO starting at removal
		updateIterator = comparePosition;
		return true;
	}

	return false;
}

void TextEngine::updateBuffer()
{
	if(glyphs > capacity)
	{
		unsigned old_capacity = capacity;

		while(capacity < glyphs)
			capacity *= 1.5;
		
		GLuint swap;
		glCreateBuffers(1, &swap);
		glNamedBufferStorage(swap, VERTEX_BYTES * capacity, NULL, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
		glCopyNamedBufferSubData(vbo, swap, 0, 0, VERTEX_BYTES * old_capacity);
		glVertexArrayVertexBuffer(vao, 0, swap, 0, VERTEX_BYTES); //Update binding point with new buffer
		glDeleteBuffers(1, &vbo);
		vbo = swap; //Store the new buffer name

		//Add some counter or other mechanism to shrink buffer after a while
	}

	assert(updateIterator != displayList.end());

	unsigned char *base = static_cast<unsigned char*>(glMapNamedBufferRange(vbo, 0, VERTEX_BYTES * glyphs, GL_MAP_WRITE_BIT));
	unsigned char *ptr = base;
	
	ptr += updateOffset;
	
	while(updateIterator != displayList.end())
	{
		loadString(ptr, *updateIterator);
		display[*updateIterator].offset = (ptr - base);
		ptr += display[*updateIterator].str.length() * VERTEX_BYTES;
		updateIterator++;
	}
	
	glUnmapNamedBuffer(vbo);
		
	updateIterator = displayList.end();
	//If a new string is added, updateIndex must be larger than the new position
	//for changeUpdateInfo to change the updateIterator
	updateIndex = displayList.size() + 1;
	updateOffset = glyphs * VERTEX_BYTES;
	update = false;
}

void TextEngine::loadString(unsigned char *offset, unsigned __int64 id)
{
	Info &ref = display[id];

	int advanceX = ref.origin.x;
	for(int i = 0; i < ref.str.length(); i++)
	{
		/*std::cout << "Copying " << '\'' << ref.str[i] << '\'' << std::endl;
		glm::vec2 tmp = orthographic * glm::vec4{ advanceX, ref.origin.y, 0, 1 };
		std::cout << '(' << tmp.x << ", " << tmp.y << ')' << std::endl;*/
		unsigned index = static_cast<unsigned>(ref.str[i]) - manager.charbase();
		*reinterpret_cast<int*>(offset) = advanceX;
		*reinterpret_cast<int*>(offset + 4) = ref.origin.y;
		*reinterpret_cast<float*>(offset + 8) = ref.color.r;
		*reinterpret_cast<float*>(offset + 12) = ref.color.g;
		*reinterpret_cast<float*>(offset + 16) = ref.color.b;
		*reinterpret_cast<unsigned*>(offset + 20) = index;
		offset += VERTEX_BYTES;
		advanceX += manager.characterInfo()[index].ax >> 6;
	}
}

void TextEngine::loadMetaInfo()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glShaderStorageBlockBinding(program.id(), 0, 0);
	
	const FontManager::CharInfo *info = manager.characterInfo();
	const FontManager::AtlasMap *map = manager.atlasMap();
	
	float map_width = static_cast<float>(manager.mapWidth());
	float map_height = static_cast<float>(manager.mapHeight());
	
	unsigned char *buffer = static_cast<unsigned char*>(glMapNamedBufferRange(ssbo, 0, META_BYTES * range, GL_MAP_WRITE_BIT));
	
	for(unsigned u = 0; u < range; u++)
	{
		*reinterpret_cast<int*>(buffer) = info[u].ax;
		*reinterpret_cast<int*>(buffer + 4) = info[u].ay;
		*reinterpret_cast<unsigned*>(buffer + 8) = info[u].bw;
		*reinterpret_cast<unsigned*>(buffer + 12) = info[u].bh;
		*reinterpret_cast<int*>(buffer + 16) = info[u].lb;
		*reinterpret_cast<int*>(buffer + 20) = info[u].tb;

		*reinterpret_cast<int*>(buffer + 24) = map[u].lx; //X base
		*reinterpret_cast<int*>(buffer + 28) = map[u].hy; //Y base, high becomes low when flipped
		/*
			*reinterpret_cast<float*>(buffer + 24) = static_cast<float>(map[u].lx) / manager.mapWidth();
			*reinterpret_cast<float*>(buffer + 28) = static_cast<float>(map[u].hx) / manager.mapWidth();
			*reinterpret_cast<float*>(buffer + 32) = static_cast<float>(manager.mapHeight() - map[u].hy) / manager.mapHeight();
			*reinterpret_cast<float*>(buffer + 36) = static_cast<float>(manager.mapHeight() - map[u].lx) / manager.mapHeight();
		*/
		buffer += META_BYTES;
	}
	
	glUnmapNamedBuffer(ssbo);
}