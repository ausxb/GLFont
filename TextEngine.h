#pragma once

#include <string>
#include <list>
#include <map>

#include "FontManager.h"
#include "Program.h"
#include "Texture.h"
#include "GL/glew.h"
#include "GLM/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class TextEngine
{
public:
	explicit TextEngine(const FontManager &mgr, const glwrap::Program &prg, unsigned width, unsigned height, unsigned initCapacity);
	~TextEngine();
	
	///Setup OpenGL state for rendering, then render.
	/*!
	 * Uses TextEngine::program, loads the orthographic view matrix,
	 * binds the texture atlas TextEngine::texture, binds the shader
	 * storage block buffer, binds the VAO and then makes the single
	 * draw call for all vertices.
	 */
	void render();
	
	///Add the string to the rendering list
	/*!
	 * \param[in] s The string to render
	 * \param[in] origin The integer coordinates origin of each font glyph, advanced for each character
	 * \param[in] color The floating point RGB color to render each character
	 */
	unsigned __int64 addString(const std::string &s, glm::ivec2 &origin, glm::vec3 &color);
	
	///Remove the string from the rendering list
	/*!
	 * \param[in] id The string id
	 * \return true if the id was found, false otherwise.
	 */
	bool removeString(unsigned __int64 id);
	
	///Change the string for a given id
	/*!
	 * \param[in] id The string id
	 * \param[in] s The new string
	 * \return true if the id was found, false otherwise.
	 */
	bool updateString(unsigned __int64 id, const std::string &s);

	///Change the position for a given id
	/*!
	* \param[in] id The string id
	* \param[in] origin The new origin
	* \return true if the id was found, false otherwise.
	*/
	bool updateOrigin(unsigned __int64 id, const glm::ivec2 &origin);

	///Change the color for a given id
	/*!
	* \param[in] id The string id
	* \param[in] color The new color
	* \return true if the id was found, false otherwise.
	*/
	bool updateColor(unsigned __int64 id, const glm::vec3 &color);
#ifdef _DEBUG
	void printVBO();

	void printSSBO();
#endif
	
private:
	static constexpr unsigned __int64 VERTEX_BYTES = 24; ///< Bytes per vertex of glyph (2 floats xy + 3 floats rgb + 1 uint index)
	static constexpr unsigned __int64 META_BYTES = 32; ///< Meta structure size in shader
	const unsigned scX, scY; ///< Screen dimensions
	const FontManager &manager;
	const glwrap::Program &program;
	glwrap::Texture texture;
	GLuint vbo, vao, ssbo; ///< Names for the VBO, VAO, and SSBO used in the engine
	unsigned range, glyphs, capacity;
	bool update;
	unsigned __int64 text_id, updateIndex, updateOffset;
	glm::mat4 orthographic;
	
	struct Info
	{
		std::string str;
		glm::ivec2 origin;
		glm::vec3 color;
		std::list<unsigned __int64>::iterator position;
		std::ptrdiff_t offset;
		
		/*
		Info::Info() { }
		
		Info::Info(
			const std::string& s,
			const glm::vec2& o,
			const glm::vec3& c,
			const std::list<unsigned __int64>::iterator &p,
			const std::ptrdiff_t &off)
		: str{ s }, origin{ o }, color{ c }, position{ p }, offset{ off } { }
		*/
	};
	
	std::map<unsigned __int64, Info> display;
	std::list<unsigned __int64> displayList;
	std::list<unsigned __int64>::iterator updateIterator;

	///Change update information if necessary
	/*!
	 * \param[in] comparePosition iterator to the id in TextEngine::displayList that was updated
	 * \param[in] compareOffset byte offset of the first glyph of the string in the VBO
	 *
	 * Updates TextEngine::updateIndex, TextEngine::updateOffset, and TextEngine::updateIterator
	 * with the value of the paramaters if and only if comparePosition is closer to the beginning
	 * of TextEngine::displayList than the current TextEngine::updateIndex.
	 *
	 * \return true if the update information was changed, false otherwise.
	 */
	bool changeUpdateInfo(std::list<unsigned __int64>::iterator comparePosition, std::ptrdiff_t compareOffset);
	
	///Rebuild part or all of the vertex buffer
	/*!
	 * Only called when updates are necessary.
	 * Begins by reallocating a larger VBO if necessary, then walks the id list starting from the update index.
	 * The updated data is copied into the VBO starting from the offset of the deleted string closest to the start of the buffer.
	 * For each string after the update location on TextEngine::displayList, the data is formatted and applied to the VBO using TextEngine::loadString().
	 * The offset into the VBO for that id is then updated to reflect its new position.
	 * TextEngine::updateIterator, TextEngine::updateIndex are then reset to reference the end of the list.
	 */
	void updateBuffer();
	
	///Copy formatted string data to VBO
	/*!
	 * \param offset pointer to the location in VBO where the string data should be copied
	 * \param id id of the string to copy
	 *
	 * Starts at the origin associated with the id.
	 * Takes each character from the associated string and puts the
	 * local glyph origin, color, and codepoint, in that order, into
	 * the buffer.
	 * The local origin's X coordinate is moved after each character
	 * by that character's advance.
	 */
	void loadString(unsigned char *offset, unsigned __int64 id);
	
	///Fill out the SSBO in the vertex shader with the details for each glyph
	/*!
	 *
	 */
	void loadMetaInfo();
};